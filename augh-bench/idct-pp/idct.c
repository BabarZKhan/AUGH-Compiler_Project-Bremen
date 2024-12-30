
#include <augh.h>

#ifdef AUGH_SYNTHESIS

	int32_t stdin;
	int8_t  stdout;

#endif

#ifdef GCC_COMPIL  // Standard compilation with gcc
	#include <stdio.h>
	#include <stdint.h>
	#include <stdbool.h>

	// Read bytes left to right
	static inline uint32_t fifo_read32_lr() {
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = getchar(); u32 = (u32 << 8) | u8; }
		return u32;
	}

	// Read bytes right to left
	static inline uint32_t fifo_read32_rl() {
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = getchar(); u32 = ((uint32_t)u8 << 24) | (u32 >> 8); }
		return u32;
	}

	static inline uint32_t fifo_read() {
		return fifo_read32_lr();
	}
	static inline void fifo_write(uint8_t ptr) {
		putchar(ptr);
	}

#endif


int32_t input[64];
uint8_t output[64];


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


int32_t idct_z1[64], idct_z2[64], idct_z3[64];

#define IDCT_ID(input, in_idx, output, out_idx) do { \
	/* Stage 1 */ \
	idct_z1[LINCOORD(in_idx, 1)] = input[LINCOORD(in_idx,0)] - input[LINCOORD(in_idx,4)]; \
	idct_z1[LINCOORD(in_idx, 0)] = input[LINCOORD(in_idx,0)] + input[LINCOORD(in_idx,4)]; \
	idct_z1[LINCOORD(in_idx, 2)] = PLUSHIFT(8867  * input[LINCOORD(in_idx,2)]) - PLUSHIFT(21407 * input[LINCOORD(in_idx,6)]); \
	idct_z1[LINCOORD(in_idx, 3)] = PLUSHIFT(21407 * input[LINCOORD(in_idx,2)]) - PLUSHIFT(8867  * input[LINCOORD(in_idx,6)]); \
	idct_z1[LINCOORD(in_idx, 4)] = input[LINCOORD(in_idx,1)] - input[LINCOORD(in_idx,7)]; \
	idct_z1[LINCOORD(in_idx, 7)] = input[LINCOORD(in_idx,1)] + input[LINCOORD(in_idx,7)]; \
	idct_z1[LINCOORD(in_idx, 5)] = PLUSHIFT(23170 * input[LINCOORD(in_idx,3)]); \
	idct_z1[LINCOORD(in_idx, 6)] = PLUSHIFT(23170 * input[LINCOORD(in_idx,5)]); \
	/* Stage 2 */ \
	idct_z2[LINCOORD(in_idx, 3)] = idct_z1[LINCOORD(in_idx, 0)] - idct_z1[LINCOORD(in_idx, 3)]; \
	idct_z2[LINCOORD(in_idx, 0)] = idct_z1[LINCOORD(in_idx, 0)] + idct_z1[LINCOORD(in_idx, 3)]; \
	idct_z2[LINCOORD(in_idx, 2)] = idct_z1[LINCOORD(in_idx, 1)] - idct_z1[LINCOORD(in_idx, 2)]; \
	idct_z2[LINCOORD(in_idx, 1)] = idct_z1[LINCOORD(in_idx, 1)] + idct_z1[LINCOORD(in_idx, 2)]; \
	idct_z2[LINCOORD(in_idx, 6)] = idct_z1[LINCOORD(in_idx, 4)] - idct_z1[LINCOORD(in_idx, 6)]; \
	idct_z2[LINCOORD(in_idx, 4)] = idct_z1[LINCOORD(in_idx, 4)] + idct_z1[LINCOORD(in_idx, 6)]; \
	idct_z2[LINCOORD(in_idx, 5)] = idct_z1[LINCOORD(in_idx, 7)] - idct_z1[LINCOORD(in_idx, 5)]; \
	idct_z2[LINCOORD(in_idx, 7)] = idct_z1[LINCOORD(in_idx, 7)] + idct_z1[LINCOORD(in_idx, 5)]; \
	/* Stage 3 */ \
	idct_z3[LINCOORD(in_idx, 0)] = idct_z2[LINCOORD(in_idx, 0)]; \
	idct_z3[LINCOORD(in_idx, 1)] = idct_z2[LINCOORD(in_idx, 1)]; \
	idct_z3[LINCOORD(in_idx, 2)] = idct_z2[LINCOORD(in_idx, 2)]; \
	idct_z3[LINCOORD(in_idx, 3)] = idct_z2[LINCOORD(in_idx, 3)]; \
	idct_z3[LINCOORD(in_idx, 4)] = PLUSHIFT(13623 * idct_z2[LINCOORD(in_idx, 4)]) - PLUSHIFT(9102  * idct_z2[LINCOORD(in_idx, 7)]); \
	idct_z3[LINCOORD(in_idx, 7)] = PLUSHIFT(9102  * idct_z2[LINCOORD(in_idx, 4)]) + PLUSHIFT(13623 * idct_z2[LINCOORD(in_idx, 7)]); \
	idct_z3[LINCOORD(in_idx, 5)] = PLUSHIFT(16069 * idct_z2[LINCOORD(in_idx, 5)]) - PLUSHIFT(3196  * idct_z2[LINCOORD(in_idx, 6)]); \
	idct_z3[LINCOORD(in_idx, 6)] = PLUSHIFT(3196  * idct_z2[LINCOORD(in_idx, 5)]) + PLUSHIFT(16069 * idct_z2[LINCOORD(in_idx, 6)]); \
	/* Stage 4 */ \
	output[LINCOORD(out_idx,7)] = idct_z3[LINCOORD(in_idx, 0)] - idct_z3[LINCOORD(in_idx, 7)]; \
	output[LINCOORD(out_idx,0)] = idct_z3[LINCOORD(in_idx, 0)] + idct_z3[LINCOORD(in_idx, 7)]; \
	output[LINCOORD(out_idx,6)] = idct_z3[LINCOORD(in_idx, 1)] - idct_z3[LINCOORD(in_idx, 6)]; \
	output[LINCOORD(out_idx,1)] = idct_z3[LINCOORD(in_idx, 1)] + idct_z3[LINCOORD(in_idx, 6)]; \
	output[LINCOORD(out_idx,5)] = idct_z3[LINCOORD(in_idx, 2)] - idct_z3[LINCOORD(in_idx, 5)]; \
	output[LINCOORD(out_idx,2)] = idct_z3[LINCOORD(in_idx, 2)] + idct_z3[LINCOORD(in_idx, 5)]; \
	output[LINCOORD(out_idx,4)] = idct_z3[LINCOORD(in_idx, 3)] - idct_z3[LINCOORD(in_idx, 4)]; \
	output[LINCOORD(out_idx,3)] = idct_z3[LINCOORD(in_idx, 3)] + idct_z3[LINCOORD(in_idx, 4)]; \
}while(0)

