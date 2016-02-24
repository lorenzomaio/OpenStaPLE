#ifndef DBGTOOLS_OPENACC_C_
#define DBGTOOLS_OPENACC_C_

#include <complex.h>

#include "../OpenAcc/geometry.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include "../Include/common_defines.h"
#include "../OpenAcc/struct_c_def.h"
#include "../OpenAcc/single_types.h"
#include "../OpenAcc/io.h"

void print_vec3_soa(vec3_soa * const fermion, const char* nomefile)
{

    FILE *fp;
    fp = fopen(nomefile,"w");
    for(int i = 0 ; i < sizeh ; i++){
        fprintf(fp, "%.18lf\t%.18lf\n",creal(fermion->c0[i]),cimag(fermion->c0[i]));
        fprintf(fp, "%.18lf\t%.18lf\n",creal(fermion->c1[i]),cimag(fermion->c1[i]));
        fprintf(fp, "%.18lf\t%.18lf\n",creal(fermion->c2[i]),cimag(fermion->c2[i]));
    }
    fclose(fp);

}




int read_vec3_soa(vec3_soa * fermion, const char* nomefile)
{


    FILE *fp;
    fp = fopen(nomefile,"r");
    if(!fp){
        printf("vec3_soa File %s not found.\n", nomefile );
        return 1;
    }
    else{
        if(verbosity_lv > 2) 
            printf("Reading vec3_soa %s\n", nomefile );

    for(int i = 0 ; i < sizeh ; i++){
        double re,im;
        CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);fermion->c0[i] = re + im * I;
        CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);fermion->c1[i] = re + im * I;
        CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);fermion->c2[i] = re + im * I;
    }
    fclose(fp);
    return 0;
    }

}

void print_1su3_soa(su3_soa * const conf, const char* nomefile)
{

    FILE *fp;
    fp = fopen(nomefile,"w");
    for(int q = 0 ; q < 1 ; q++){ // q = 0 !!!!!
        for(int i = 0 ; i < sizeh ; i++){
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r0.c0[i]),cimag(conf[q].r0.c0[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r0.c1[i]),cimag(conf[q].r0.c1[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r0.c2[i]),cimag(conf[q].r0.c2[i]));//
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r1.c0[i]),cimag(conf[q].r1.c0[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r1.c1[i]),cimag(conf[q].r1.c1[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r1.c2[i]),cimag(conf[q].r1.c2[i]));//
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r2.c0[i]),cimag(conf[q].r2.c0[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r2.c1[i]),cimag(conf[q].r2.c1[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(conf[q].r2.c2[i]),cimag(conf[q].r2.c2[i]));
        }
    }
    fclose(fp);
}


void print_tamat_soa(tamat_soa * const ipdot, const char* nomefile)
{
    FILE *fp;
    fp = fopen(nomefile,"w");
    for(int q = 0 ; q < 8 ; q++){
        for(int i = 0 ; i < sizeh ; i++){
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c01[i]),cimag(ipdot[q].c01[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c02[i]),cimag(ipdot[q].c02[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c12[i]),cimag(ipdot[q].c12[i]));//
            fprintf(fp, "%.18lf\n",ipdot[q].rc00[i]);
            fprintf(fp, "%.18lf\n",ipdot[q].rc11[i]);
        }
    }
    fclose(fp);
}

int read_tamat_soa(tamat_soa * ipdot, const char* nomefile)
{

    FILE *fp;
    fp = fopen(nomefile,"r");
    if(!fp){
        printf("Tamat_soa File %s not found.\n", nomefile );
        return 1;
    }
    else{
    for(int q = 0 ; q < 8 ; q++){
        for(int i = 0 ; i < sizeh ; i++){
            double re,im;
            CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);ipdot[q].c01[i] = re + im * I;
            CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);ipdot[q].c02[i] = re + im * I;
            CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);ipdot[q].c12[i] = re + im * I;//
            CHECKREAD(fscanf(fp, "%lf\n",&(ipdot[q].rc00[i])),2);
            CHECKREAD(fscanf(fp, "%lf\n",&(ipdot[q].rc11[i])),2);
        }
    }
    fclose(fp);
    return 0;
    }
}


void print_thmat_soa(thmat_soa * const ipdot, const char* nomefile)
{
    FILE *fp;
    fp = fopen(nomefile,"w");
    for(int q = 0 ; q < 8 ; q++){
        for(int i = 0 ; i < sizeh ; i++){
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c01[i]),cimag(ipdot[q].c01[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c02[i]),cimag(ipdot[q].c02[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c12[i]),cimag(ipdot[q].c12[i]));//
            fprintf(fp, "%.18lf\n",ipdot[q].rc00[i]);
            fprintf(fp, "%.18lf\n",ipdot[q].rc11[i]);
        }
    }
    fclose(fp);
}
void print_1thmat_soa(thmat_soa * const ipdot, const char* nomefile)
{
    FILE *fp;
    fp = fopen(nomefile,"w");
    for(int q = 0 ; q < 1 ; q++){// q = 1 !!!!
        for(int i = 0 ; i < sizeh ; i++){
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c01[i]),cimag(ipdot[q].c01[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c02[i]),cimag(ipdot[q].c02[i]));
            fprintf(fp, "%.18lf\t%.18lf\n",creal(ipdot[q].c12[i]),cimag(ipdot[q].c12[i]));//
            fprintf(fp, "%.18lf\n",ipdot[q].rc00[i]);
            fprintf(fp, "%.18lf\n",ipdot[q].rc11[i]);
        }
    }
    fclose(fp);
}

