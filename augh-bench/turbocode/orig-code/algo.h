#ifndef ALGO_H
#define ALGO_H
#include <stdint.h>
#include "constante.h"
#include "decodage.h"

typedef enum {forward, backward} way_t;

#define MAX(x,y) ((x)>(y)?(x):(y))
void CALC_MET_NOEUD (decoder_input input, uint8_t dec, way_t sens, 
		int16_t gamma_intr[4],int16_t met_ext[4*H_TREILLIS],
		int16_t met_max_noeud[H_TREILLIS]);

void CALC_INFO_EXT (int16_t met_ext_forward[4*H_TREILLIS],int16_t
		mem_met_backward[H_TREILLIS],int16_t Z_out_f[4]);


void CALC_DECISION (int16_t gamma_intr[4], int16_t Z[4],int16_t dec[2]);

#endif
