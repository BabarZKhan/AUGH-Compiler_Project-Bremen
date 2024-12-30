/**
 * ***********************************************************************
 * Papillon: ensemble de recherche distribué et multi-critères
 *
 * Auteur: Ivan auge (ivan.auge@ensiie.fr)
 * ***********************************************************************
**/

#define SYNTHESIS

#include <augh.h>
#include <augh_annot.h>

#include "README.h"
#include "config.h"
//#include "papillon.h"


#undef PAP_READ
#undef PAP_WRITE

#define PAP_READ(stream,buf,size)  augh_read(stream##0,&buf)
#define PAP_WRITE(stream,buf,size) augh_write(stream##1,&buf)


uint32_t  stream0,stream1;               // streams to/from parent
#ifdef NODE
uint32_t  stream_left0, stream_left1;    // streams to/from left child
uint32_t  stream_right0, stream_right1;  // stream  to/from right child
#endif


// Tnode data;
unsigned char freeRate,left_freeRate,right_freeRate;  // 0:free ; 255:completly full
unsigned int data[MEM_SIZE];
unsigned int nbDataUsed;



static inline void papillon_add(unsigned int cmd);
static inline void papillon_query(unsigned int cmd);
#ifdef WITH_XQUERY
static void papillon_xquery(unsigned int cmd);
#endif

void augh_main()
{
    unsigned int cmd;

		freeRate=0;
		left_freeRate=0;
		right_freeRate=0;
		nbDataUsed=0;

    while (1) {
        PAP_READ(stream,cmd,4);
        switch ( CMD_CODOP(cmd) ) {
            case CMD_ADD_SHORT:
            case CMD_ADD_LONG:
               papillon_add(cmd);
               break;
            case CMD_QUERY:
augh_branch_prob=100;
               papillon_query(cmd);
               break;
#ifdef WITH_XQUERY
            case CMD_XQUERY:
               papillon_xquery(cmd);
               break;
							 #endif
#ifndef SYNTHESIS
            default:
            fprintf(stderr," 0x%08x is an invalid command (ignored)\n");
#endif
        }
    }
}

/*************************************************************************/

#define MIN(x,y)   ((x)<(y) ? (x) : (y))
#define MIN3(x,y,z) MIN(MIN(x,y),z)

unsigned int key[MAX_ELESIZE];

// return 0 if key matches data, 1 otherwise
static inline bool key_cmp(int len, unsigned int dataptr)
{
    bool ret=0;
    unsigned int i,k,x;

    for (i=0 ; i<len && ret==0 ; i+=1) {
#if 0
        unsigned int j=0xff; // joker
augh_iter_nb=4;
        k=key[i];
        x  = (k&j)==j ? j : data[dataptr+i]&j; j <<= 8;
        x |= (k&j)==j ? j : data[dataptr+i]&j; j <<= 8;
        x |= (k&j)==j ? j : data[dataptr+i]&j; j <<= 8;
        x |= (k&j)==j ? j : data[dataptr+i]&j;
//printf("key_cmp: i/len=%2d/%2d : key[i]=%08x ; data[i]=%08x ; x=%08x\n",i,len,k,data[i],x);
        if ( x!=k )
            ret = 1;
#else
#define J0 0xFF
#define J1 0xFF00
#define J2 0xFF0000
#define J3 0xFF000000
	augh_iter_nb=4;
        k=key[i];
        x  =  ((k&J0)==J0 ? J0 :(data[dataptr+i]&J0))
           |  ((k&J1)==J1 ? J1 :(data[dataptr+i]&J1))
           |  ((k&J2)==J2 ? J2 :(data[dataptr+i]&J2))
           |  ((k&J3)==J3 ? J3 :(data[dataptr+i]&J3));
        if ( x!=k )
            ret = 1;
#endif
    }
    return ret;
}

/*************************************************************************/

