#ifndef BACKFIELD_C_
#define BACKFIELD_C_

#include "./geometry.h"
#include "./backfield.h"
#include "./struct_c_def.h"
#include "../Mpi/multidev.h"
#include <stdio.h>


#define acc_twopi 2*3.14159265358979323846
#ifndef __GNUC__
#include <accelmath.h>
#endif


bf_param backfield_parameters;

void calc_u1_phases(double_soa * phases,bf_param bf_pars,
        double im_chem_pot, double ferm_charge)
{
    double  ex_quantum = bf_pars.ex;
    double  ey_quantum = bf_pars.ey;
    double  ez_quantum = bf_pars.ez;
    double  bx_quantum = bf_pars.bx;
    double  by_quantum = bf_pars.by;
    double  bz_quantum = bf_pars.bz;


    int X,Y,Z,T;
    double  arg;
    double chpotphase = im_chem_pot/geom_par.gnt; 

    int x, y, z, t, parity;
    int d[4], idxh;

    if(verbosity_lv > 2 && 0 == devinfo.myrank ) { 
        printf("Direction mapping  x y z t: %d %d %d %d\n",
                geom_par.xmap,geom_par.ymap,geom_par.zmap,geom_par.tmap);
        printf("EM field quanta: \n\tB: %f %f %f \n\tE: %f %f %f\n",
                bx_quantum, by_quantum,bz_quantum,ex_quantum,ey_quantum,ez_quantum);

    }

#ifdef MULTIDEVICE

    printf("MPI%02d: Origin coordinates: (x=%02d y=%02d z=%02d t=%02d)\n",
            devinfo.myrank ,  devinfo.origin_0123[geom_par.xmap],
            devinfo.origin_0123[geom_par.ymap], devinfo.origin_0123[geom_par.zmap],
            devinfo.origin_0123[geom_par.tmap]);


    if(0==devinfo.myrank) printf("Halo Widths: x: %d y: %d z: %d t :%d\n",
            devinfo.halo_widths0123[geom_par.xmap],
            devinfo.halo_widths0123[geom_par.ymap],
            devinfo.halo_widths0123[geom_par.zmap],
            devinfo.halo_widths0123[geom_par.tmap]);

#endif                  


    for(d[3]=0; d[3] < nd3; d[3]++) for(d[2]=0; d[2] < nd2; d[2]++)
        for(d[1]=0; d[1] < nd1; d[1]++) for(d[0]=0; d[0] < nd0; d[0]++){

            idxh = snum_acc(d[0],d[1],d[2],d[3]);

            x = d[geom_par.xmap];int tnx = geom_par.gnx;
            y = d[geom_par.ymap];int tny = geom_par.gny;
            z = d[geom_par.zmap];int tnz = geom_par.gnz;
            t = d[geom_par.tmap];int tnt = geom_par.gnt;

#ifdef MULTIDEVICE
            x+= devinfo.origin_0123[geom_par.xmap]
                - devinfo.halo_widths0123[geom_par.xmap];  
            y+= devinfo.origin_0123[geom_par.ymap]
                - devinfo.halo_widths0123[geom_par.ymap];
            z+= devinfo.origin_0123[geom_par.zmap]
                - devinfo.halo_widths0123[geom_par.zmap];
            t+= devinfo.origin_0123[geom_par.tmap]
                - devinfo.halo_widths0123[geom_par.tmap];

            if(x>tnx-1)  x-= tnx ; if(x<0)  x+= tnx ; 
            if(y>tny-1)  y-= tny ; if(y<0)  y+= tny ; 
            if(z>tnz-1)  z-= tnz ; if(z<0)  z+= tnz ; 
            if(t>tnt-1)  t-= tnt ; if(t<0)  t+= tnt ; 

#endif                  

            parity = (x+y+z+t)%2; 
            // NOTICE: (x+y+z+t)%2 =/= (nd0+nd1+nd2+nd3)%2


            X = x + 1;
            Y = y + 1;
            Z = z + 1;
            T = t + 1;

            ////////X-oriented////////
            if(X == tnx){
                // x-oriented links on the boundary
                arg = (y+1)*tnx*bz_quantum/(tnx*tny);
                arg+= (t+1)*tnx*ex_quantum/(tnx*tnt);
                arg-= (z+1)*by_quantum/(tnz*tnx);
            }
            else arg = -(z+1)*by_quantum/(tnz*tnx);

            arg *= ferm_charge;// only em phase so far
            if(KSphaseX(x,y,z,t) == -1) arg += 0.5;
            // ^this should be false, UNLESS one chooses
            // a different setup for the staggered phases....
            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.xmap*2+parity].d[idxh]= acc_twopi*arg; 


            ////////Y-oriented/////////
            if(Y == tny){
                // y-oriented links on the boundary
                arg = (z+1)*tny*bx_quantum/(tny*tnz);
                arg+= (t+1)*tny*ey_quantum/(tny*tnt);
                arg-= (x+1)*bz_quantum/(tnx*tny);
            }
            else arg = -(x+1)*bz_quantum/(tnx*tny);

            arg *= ferm_charge;// only am phase so far
            if(KSphaseY(x,y,z,t) == -1) arg += 0.5;
            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.ymap*2+parity].d[idxh]=acc_twopi*arg;


            ////////Z-oriented////////
            if(Z == tnz){
                // z-oriented links on the boundary
                arg = (t+1)*tnz*ez_quantum/(tnz*tnt);
                arg += (x+1)*tnz*by_quantum/(tnz*tnx);
                arg -= (y+1)*bx_quantum/(tny*tnz);
            }
            else arg = -(y+1)*bx_quantum/(tny*tnz);

            arg *= ferm_charge;// only am phase so far
            if(KSphaseZ(x,y,z,t) == -1) arg += 0.5;
            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.zmap*2+parity].d[idxh]=acc_twopi*arg;


            ///////T-oriented////////
            arg = -(z+1)*ez_quantum/(tnz*tnt);
            arg -= (y+1)*ey_quantum/(tny*tnt);
            arg -= (x+1)*ex_quantum/(tnx*tnt);

            arg *= ferm_charge;// only am phase so far
            if(KSphaseT(x,y,z,t) == -1) arg += 0.5;
            arg += chpotphase*0.5; // it must be multiplied by pi, not 2pi
            if(T == tnt) arg += 0.5;
            // antiperiodic boundary conds
            // notice T = t+1 !!

            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.tmap*2+parity].d[idxh]=acc_twopi*arg;
        } // d3,d2,d1,d0 loops







}