int read_thmat_soa(thmat_soa * ipdot, const char* nomefile)
{

    FILE *fp;
    fp = fopen(nomefile,"r");
    if(!fp){
        printf("Thmat_soa File %s not found.\n", nomefile );
        return 1;
    }
    else{
    for(int q = 0 ; q < 8 ; q++){
        for(int i = 0 ; i < sizeh ; i++){
            double re,im;
            CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);ipdot[q].c01[i] = re + im * I;
            CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);ipdot[q].c02[i] = re + im * I;
            CHECKREAD(fscanf(fp, "%lf\t%lf\n",&re,&im),2);ipdot[q].c12[i] = re + im * I;//
            CHECKREAD(fscanf(fp, "%lf\n",&(ipdot[q].rc00[i])),1);
     	    CHECKREAD(fscanf(fp, "%lf\n",&(ipdot[q].rc11[i])),1);
        }
    }
    fclose(fp);
    return 0;
    }
}



void print_double_soa(double_soa * const backfield, const char* nomefile)
{


    FILE *fp;
    fp = fopen(nomefile,"w");
    for(int dir = 0 ; dir < 4 ; dir++)
        for(int t = 0 ; t <geom_par.gnt ; t++)
        for(int z = 0 ; z <geom_par.gnz ; z++)
        for(int y = 0 ; y <geom_par.gny ; y++)
        for(int x = 0 ; x <geom_par.gnx ; x++){
            int parity = (x+y+z+t)%2;
            int xyzt[4] = {x,y,z,t};
            int d0 = xyzt[geom_par.d0123map[0]];
            int d1 = xyzt[geom_par.d0123map[1]];
            int d2 = xyzt[geom_par.d0123map[2]];
            int d3 = xyzt[geom_par.d0123map[3]];

            int idxh = snum_acc(d0,d1,d2,d3);

            fprintf(fp, "dir: %d (x,y,z,t): %d %d %d %d value: %.18lf\n",dir, x,y,z,t,backfield[geom_par.xyztmap[dir]+parity].d[idxh]);
    }
    fclose(fp);
}

void print_1double_soa(double_soa * const vettore, const char* nomefile)
{

    FILE *fp;
    fp = fopen(nomefile,"w");
    for(int i = 0 ; i < sizeh ; i++){
      fprintf(fp, "%.18lf\n",vettore->d[i]);
    }
    fclose(fp);
}


void transpose_vec3_soa(vec3_soa * vecin, vec3_soa *vecout, int xmap, int ymap, int zmap, int tmap){

// WARNING : NOT TESTED
    int x,y,z,t;
    int d[4], idxh, newidxh;

    for(d[3]=0; d[3] < nd3; d[3]++) for(d[2]=0; d[2] < nd2; d[2]++)
        for(d[1]=0; d[1] < nd1; d[1]++) for(d[0]=0; d[0] < nd0; d[0]++){

                    idxh = snum_acc(d[0],d[1],d[2],d[3]);
                    int tnd[4] = {nd0,nd1,nd2,nd3};
                    x = d[xmap];int tnx = tnd[xmap] ;
                    y = d[ymap];int tny = tnd[ymap] ;
                    z = d[zmap];int tnz = tnd[zmap] ;
                    t = d[tmap];int tnt = tnd[tmap] ;
                    newidxh = (x+tnx*(y+tny*(z+tnz*t)))/2;
                    
                    vecout->c0[newidxh] = vecin->c0[idxh];
                    vecout->c1[newidxh] = vecin->c1[idxh];
                    vecout->c2[newidxh] = vecin->c2[idxh];

        }

}

void transpose_su3_soa(su3_soa * matin, su3_soa *matout, int xmap, int ymap, int zmap, int tmap){

// WARNING : NOT TESTED
    int dir,parity;
    for(dir = 1; dir < 4 ; dir++) for(parity = 1; parity < 2 ; parity++)
    {
        int dirmod = geom_par.xyztmap[dir];
        transpose_vec3_soa(&(matin[2*dirmod+parity].r0),
                &(matout[2*dir+parity].r0),xmap,ymap,zmap,tmap);
        transpose_vec3_soa(&(matin[2*dirmod+parity].r1),
                &(matout[2*dir+parity].r1),xmap,ymap,zmap,tmap);
        transpose_vec3_soa(&(matin[2*dirmod+parity].r2),
                &(matout[2*dir+parity].r2),xmap,ymap,zmap,tmap);

    }

}


#endif
