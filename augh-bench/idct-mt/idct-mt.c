
#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)
	#include <augh.h>
	#include <augh_annot.h>

	int32_t stdin;
	int8_t  stdout;

	static inline uint32_t read32() {
		uint32_t u32 = 0;
		fifo_read(stdin, &u32);
		return u32;
	}

	static inline void write8(uint8_t u8) {
		fifo_write(stdout, &u8);
	}

#endif

#if defined(AUGH_SYNTHESIS) && defined(STDIO8)

	#include "../stdio8_defs.h"

	#define read32 read32_be

#endif

#ifdef GCC_EXEC
	#define GCC_COMPIL
#endif

#ifdef GCC_COMPIL  // Standard compilation with gcc
	#include <stdio.h>
	#include <stdint.h>
	#include <stdbool.h>

	// Read bytes left to right
	static inline uint32_t read32_lr() {
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = getchar(); u32 = (u32 << 8) | u8; }
		return u32;
	}

	// Read bytes right to left
	static inline uint32_t read32_rl() {
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = getchar(); u32 = ((uint32_t)u8 << 24) | (u32 >> 8); }
		return u32;
	}

	static inline uint32_t read32() {
		return read32_lr();
	}
	static inline void write8(uint8_t u8) {
		putchar(u8);
	}

#endif

#ifdef GCC_EXEC
	#undef GCC_COMPIL
#endif


int32_t input[64];
int32_t pass1[64];
uint8_t output[64];

static bool sync1;
static bool sync2;
static bool sync3;

// This is the actual code
//#define PLUSHIFT(x) ((((x)>>13)+1)>>1)

// These are tests because the above macro makes things ugly
//#define PLUSHIFT(x) ((((x)>>13)+1))
//#define PLUSHIFT(x) ((x)>>14)
//#define PLUSHIFT(x) ((x)+1)
#define PLUSHIFT(x) (x)

// Access 1D arrays with 2D coordinates
//#define LINCOORD(x,y) (((x)<<3)+(y))
//#define LINCOORD(x,y) (((x)<<3)|(y))
#define LINCOORD(x,y) (((x)<<3)|((y)&0x7))



#define IDCT_ID(input, in_idx, output, out_idx) do { \
	/* Stage 1 */ \
	idct_z1[1] = input[LINCOORD(in_idx,0)] - input[LINCOORD(in_idx,4)]; \
	idct_z1[0] = input[LINCOORD(in_idx,0)] + input[LINCOORD(in_idx,4)]; \
	idct_z1[2] = PLUSHIFT(8867  * input[LINCOORD(in_idx,2)]) - PLUSHIFT(21407 * input[LINCOORD(in_idx,6)]); \
	idct_z1[3] = PLUSHIFT(21407 * input[LINCOORD(in_idx,2)]) - PLUSHIFT(8867  * input[LINCOORD(in_idx,6)]); \
	idct_z1[4] = input[LINCOORD(in_idx,1)] - input[LINCOORD(in_idx,7)]; \
	idct_z1[7] = input[LINCOORD(in_idx,1)] + input[LINCOORD(in_idx,7)]; \
	idct_z1[5] = PLUSHIFT(23170 * input[LINCOORD(in_idx,3)]); \
	idct_z1[6] = PLUSHIFT(23170 * input[LINCOORD(in_idx,5)]); \
	/* Stage 2 */ \
	idct_z2[3] = idct_z1[0] - idct_z1[3]; \
	idct_z2[0] = idct_z1[0] + idct_z1[3]; \
	idct_z2[2] = idct_z1[1] - idct_z1[2]; \
	idct_z2[1] = idct_z1[1] + idct_z1[2]; \
	idct_z2[6] = idct_z1[4] - idct_z1[6]; \
	idct_z2[4] = idct_z1[4] + idct_z1[6]; \
	idct_z2[5] = idct_z1[7] - idct_z1[5]; \
	idct_z2[7] = idct_z1[7] + idct_z1[5]; \
	/* Stage 3 */ \
	idct_z3[0] = idct_z2[0]; \
	idct_z3[1] = idct_z2[1]; \
	idct_z3[2] = idct_z2[2]; \
	idct_z3[3] = idct_z2[3]; \
	idct_z3[4] = PLUSHIFT(13623 * idct_z2[4]) - PLUSHIFT(9102  * idct_z2[7]); \
	idct_z3[7] = PLUSHIFT(9102  * idct_z2[4]) + PLUSHIFT(13623 * idct_z2[7]); \
	idct_z3[5] = PLUSHIFT(16069 * idct_z2[5]) - PLUSHIFT(3196  * idct_z2[6]); \
	idct_z3[6] = PLUSHIFT(3196  * idct_z2[5]) + PLUSHIFT(16069 * idct_z2[6]); \
	/* Stage 4 */ \
	output[LINCOORD(out_idx,7)] = idct_z3[0] - idct_z3[7]; \
	output[LINCOORD(out_idx,0)] = idct_z3[0] + idct_z3[7]; \
	output[LINCOORD(out_idx,6)] = idct_z3[1] - idct_z3[6]; \
	output[LINCOORD(out_idx,1)] = idct_z3[1] + idct_z3[6]; \
	output[LINCOORD(out_idx,5)] = idct_z3[2] - idct_z3[5]; \
	output[LINCOORD(out_idx,2)] = idct_z3[2] + idct_z3[5]; \
	output[LINCOORD(out_idx,4)] = idct_z3[3] - idct_z3[4]; \
	output[LINCOORD(out_idx,3)] = idct_z3[3] + idct_z3[4]; \
}while(0)