// Perform the 2D IDCT
// Input data is read from the array "input"
// Output data is written to the array "output"
static inline void IDCT_2D() {
	unsigned k, l;

	int32_t data2[64];
	int32_t data3[64];
	int32_t data4[64];
	int32_t data5[64];
	int32_t data6[64];

	// Scale
	for(k=0; k<64; k++) data2[k] = input[k] << 3;

	// First IDCT
	for(k=0; k<8; k++) {
		IDCT_ID(data2, k, data3, k);
	}

	// Transpose
	for(l=0; l<8; l++) {
		for(k=0; k<8; k++) {
			data4[LINCOORD(l, k)] = data3[LINCOORD(k, l)];
		}
	}

	// Second IDCT
	for(k=0; k<8; k++) {
		IDCT_ID(data4, k, data5, k);
	}

	// Descale
	for(k=0; k<8; k++) {
		for(l=0; l<8; l++) {
			//data6[LINCOORD(k, l)] = ((data5[LINCOORD(k, l)] + 0x20) >> 6) + 128;
			data6[LINCOORD(k, l)] = ((data5[LINCOORD(k, l)] + 0x20) >> 6) + 128;
		}
	}

	// Transpose and saturate
	for(k=0; k<8; k++) {
		for(l=0; l<8; l++) {
			#define src data6[LINCOORD(l, k)]
			#define dst output[LINCOORD(k, l)]

			if     (src<0)   dst = 0;
			else if(src>255) dst = 255;
			else             dst = src;

			#undef src
			#undef dst
		}
	}

}


#ifdef AUGH_SYNTHESIS
void augh_main ()
#endif
#ifdef GCC_COMPIL
int main ()
#endif
{
	unsigned k;

	do {

		fifo_read_vector(stdin, input, 64);

		IDCT_2D();

		fifo_write_vector(stdout, output, 64);

		#ifdef GCC_COMPIL
		break;
		#endif

	}while(1);

	#ifdef GCC_COMPIL
	return 0;
	#endif
}


