#ifndef FERMION_FORCE_C
#define FERMION_FORCE_C


#include "../DbgTools/dbgtools.h"
#include "../Include/common_defines.h"
#include "../Include/debug.h"
#include "../Include/fermion_parameters.h"
#include "../Mpi/multidev.h"
#include "./action.h"
#include "./alloc_vars.h"
#include "./backfield.h"
#include "./fermion_force.h"
#include "./fermion_force_utilities.h"
#include "./inverter_multishift_full.h"
#include "./md_parameters.h"
#include "./plaquettes.h"
#include "./stouting.h"
#include "./struct_c_def.h"
#include "./su3_measurements.h"
#include "./su3_utilities.h"

// includes related to single precision acceleration
// Global variable used for convenience
#include "./alloc_vars.h"
#include "./float_double_conv.h"
#include "./sp_alloc_vars.h"
#include "./sp_struct_c_def.h"
#include "./sp_inverter_multishift_full.h"
#include "./inverter_full.h"



#ifndef __GNUC__
#define TIMING_FERMION_FORCE
#endif

// if using GCC, there are some problems with __restrict.
#ifdef __GNUC__
#define __restrict
#endif

#ifdef MULTIDEVICE
#include "../Mpi/communications.h"
#endif 

extern int verbosity_lv;

extern tamat_soa * ipdot_f_old; // see alloc_vars.c