static inline void papillon_add(unsigned int cmd)
{
    unsigned int x;
    int nb = CMD_KEYSZ(cmd);

#ifdef NODE
    if ( freeRate<=left_freeRate &&
         freeRate<=right_freeRate ) {
#endif
        // add to current
        unsigned int associated_value;
        if ( CMD_CODOP(cmd) == CMD_ADD_SHORT )
            associated_value = CMD_SHORTVAL(cmd);
        else {
            PAP_READ(stream,associated_value,4);
        }
        data[nbDataUsed++] = nb+2;
        data[nbDataUsed++] = associated_value;
        while (nb>0) {
            nb-=1;
            PAP_READ(stream,data[nbDataUsed],4);
            nbDataUsed+=1;
        }
        freeRate = (256*nbDataUsed)/MEM_SIZE;
        if ( freeRate==255 && (MEM_SIZE-nbDataUsed+2)>=MAX_ELESIZE)
            freeRate=254;
        else if (  (MEM_SIZE-nbDataUsed+2)<MAX_ELESIZE)
            freeRate=255;
#ifdef NODE
    } else if ( left_freeRate<=right_freeRate ) {
        // add to left child
        PAP_WRITE(stream_left,cmd,4);
        if ( CMD_CODOP(cmd) == CMD_ADD_LONG )
            nb += 1;
        while ( nb!= 0 ) {
            nb-=1;
            PAP_READ(stream,x,4);
            PAP_WRITE(stream_left,x,4);
        }
        PAP_READ(stream_left,x,4);
        left_freeRate = CMD_FREERATE(x);
    } else {
        // add to right child
        PAP_WRITE(stream_right,cmd,4);
        if ( CMD_CODOP(cmd) == CMD_ADD_LONG )
            nb += 1;
        while ( nb!= 0 ) {
            nb-=1;
            PAP_READ(stream,x,4);
            PAP_WRITE(stream_right,x,4);
        }
        PAP_READ(stream_right,x,4);
        right_freeRate = CMD_FREERATE(x);
    }
#endif
#ifdef NODE
	if ( left_freeRate < freeRate && left_freeRate<right_freeRate )
    x = CMD_SET_CODOP_FREERATE(CMD_ADD_STATUS, left_freeRate);
	else if ( freeRate < left_freeRate && freeRate<right_freeRate )
    x = CMD_SET_CODOP_FREERATE(CMD_ADD_STATUS, freeRate);
	else
    x = CMD_SET_CODOP_FREERATE(CMD_ADD_STATUS, right_freeRate);
#else
    x = CMD_SET_CODOP_FREERATE(CMD_ADD_STATUS, freeRate);
#endif
    PAP_WRITE(stream,x,4);
}

static inline void papillon_query(unsigned int cmd)
{
    unsigned int result;
    // store searf key
    int keylen = CMD_KEYSZ(cmd);
    int pos    = CMD_SHORTVAL(cmd);
    int nb;
#ifdef NODE
    unsigned int x;
		bool end;
    PAP_WRITE(stream_left, cmd,4);
    PAP_WRITE(stream_right,cmd,4);
#endif
    for (nb=0 ; nb<keylen ; nb+=1 ) {
augh_iter_nb=4;
        PAP_READ(stream,key[nb],4);
#ifdef NODE
        PAP_WRITE(stream_left, key[nb],4);
        PAP_WRITE(stream_right,key[nb],4);
#endif
    }

//result = CMD_SET_CODOP_VALUE(CMD_RESULT,keylen&0xFFFFFF);
//PAP_WRITE(stream,result,4);
    // search for key in local data and send result to parent
    for (nb=0 ; data[nb]!=0 ; nb+=data[nb] ) {
        int datalen = data[nb] - 2;
        unsigned int  associated_value = data[nb+1];
        unsigned int  dataptr = nb+2+pos;
augh_iter_nb=100;
        if ( (pos+keylen) <= datalen ) {
	  			bool match;
augh_branch_prob = 99;
//result = CMD_SET_CODOP_VALUE(CMD_RESULT,data[dataptr]&0xFFFFFF);
//PAP_WRITE(stream,result,4);
					match=  key_cmp(keylen,dataptr);
        	if ( match==0 ) {
augh_branch_prob = 5;
        		// data is matching key, we send the associated_value to parent
        		result = CMD_SET_CODOP_VALUE(CMD_RESULT,associated_value);
        		PAP_WRITE(stream,result,4);
       		}
				}
    }
#ifndef NODE
    // signal to the parent that the query is completed.
    result = CMD_SET_CODOP(CMD_END);
    PAP_WRITE(stream,result,4);
#else
    // wait to left child end
    while (1) {
        PAP_READ(stream_left,x,4);
        if (CMD_CODOP(x)==CMD_END )
					break;
       	PAP_WRITE(stream,x,4);
    }

    // wait to right child end and signal to the parent that
    // the query is completed.
    while (1) {
        PAP_READ(stream_right,x,4);
        PAP_WRITE(stream,x,4);
        if (CMD_CODOP(x)==CMD_END )
					break;
    }
#endif
}

