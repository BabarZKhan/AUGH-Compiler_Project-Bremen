/**
 * ***********************************************************************
 * Papillon: ensemble de recherche distribué et multi-critères
 *
 * Auteur: Ivan auge (ivan.auge@ensiie.fr)
 * ***********************************************************************
**/

#ifndef FILE_CONFIG_H
#define FILE_CONFIG_H

#define MEM_SIZE    128    // size of memory data in word (4 bytes)
#define MAX_ELESIZE  16    // max size of an element in word (4 bytes)
#define MIN_ELESIZE  10    // min size of an element in word (4 bytes)

#define WORKSET_NBELE ((MEM_SIZE+11)/12) // # elements of work set

#define OPE_NB          4    // maximum number of operators in xquery request
#define OPE_OpeDtSize  64    // memory byte size of operator data
#define OPE_RecDtSize 128    // memory byte size of record data in operator

// Input/output Macros
#define PAP_READ(stream,buf,size) do {\
        if ( read(stream,buf,size)!=size ) { \
            fprintf(stderr,"FATAL:%s: read failure, exited\n",__func__); \
            exit(0); \
        } } while (0)
#define PAP_WRITE(stream,buf,size) do {\
        if ( write(stream,buf,size)!=size ) { \
            fprintf(stderr,"FATAL:%s: write failure, exited\n",__func__); \
            exit(0); \
        } } while (0)

#endif