void compute_sigma_from_sigma_prime_backinto_sigma_prime(  __restrict su3_soa    * Sigma, // la var globale e' auxbis_conf_acc [sia input che ouptput]
        __restrict thmat_soa  * Lambda, // la var globale e' aux_th
        __restrict tamat_soa  * QA, // la var globale e' aux_ta
        __restrict const su3_soa * const U,// la var globale e' .... per adesso conf_acc
        __restrict su3_soa * const TMP//la var globale e' aux_conf_acc //PARCHEGGIO??
        ){
    
    if(verbosity_lv > 3) printf("DOUBLE PRECISION VERSION OF COMPUTE_SIGMA_FROM_SIGMA_PRIME_BACKINTO_SIGMA_PRIME\n");




    if(verbosity_lv > 2) printf("MPI%02d:\t\tSIGMA_PRIME --> SIGMA\n",
            devinfo.myrank);
    if(verbosity_lv > 5){// printing stuff
#pragma acc update host(Sigma[0:8])
        printf("-------------Sigma[old]------------------\n");                                                                                             
        printf("Sigma[old]00 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r0.c0[0]),cimag(Sigma[0].r0.c0[0]));                                               
        printf("Sigma[old]01 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r0.c1[0]),cimag(Sigma[0].r0.c1[0]));                                               
        printf("Sigma[old]02 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r0.c2[0]),cimag(Sigma[0].r0.c2[0]));                                               
        printf("Sigma[old]10 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r1.c0[0]),cimag(Sigma[0].r1.c0[0]));                                               
        printf("Sigma[old]11 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r1.c1[0]),cimag(Sigma[0].r1.c1[0]));                                               
        printf("Sigma[old]12 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r1.c2[0]),cimag(Sigma[0].r1.c2[0]));                                               
        printf("Sigma[old]20 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r2.c0[0]),cimag(Sigma[0].r2.c0[0]));                                               
        printf("Sigma[old]21 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r2.c1[0]),cimag(Sigma[0].r2.c1[0]));                                               
        printf("Sigma[old]22 = %.18lf + (%.18lf)*I\n\n",creal(Sigma[0].r2.c2[0]),cimag(Sigma[0].r2.c2[0]));                
    }

    set_su3_soa_to_zero(TMP);

    calc_loc_staples_nnptrick_all(U,TMP);
    if(verbosity_lv > 4)printf("MPI%02d:\t\tcomputed staples  \n",
            devinfo.myrank);
#ifdef MULTIDEVICE
    communicate_gl3_borders(TMP,1);
#endif

    RHO_times_conf_times_staples_ta_part(U,TMP,QA);

#ifdef MULTIDEVICE
    communicate_tamat_soa_borders(QA,1);
#endif

    // check: TMP = local staples.
    if(verbosity_lv > 4) printf("MPI%02d:\t\tcomputed Q  \n",
            devinfo.myrank);
    if(verbosity_lv > 5) {// printing stuff
#pragma acc update host(QA[ 0:8])
        printf("-------------Q------------------\n");
        printf("Q00 = %.18lf\n",QA[0].ic00[0]);
        printf("Q00 = %.18lf\n",QA[0].ic11[0]);
        printf("Q01 = %.18lf + (%.18lf)*I\n",creal(QA[0].c01[0]),cimag(QA[0].c01[0]));
        printf("Q02 = %.18lf + (%.18lf)*I\n",creal(QA[0].c02[0]),cimag(QA[0].c02[0]));
        printf("Q12 = %.18lf + (%.18lf)*I\n\n",creal(QA[0].c12[0]),cimag(QA[0].c12[0]));
    }
    compute_lambda(Lambda,Sigma,U,QA,TMP);

#ifdef MULTIDEVICE
    communicate_thmat_soa_borders(Lambda,1);
#endif

    if(verbosity_lv > 4)   printf("MPI%02d:\t\tcomputed Lambda  \n",
            devinfo.myrank);

    if(verbosity_lv > 5) {// printing stuff
#pragma acc update host(Lambda[0:8])
        printf("-------------LAMBDA------------------\n");
        printf("Lambda00 = %.18lf\n",Lambda[0].rc00[0]);
        printf("Lambda00 = %.18lf\n",Lambda[0].rc11[0]);
        printf("Lambda01 = %.18lf + (%.18lf)*I\n",creal(Lambda[0].c01[0]),cimag(Lambda[0].c01[0]));
        printf("Lambda02 = %.18lf + (%.18lf)*I\n",creal(Lambda[0].c02[0]),cimag(Lambda[0].c02[0]));
        printf("Lambda12 = %.18lf + (%.18lf)*I\n\n",creal(Lambda[0].c12[0]),cimag(Lambda[0].c12[0]));
    }
    compute_sigma(Lambda,U,Sigma,QA,TMP);
    if(verbosity_lv > 4)   printf("MPI%02d:\t\tcomputed Sigma  \n",
            devinfo.myrank);

    if(verbosity_lv > 5) {// printing stuff
#pragma acc update host(Sigma[0:8])
        printf("-------------Sigma[new]------------------\n");                                                                                             
        printf("Sigma[new]00 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r0.c0[0]),cimag(Sigma[0].r0.c0[0]));                                               
        printf("Sigma[new]01 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r0.c1[0]),cimag(Sigma[0].r0.c1[0]));                                               
        printf("Sigma[new]02 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r0.c2[0]),cimag(Sigma[0].r0.c2[0]));                                               
        printf("Sigma[new]10 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r1.c0[0]),cimag(Sigma[0].r1.c0[0]));                                               
        printf("Sigma[new]11 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r1.c1[0]),cimag(Sigma[0].r1.c1[0]));                                               
        printf("Sigma[new]12 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r1.c2[0]),cimag(Sigma[0].r1.c2[0]));                                               
        printf("Sigma[new]20 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r2.c0[0]),cimag(Sigma[0].r2.c0[0]));                                               
        printf("Sigma[new]21 = %.18lf + (%.18lf)*I\n",creal(Sigma[0].r2.c1[0]),cimag(Sigma[0].r2.c1[0]));                                               
        printf("Sigma[new]22 = %.18lf + (%.18lf)*I\n\n",creal(Sigma[0].r2.c2[0]),cimag(Sigma[0].r2.c2[0]));                
    }

#ifdef MULTIDEVICE
        communicate_gl3_borders(Sigma,1);
#endif

}



void fermion_force_soloopenacc(__restrict su3_soa    * tconf_acc,
#ifdef STOUT_FERMIONS        
			       __restrict su3_soa * tstout_conf_acc_arr,// parking
#endif
			       __restrict su3_soa * gl3_aux, // gl(3) parking
			       __restrict tamat_soa  * tipdot_acc,
			       __restrict ferm_param * tfermion_parameters,// [nflavs] 
			       int tNDiffFlavs,
			       __restrict const vec3_soa * ferm_in_acc, // [NPS_tot]         
			       double res,
			       __restrict su3_soa  * taux_conf_acc,
			       __restrict vec3_soa * tferm_shiftmulti_acc,//parking variable [max_ps*max_approx_order]           
			       __restrict vec3_soa * tkloc_r, // parking 
			       __restrict vec3_soa * tkloc_h, // parking 
			       __restrict vec3_soa * tkloc_s, // parking 
			       __restrict vec3_soa * tkloc_p, // parking 
                   __restrict vec3_soa * tk_p_shiftferm,//parking [max_approx_order] 
                   const int max_cg )
{
    if(verbosity_lv > 3) printf("DOUBLE PRECISION VERSION OF FERMION_FORCE_SOLOOPENACC\n");

    if(verbosity_lv > 2){
        printf("MPI%02d:\tCalculation of fermion force...\n", 
                devinfo.myrank);
    }

#ifdef TIMING_FERMION_FORCE
    struct timeval t1,t2;
    gettimeofday ( &t1, NULL );
#endif

    __restrict su3_soa * conf_to_use; // CONF TO USE IN CALCULATION OF 
    __restrict su3_soa_f * conf_to_use_f; // CONF TO USE IN CALCULATION OF 
    // FERMION FORCE
#ifdef STOUT_FERMIONS
    stout_wrapper(tconf_acc,tstout_conf_acc_arr);// calcolo 
    if(act_params.stout_steps > 0) 
        conf_to_use =  
            &(tstout_conf_acc_arr[8*(act_params.stout_steps-1)]);
    else conf_to_use = tconf_acc;
#else
    conf_to_use = tconf_acc;
#endif
    set_su3_soa_to_zero(gl3_aux); // pseudo ipdot
    set_tamat_soa_to_zero(tipdot_acc);

    if(md_parameters.singlePInvAccel == 1 && md_parameters.singlePrecMD != 1){
       conf_to_use_f = conf_acc_f; // USING GLOBAL VARIABLE FOR CONVENIENCE
       convert_double_to_float_su3_soa(conf_to_use,conf_to_use_f);
    }


    for(int iflav = 0; iflav < tNDiffFlavs; iflav++) {
        set_su3_soa_to_zero(taux_conf_acc);
        int ifps = tfermion_parameters[iflav].index_of_the_first_ps;
        for(int ips = 0 ; ips < tfermion_parameters[iflav].number_of_ps ; ips++){

            if(md_parameters.singlePInvAccel == 1 && md_parameters.singlePrecMD != 1){

                // USING FLOAT GLOBAL VARIABLES, HOPEFULLY NOT USED ELSEWHERE
                convert_double_to_float_vec3_soa(&(ferm_in_acc[ifps+ips]),aux1_f);
                // multishift inverter in single precision
                float realisticTargetRes = 8e-7f*sqrtf(sizeh);

                multishift_invert_f(conf_to_use_f, &tfermion_parameters[iflav], 
                    &(tfermion_parameters[iflav].approx_md), 
                    ferm_shiftmulti_acc_f,aux1_f, realisticTargetRes, 
                    kloc_r_f, kloc_h_f, kloc_s_f, kloc_p_f, k_p_shiftferm_f, max_cg);

               // single inversions on all shifts
               
               int ishift;
               for(ishift=0;ishift<tfermion_parameters[iflav].approx_md.approx_order;ishift++){

                   convert_float_to_double_vec3_soa(&ferm_shiftmulti_acc_f[ishift],
                           aux1);// trial sol, hopefully close

                   double bshift = tfermion_parameters[iflav].approx_md.RA_b[ishift];
                   ker_invert_openacc(conf_to_use, &tfermion_parameters[iflav],
                        &tferm_shiftmulti_acc[ishift],&(ferm_in_acc[ifps+ips]),res,
                        aux1,// trial solution
                        tkloc_r,tkloc_h,tkloc_s,tkloc_p,max_cg,bshift);

                   // direct conversion, without 'double precision refining'
                   //convert_float_to_double_vec3_soa(&ferm_shiftmulti_acc_f[ishift],
                   //        &tferm_shiftmulti_acc[ishift]);// trial sol, hopefully close


               }

            }
            else multishift_invert(conf_to_use, &tfermion_parameters[iflav], 
                    &(tfermion_parameters[iflav].approx_md), 
                    tferm_shiftmulti_acc, &(ferm_in_acc[ifps+ips]), res, 
                    tkloc_r, tkloc_h, tkloc_s, tkloc_p, tk_p_shiftferm, max_cg);

            ker_openacc_compute_fermion_force(conf_to_use, taux_conf_acc, tferm_shiftmulti_acc, tkloc_s, tkloc_h, &(tfermion_parameters[iflav]));

        }

        // JUST MULTIPLY BY STAGGERED PHASES,
        // BACK FIELD AND/OR CHEMICAL POTENTIAL 
        multiply_backfield_times_force(&(tfermion_parameters[iflav]),taux_conf_acc,gl3_aux);
        if(md_dbg_print_count<debug_settings.md_dbg_print_max_count){
            char taux_conf_acc_name[50];
            sprintf(taux_conf_acc_name,
                    "taux_conf_acc_%s_%d_%d",tfermion_parameters[iflav].name,
                    devinfo.myrank, md_dbg_print_count);
            dbg_print_su3_soa(taux_conf_acc,taux_conf_acc_name, 1);
        }


    }
#ifdef STOUT_FERMIONS

    for(int stout_level = act_params.stout_steps ; stout_level > 1 ; 
            stout_level--){
        if(verbosity_lv > 1) 
            printf("MPI%02d:\t\tSigma' to Sigma [lvl %d to lvl %d]\n",
                    devinfo.myrank, stout_level,stout_level-1);
        conf_to_use = &(tstout_conf_acc_arr[8*(stout_level-2)]);
        compute_sigma_from_sigma_prime_backinto_sigma_prime(gl3_aux,
                aux_th,aux_ta,conf_to_use, taux_conf_acc );
        if(md_dbg_print_count<debug_settings.md_dbg_print_max_count){
            char gl3_aux_name[50];
            sprintf(gl3_aux_name,
                    "gl3_aux_name_%dstout_%d_%d",stout_level,
                    devinfo.myrank, md_dbg_print_count);
            dbg_print_su3_soa(gl3_aux,gl3_aux_name,1);
        }

    }
    if(act_params.stout_steps > 0 ){
    if(verbosity_lv > 1) 
        printf("MPI%02d:\t\tSigma' to Sigma [lvl 1 to lvl 0]\n",
                devinfo.myrank);
    compute_sigma_from_sigma_prime_backinto_sigma_prime(gl3_aux,
            aux_th,aux_ta,tconf_acc, taux_conf_acc );
    }
#endif




    multiply_conf_times_force_and_take_ta_nophase(tconf_acc, gl3_aux,
            tipdot_acc);



    /*
#pragma acc update host(tipdot_acc[0:8])
printf("-------------FFORCE------------------\n");
printf("F00 = %.18lf\n",tipdot_acc[0].rc00[0]);
printf("F11 = %.18lf\n",tipdot_acc[0].rc11[0]);
printf("F01 = %.18lf + (%.18lf)*I\n",creal(tipdot_acc[0].c01[0]),cimag(tipdot_acc[0].c01[0]));
printf("F02 = %.18lf + (%.18lf)*I\n",creal(tipdot_acc[0].c02[0]),cimag(tipdot_acc[0].c02[0]));
printf("F12 = %.18lf + (%.18lf)*I\n\n",creal(tipdot_acc[0].c12[0]),cimag(tipdot_acc[0].c12[0]));

*/

#ifdef TIMING_FERMION_FORCE
    gettimeofday ( &t2, NULL );
    double dt_preker_to_postker = (double)(t2.tv_sec - t1.tv_sec) + ((double)(t2.tv_usec - t1.tv_usec)/1.0e6);
    printf("MPI%02d\t\t\
FULL FERMION FORCE COMPUTATION  PreKer->PostKer :%f sec  \n",
dt_preker_to_postker,devinfo.myrank);
#endif
    if(verbosity_lv > 0){
        printf("MPI%02d:\t\tCompleted fermion force openacc\n",
                devinfo.myrank);
    }

    if(debug_settings.save_diagnostics == 1 ){
        double  force_norm, diff_force_norm;
        force_norm = calc_force_norm(tipdot_acc);
        diff_force_norm = calc_diff_force_norm(tipdot_acc,ipdot_f_old);
        copy_ipdot_into_old(tipdot_acc,ipdot_f_old);


        FILE *foutfile = 
            fopen(debug_settings.diagnostics_filename,"at");
        fprintf(foutfile,"FFHN %e \tDFFHN %e \t",
                force_norm,diff_force_norm);
        fclose(foutfile);

        if(verbosity_lv > 1)
            printf("MPI%02d:\
\t\t\tFermion Force Half Norm: %e, Diff with previous:%e\n",
                    devinfo.myrank, force_norm, diff_force_norm);
    } 


}

/*
//STANDARD VERSION OF THE FERMIONIC FORCE
void fermion_force_soloopenacc(__restrict su3_soa    * tconf_acc,
        // la configurazione qui dentro e' costante e non viene modificata
#ifdef STOUT_FERMIONS        
        __restrict su3_soa * tstout_conf_acc_arr,// parking
#endif
        __restrict su3_soa * gl3_aux, // gl(3) parking
        __restrict tamat_soa  * tipdot_acc,
        __restrict ferm_param * tfermion_parameters,// [nflavs] 
        int tNDiffFlavs,
        __restrict vec3_soa * ferm_in_acc, // [NPS_tot]         
        double res,
        __restrict su3_soa  * taux_conf_acc,
        __restrict vec3_soa * tferm_shiftmulti_acc,//parking variable [max_ps*max_approx_order]           
        __restrict vec3_soa * tkloc_r, // parking 
        __restrict vec3_soa * tkloc_h, // parking 
        __restrict vec3_soa * tkloc_s, // parking 
        __restrict vec3_soa * tkloc_p, // parking 
        __restrict vec3_soa * tk_p_shiftferm,//parking variable [max_approx_order]
        const int max_cg )
{
    if(verbosity_lv > 3) printf("DOUBLE PRECISION VERSION OF FERMION_FORCE_SOLOOPENACC\n");

    if(verbosity_lv > 2){
        printf("MPI%02d:\tCalculation of fermion force...\n", 
                devinfo.myrank);
    }

#ifdef TIMING_FERMION_FORCE
    struct timeval t1,t2;
    gettimeofday ( &t1, NULL );
#endif

    __restrict su3_soa * conf_to_use; // CONF TO USE IN CALCULATION OF 
    // FERMION FORCE
#ifdef STOUT_FERMIONS
    stout_wrapper(tconf_acc,tstout_conf_acc_arr);// calcolo 
    if(act_params.stout_steps > 0) 
        conf_to_use =  
            &(tstout_conf_acc_arr[8*(act_params.stout_steps-1)]);
    else conf_to_use = tconf_acc;
#else
    conf_to_use = tconf_acc;
#endif
    set_su3_soa_to_zero(gl3_aux); // pseudo ipdot
    set_tamat_soa_to_zero(tipdot_acc);



    for(int iflav = 0; iflav < tNDiffFlavs; iflav++) {
        set_su3_soa_to_zero(taux_conf_acc);
        int ifps = tfermion_parameters[iflav].index_of_the_first_ps;
        for(int ips = 0 ; ips < tfermion_parameters[iflav].number_of_ps ; ips++){

            multishift_invert(conf_to_use, &tfermion_parameters[iflav], 
                    &(tfermion_parameters[iflav].approx_md), 
                    tferm_shiftmulti_acc, &(ferm_in_acc[ifps+ips]), res, 
                    tkloc_r, tkloc_h, tkloc_s, tkloc_p, tk_p_shiftferm, max_cg);

            ker_openacc_compute_fermion_force(conf_to_use, taux_conf_acc, tferm_shiftmulti_acc, tkloc_s, tkloc_h, &(tfermion_parameters[iflav]));

        }

        // JUST MULTIPLY BY STAGGERED PHASES,
        // BACK FIELD AND/OR CHEMICAL POTENTIAL 
        multiply_backfield_times_force(&(tfermion_parameters[iflav]),taux_conf_acc,gl3_aux);
        if(md_dbg_print_count<debug_settings.md_dbg_print_max_count){
            char taux_conf_acc_name[50];
            sprintf(taux_conf_acc_name,
                    "taux_conf_acc_%s_%d_%d",tfermion_parameters[iflav].name,
                    devinfo.myrank, md_dbg_print_count);
            dbg_print_su3_soa(taux_conf_acc,taux_conf_acc_name, 1);
        }


    }
#ifdef STOUT_FERMIONS

    for(int stout_level = act_params.stout_steps ; stout_level > 1 ; 
            stout_level--){
        if(verbosity_lv > 1) 
            printf("MPI%02d:\t\tSigma' to Sigma [lvl %d to lvl %d]\n",
                    devinfo.myrank, stout_level,stout_level-1);
        conf_to_use = &(tstout_conf_acc_arr[8*(stout_level-2)]);
        compute_sigma_from_sigma_prime_backinto_sigma_prime(gl3_aux,
                aux_th,aux_ta,conf_to_use, taux_conf_acc );
        if(md_dbg_print_count<debug_settings.md_dbg_print_max_count){
            char gl3_aux_name[50];
            sprintf(gl3_aux_name,
                    "gl3_aux_name_%dstout_%d_%d",stout_level,
                    devinfo.myrank, md_dbg_print_count);
            dbg_print_su3_soa(gl3_aux,gl3_aux_name,1);
        }

    }
    if(act_params.stout_steps > 0 ){
    if(verbosity_lv > 1) 
        printf("MPI%02d:\t\tSigma' to Sigma [lvl 1 to lvl 0]\n",
                devinfo.myrank);
    compute_sigma_from_sigma_prime_backinto_sigma_prime(gl3_aux,
            aux_th,aux_ta,tconf_acc, taux_conf_acc );
    }
#endif




    multiply_conf_times_force_and_take_ta_nophase(tconf_acc, gl3_aux,
            tipdot_acc);



 

#ifdef TIMING_FERMION_FORCE
    gettimeofday ( &t2, NULL );
    double dt_preker_to_postker = (double)(t2.tv_sec - t1.tv_sec) + ((double)(t2.tv_usec - t1.tv_usec)/1.0e6);
    printf("MPI%02d\t\t\
FULL FERMION FORCE COMPUTATION  PreKer->PostKer :%f sec  \n",
dt_preker_to_postker,devinfo.myrank);
#endif
    if(verbosity_lv > 0){
        printf("MPI%02d:\t\tCompleted fermion force openacc\n",
                devinfo.myrank);
    }

    if(debug_settings.save_diagnostics == 1 ){
        double  force_norm, diff_force_norm;
        force_norm = calc_force_norm(tipdot_acc);
        diff_force_norm = calc_diff_force_norm(tipdot_acc,ipdot_f_old);
        copy_ipdot_into_old(tipdot_acc,ipdot_f_old);


        FILE *foutfile = 
            fopen(debug_settings.diagnostics_filename,"at");
        fprintf(foutfile,"FFHN %e \tDFFHN %e \t",
                force_norm,diff_force_norm);
        fclose(foutfile);

        if(verbosity_lv > 1)
            printf("MPI%02d:\
\t\t\tFermion Force Half Norm: %e, Diff with previous:%e\n",
                    devinfo.myrank, force_norm, diff_force_norm);
    } 


}
*/

#endif