/*************************************************************************/
#ifdef WITH_XQUERY
#include "xquery.h"

static inline void papillon_xquery(unsigned int cmd, Tnode* data)
{
    int i,j;
    // get operators and transmit command to children
#ifdef NODE
    PAP_WRITE(stream_left, &cmd,4);
    PAP_WRITE(stream_right,&cmd,4);
#endif
    Toperator operators[OPE_NB];
    for (i=0 ; i<OPE_NB ; i++) {
        Toperator* ope=operators+i;
        if ( i>=CMD_NBOPE(cmd) ) {
            ope->cmd = CMD_LOGOP(cmd)==OPLOG_AND ? OPE_SET_FCT(OPE_FCT_TRUE)
                     : CMD_LOGOP(cmd)==OPLOG_OR  ? OPE_SET_FCT(OPE_FCT_FALSE)
                     : OPE_SET_FCT(OPE_FCT_FALSE);
            continue;
        }
        PAP_READ(stream,ope->cmd,4);
#ifdef NODE
        PAP_WRITE(stream_left, ope->cmd,4);
        PAP_WRITE(stream_right,ope->cmd,4);
#endif
        int nbDtWord = operateur_nbDtWord(ope->cmd);
        for (j=0 ; j<nbDtWord ; j+=1) {
            PAP_READ(stream,ope->opeData+4*j,4);
#ifdef NODE
            PAP_WRITE(stream_left, ope->opeData+4*j,4);
            PAP_WRITE(stream_right,ope->opeData+4*j,4);
#endif
        }
    }

    // do the works
    if ( CMD_SETOP(cmd)==OPSET_UNION )
        papillon_xquery_union(cmd,data,operators);
    else if ( CMD_SETOP(cmd)==OPSET_INTER )
        papillon_xquery_inter(cmd,data,operators);
#ifndef SYNTHESIS
    else {
        fprintf(stderr,"unexpected set operation %d\n",CMD_SETOP(cmd));
        exit(1);
    }
#endif

    // commit if needed
    if ( CMD_COMMIT(cmd) ) {
        unsigned int result;
#ifdef NODE
        // transmit result of left child
        while (1) {
            PAP_READ(stream_left,&result,4);
            if (CMD_CODOP(result)==CMD_END ) break;
            PAP_WRITE(stream,&result,4);
        }
        // transmit result of right child
        while (1) {
            PAP_READ(stream_right,&result,4);
            if (CMD_CODOP(result)==CMD_END ) break;
            PAP_WRITE(stream,&result,4);
        }
#endif
        // send our proper results
        for ( i=0 ; i<nbWDataUsed ; i+=1 ) {
            result = CMD_SET_CODOP_VALUE(CMD_RESULT,
                    wdata[i].associated_value);
            PAP_WRITE(stream,&result,4);
        }
        // signal the end of command
        result = CMD_SET_CODOP(CMD_END);
        PAP_WRITE(stream,&result,4);
        // ready for next xquery
        nbWDataUsed=0;
    }
}
#endif
