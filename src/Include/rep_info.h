#ifndef rep_info_h
#define rep_info_h

#define MAXCRLENGTH 300

typedef struct rep_info_t{
    
    int replicas_total_number;
    double cr_vet [MAXCRLENGTH];
    
    
}rep_info;
extern rep_info *rep;


#endif /* rep_info_h */
