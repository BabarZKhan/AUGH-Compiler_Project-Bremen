
#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)
	#include <augh.h>

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


int32_t idct_z1[8], idct_z2[8], idct_z3[8];

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


// Don't reuse the same temp reg in loop
//#define OPTIM_TMPREG
// Don't use same input and output array for second idct
//#define OPTIM_YC

// Perform the 2D IDCT
// Input data is read from the array "input"
// Output data is written to the array "output"
void IDCT_2D() {
	unsigned k, l;

	#ifndef OPTIM_TMPREG
		int32_t r;
	#else
		int32_t r[8];
		#define r r[k]
	#endif

	#ifndef OPTIM_YC
		int32_t Yc[8];
		#define Yc2 Yc
	#else
		int32_t Yc[8];
		int32_t Yc2[8];
	#endif

	// Scale
	for(k=0; k<64; k++) input[k] = input[k] << 3;

	// First IDCT
	for(k=0; k<8; k++) {
		IDCT_ID(input, k, input, k);
	}

	// Second IDCT
	for(l=0; l<8; l++) {
		// Transpose
		for(k=0; k<8; k++) Yc[k] = input[LINCOORD(k, l)];

		IDCT_ID(Yc, 0, Yc2, 0);

		// Descale and saturate
		for(k=0; k<8; k++) {
			r = ((Yc2[k] + 0x20) >> 6) + 128;
			if(r<0)        output[LINCOORD(k, l)] = 0;
			else if(r>255) output[LINCOORD(k, l)] = 255;
			else           output[LINCOORD(k, l)] = r;
		}

	}

	#ifdef r
		#undef r
	#endif

	#ifdef Yc2
		#undef Yc2
	#endif
}


#ifdef AUGH_SYNTHESIS
void augh_main ()
#endif
#ifdef GCC_COMPIL
int main ()
#endif
{
	unsigned k;

	#ifdef ASKSEND
	read8();
	#endif

	do {

		for (k = 0; k < 64; k++) input[k] = read32();

		IDCT_2D();

		for (k = 0; k < 64; k++) write8(output[k]);

		#ifdef GCC_COMPIL
		break;
		#endif

	}while(1);

	#ifdef GCC_COMPIL
	return 0;
	#endif
}



#ifdef GCC_EXEC

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct timespec shrtime;

double TimeSpec_GetDiff_Double(struct timespec *oldtime, struct timespec *newtime) {
	return (double)(newtime->tv_sec - oldtime->tv_sec) +
	       (double)(newtime->tv_nsec - oldtime->tv_nsec)/1.0e9;
}

double Time_GetReal() {
	struct timespec newtime;
	clock_gettime(CLOCK_REALTIME, &newtime);
	return TimeSpec_GetDiff_Double(&shrtime, &newtime);
}

#define N 1000000

int main () {

	printf("Init vectors...\n");

	int32_t* tv = malloc(N*64*sizeof(*tv));
	for(unsigned i=0; i<N*64; i++) tv[i] = rand();

	int8_t* tvo = malloc(N*64*sizeof(*tvo));
	for(unsigned i=0; i<N*64; i++) tvo[i] = rand();

	unsigned k;

	printf("Launch computing...\n");

	// Initialize time
	clock_gettime(CLOCK_REALTIME, &shrtime);

	// Launch the big computing
	for(unsigned i=0; i<N; i++) {
		for (k = 0; k < 64; k++) input[k] = tv[N*64 + k];

		IDCT_2D();

		for (k = 0; k < 64; k++) tvo[N*64 + k] = output[k];
	}

	// Get computing time
	double t = Time_GetReal();

	// Print time

	printf("Execute %u 2D-idct : %g seconds\n", N, t);
	printf("  Average %g per second\n", N / t);

	return 0;
}

#endif


