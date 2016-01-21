#ifndef DBGTOOLS_OPENACC_H_
#define DBGTOOLS_OPENACC_H_

#include "../OpenAcc/struct_c_def.h"

void print_vec3_soa(vec3_soa * const fermion, const char* nomefile);
int read_vec3_soa(vec3_soa * fermion, const char* nomefile);
void print_su3_soa(su3_soa * const conf, const char* nomefile,int conf_id_iter);
void print_1su3_soa(su3_soa * const conf, const char* nomefile);
void read_su3_soa(su3_soa * conf, const char* nomefile,int * conf_id_iter );
void print_tamat_soa(tamat_soa * const ipdot, const char* nomefile);
int read_tamat_soa(tamat_soa * ipdot, const char* nomefile);
void print_thmat_soa(thmat_soa * const ipdot, const char* nomefile);
void print_1thmat_soa(thmat_soa * const ipdot, const char* nomefile);
int read_thmat_soa(thmat_soa * ipdot, const char* nomefile);
void print_double_soa(double_soa * const backfield, const char* nomefile);
void print_1double_soa(double_soa * const vettore, const char* nomefile);

#endif