// Read data from fifo, write it in array "input"
void thread_input() {
	unsigned k;

	bool sync_data = 0;

	augh_toplevel = 1;

	#ifdef ASKSEND
	read8();
	#endif

	do {

		for (k = 0; k < 64; k++) input[k] = read32();

		fifo_write(sync1, &sync_data);

	} while(1);

}

// IDCT 1D, first pass
// Input data is read from the array "input"
// Output data is written to the array "pass1"
void thread_pass1() {
	unsigned k, i;

	int32_t tmp[8];
	int32_t idct_z1[8], idct_z2[8], idct_z3[8];

	bool sync_data = 0;

	augh_toplevel = 1;

	do {

		fifo_read(sync1, &sync_data);

		for(k=0; k<8; k++) {

			// Scale
			for(i=0; i<8; i++) tmp[i] = input[LINCOORD(k, i)] << 3;

			// IDCT
			IDCT_ID(tmp, 0, pass1, k);

		}

		fifo_write(sync2, &sync_data);

	} while(1);

}

// IDCT 1D, second pass
// Input data is read from the array "pass1"
// Output data is written to the array "output"
void thread_pass2() {
	unsigned k, l;

	int32_t r;
	int32_t Yc[8], Yc2[8];
	int32_t idct_z1[8], idct_z2[8], idct_z3[8];

	bool sync_data = 0;

	augh_toplevel = 1;

	do {

		fifo_read(sync2, &sync_data);

		for(l=0; l<8; l++) {

			// Transpose
			for(k=0; k<8; k++) Yc[k] = pass1[LINCOORD(k, l)];

			// IDCT
			IDCT_ID(Yc, 0, Yc2, 0);

			// Descale and saturate
			for(k=0; k<8; k++) {
				r = ((Yc2[k] + 0x20) >> 6) + 128;
				if(r<0)        output[LINCOORD(k, l)] = 0;
				else if(r>255) output[LINCOORD(k, l)] = 255;
				else           output[LINCOORD(k, l)] = r;
			}

		}

		fifo_write(sync3, &sync_data);

	} while(1);

}

// Read data from array "output", write it to output fifo
void thread_output() {
	unsigned k;

	bool sync_data;

	augh_toplevel = 1;

	do {

		fifo_read(sync3, &sync_data);

		for (k = 0; k < 64; k++) write8(output[k]);

	} while(1);

}


