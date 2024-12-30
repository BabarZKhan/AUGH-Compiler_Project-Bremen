#ifndef _DEC_H
#define _DEC_H
#include <stdint.h>

typedef struct{
		int8_t xa; //systematic information bit 0
		int8_t xb;
		int8_t y1_0; //first redunduncy for decoder 0
		int8_t y1_1; // first redunduncy for decoder 1
		int8_t y2_0;
		int8_t y2_1;
	} decoder_input;

void DECODAGE(decoder_input input[],int16_t dec_out[][2]);

typedef enum {none,flipz,flipall} t_flip;

#endif
