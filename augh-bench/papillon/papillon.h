/**
 * ***********************************************************************
 * Papillon: ensemble de recherche distribué et multi-critères
 *
 * Auteur: Ivan auge (ivan.auge@ensiie.fr)
 * ***********************************************************************
**/

#include "config.h"

#ifndef FILE_PAPILLON_H
#define FILE_PAPILLON_H
typedef struct _Twdata {
        unsigned int associated_value;
        unsigned int offset;   // can be reduced
} Twdata;

typedef struct _Tnode {
    unsigned char freeRate,left_freeRate,right_freeRate;  // 0:free ; 255:completly full
    // data[0] is the length of the first record, 
    // data[1] is the vallue associated to the first record,
    // data[2:data[0]-1] is the key of the  first record.
    // The second record has the same structure and starts at data[data[0]],
    // be i=data[0], so
    // data[i] is the length of the second record, 
    // data[i+1] is the vallue associated to the second record,
    // data[i+2:data[i]-1] is the key of the  second record.
    // and so on for thr next records.
    unsigned int data[MEM_SIZE];
    unsigned int nbDataUsed;
    
    // working set for xquery request
    Twdata wdata[WORKSET_NBELE];
    unsigned int nbWDataUsed;
} Tnode;

typedef struct _Toperator {
    unsigned int cmd;
    unsigned char opeData[OPE_OpeDtSize];    // operator data
    unsigned char recData[OPE_RecDtSize];    // record data
} Toperator;


#endif