void calc_u1_phases_nissaversion(double_soa * phases,bf_param bf_pars,
        double im_chem_pot, double ferm_charge)
{
    double  ex_quantum = bf_pars.ex;
    double  ey_quantum = bf_pars.ey;
    double  ez_quantum = bf_pars.ez;
    double  bx_quantum = bf_pars.bx;
    double  by_quantum = bf_pars.by;
    double  bz_quantum = bf_pars.bz;


    int X,Y,Z,T;
    double  arg;
    double chpotphase = im_chem_pot/geom_par.gnt; 

    int x, y, z, t, parity;
    int d[4], idxh;

    if(verbosity_lv > 2 && 0 == devinfo.myrank ) { 
        printf("Direction mapping  x y z t: %d %d %d %d\n",
                geom_par.xmap,geom_par.ymap,geom_par.zmap,geom_par.tmap);
        printf("EM field quanta: \n\tB: %f %f %f \n\tE: %f %f %f\n",
                bx_quantum, by_quantum,bz_quantum,ex_quantum,ey_quantum,ez_quantum);

    }

#ifdef MULTIDEVICE

    printf("MPI%02d: Origin coordinates: (x=%02d y=%02d z=%02d t=%02d)\n",
            devinfo.myrank ,  devinfo.origin_0123[geom_par.xmap],
            devinfo.origin_0123[geom_par.ymap], devinfo.origin_0123[geom_par.zmap],
            devinfo.origin_0123[geom_par.tmap]);


    if(0==devinfo.myrank) printf("Halo Widths: x: %d y: %d z: %d t :%d\n",
            devinfo.halo_widths0123[geom_par.xmap],
            devinfo.halo_widths0123[geom_par.ymap],
            devinfo.halo_widths0123[geom_par.zmap],
            devinfo.halo_widths0123[geom_par.tmap]);

#endif                  


    for(d[3]=0; d[3] < nd3; d[3]++) for(d[2]=0; d[2] < nd2; d[2]++)
        for(d[1]=0; d[1] < nd1; d[1]++) for(d[0]=0; d[0] < nd0; d[0]++){

            idxh = snum_acc(d[0],d[1],d[2],d[3]);

            x = d[geom_par.xmap];int tnx = geom_par.gnx;
            y = d[geom_par.ymap];int tny = geom_par.gny;
            z = d[geom_par.zmap];int tnz = geom_par.gnz;
            t = d[geom_par.tmap];int tnt = geom_par.gnt;

#ifdef MULTIDEVICE
            x+= devinfo.origin_0123[geom_par.xmap]
                - devinfo.halo_widths0123[geom_par.xmap];  
            y+= devinfo.origin_0123[geom_par.ymap]
                - devinfo.halo_widths0123[geom_par.ymap];
            z+= devinfo.origin_0123[geom_par.zmap]
                - devinfo.halo_widths0123[geom_par.zmap];
            t+= devinfo.origin_0123[geom_par.tmap]
                - devinfo.halo_widths0123[geom_par.tmap];

            if(x>tnx-1)  x-= tnx ; if(x<0)  x+= tnx ; 
            if(y>tny-1)  y-= tny ; if(y<0)  y+= tny ; 
            if(z>tnz-1)  z-= tnz ; if(z<0)  z+= tnz ; 
            if(t>tnt-1)  t-= tnt ; if(t<0)  t+= tnt ; 

#endif                  

            parity = (x+y+z+t)%2; 
            // NOTICE: (x+y+z+t)%2 =/= (nd0+nd1+nd2+nd3)%2


            X = x + 1;
            Y = y + 1;
            Z = z + 1;
            T = t + 1;

            ////////X-oriented////////
            if(X == tnx){
                // x-oriented links on the boundary
                arg = (y+1-tny/2)*tnx*bz_quantum/(tnx*tny);
                arg+= (t+1-tnt/2)*tnx*ex_quantum/(tnx*tnt);
                arg-= (z+1-tnz/2)*by_quantum/(tnz*tnx);
            }
            else arg = -(z+1-tnz/2)*by_quantum/(tnz*tnx);

            arg *= ferm_charge;// only em phase so far
            if(KSphaseX(x,y,z,t) == -1) arg += 0.5;
            // ^this should be false, UNLESS one chooses
            // a different setup for the staggered phases....
            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.xmap*2+parity].d[idxh]= acc_twopi*arg; 


            ////////Y-oriented/////////
            if(Y == tny){
                // y-oriented links on the boundary
                arg = (z+1-tnz/2)*tny*bx_quantum/(tny*tnz);
                arg+= (t+1-tnt/2)*tny*ey_quantum/(tny*tnt);
                arg-= (x+1-tnx/2)*bz_quantum/(tnx*tny);
            }
            else arg = -(x+1)*bz_quantum/(tnx*tny);

            arg *= ferm_charge;// only am phase so far
            if(KSphaseY(x,y,z,t) == -1) arg += 0.5;
            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.ymap*2+parity].d[idxh]=acc_twopi*arg;


            ////////Z-oriented////////
            if(Z == tnz){
                // z-oriented links on the boundary
                arg =  (t+1-tnt/2)*tnz*ez_quantum/(tnz*tnt);
                arg += (x+1-tnx/2)*tnz*by_quantum/(tnz*tnx);
                arg -= (y+1-tny/2)*bx_quantum/(tny*tnz);
            }
            else arg = -(y+1-tny/2)*bx_quantum/(tny*tnz);

            arg *= ferm_charge;// only am phase so far
            if(KSphaseZ(x,y,z,t) == -1) arg += 0.5;
            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.zmap*2+parity].d[idxh]=acc_twopi*arg;


            ///////T-oriented////////
            arg = -(z+1-tnz/2)*ez_quantum/(tnz*tnt);
            arg -= (y+1-tny/2)*ey_quantum/(tny*tnt);
            arg -= (x+1-tnx/2)*ex_quantum/(tnx*tnt);

            arg *= ferm_charge;// only am phase so far
            if(KSphaseT(x,y,z,t) == -1) arg += 0.5;
            arg += chpotphase*0.5; // it must be multiplied by pi, not 2pi
            if(T == tnt) arg += 0.5;
            // antiperiodic boundary conds
            // notice T = t+1 !!

            while(arg > 0.5) arg -= 1.0;
            while(arg < -0.5) arg += 1.0;

            phases[geom_par.tmap*2+parity].d[idxh]=acc_twopi*arg;
        } // d3,d2,d1,d0 loops

}




void phase_diff_in_place(double_soa * inout, double_soa * sottraendo){
    int dir,idxh;

    for(dir=0;dir<8;dir++)
        for(idxh=0;idxh<sizeh;idxh++)
            inout[dir].d[idxh] -= sottraendo[dir].d[idxh];

}

void u1_diff(double_soa * out_re, double_soa * out_im,
        double_soa * phase_p,double_soa * phase_m ){

     int dir,idxh;

    for(dir=0;dir<8;dir++)
        for(idxh=0;idxh<sizeh;idxh++){
            out_re[dir].d[idxh] = cos(phase_p[dir].d[idxh]) - cos(phase_m[dir].d[idxh]);
            out_im[dir].d[idxh] = sin(phase_p[dir].d[idxh]) - sin(phase_m[dir].d[idxh]);
        }
    
}


void set_double_soa_to_zero(double_soa * p){
    int dir,idxh;

    for(dir=0;dir<8;dir++)
        for(idxh=0;idxh<sizeh;idxh++)
            p[dir].d[idxh]=0;
    
}





#endif

