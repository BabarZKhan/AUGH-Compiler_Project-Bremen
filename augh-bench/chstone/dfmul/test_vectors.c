
#include <stdio.h>
#include <stdint.h>


/*
+--------------------------------------------------------------------------+
| * Test Vectors (added for CHStone)                                       |
|     a_input, b_input : input data                                        |
|     z_output : expected output data                                      |
+--------------------------------------------------------------------------+
*/
#define N 20
const uint64_t a_input[N] = {
	0x7FF0000000000000ULL,	/* inf */
	0x7FFF000000000000ULL,	/* nan */
	0x7FF0000000000000ULL,	/* inf */
	0x7FF0000000000000ULL,	/* inf */
	0x3FF0000000000000ULL,	/* 1.0 */
	0x0000000000000000ULL,	/* 0.0 */
	0x3FF0000000000000ULL,	/* 1.0 */
	0x0000000000000000ULL,	/* 0.0 */
	0x8000000000000000ULL,	/* -0.0 */
	0x3FF0000000000000ULL,	/* 1.0 */
	0x3FF0000000000000ULL,	/* 1.0 */
	0x4000000000000000ULL,	/* 2.0 */
	0x3FD0000000000000ULL,	/* 0.25 */
	0xC000000000000000ULL,	/* -2.0 */
	0xBFD0000000000000ULL,	/* -0.25 */
	0x4000000000000000ULL,	/* 2.0 */
	0xBFD0000000000000ULL,	/* -0.25 */
	0xC000000000000000ULL,	/* -2.0 */
	0x3FD0000000000000ULL,	/* 0.25 */
	0x0000000000000000ULL		/* 0.0 */
};

const uint64_t b_input[N] = {
	0xFFFFFFFFFFFFFFFFULL,	/* nan */
	0xFFF0000000000000ULL,	/* -inf */
	0x0000000000000000ULL,	/* nan */
	0x3FF0000000000000ULL,	/* -inf */
	0xFFFF000000000000ULL,	/* nan */
	0x7FF0000000000000ULL,	/* inf */
	0x7FF0000000000000ULL,	/* inf */
	0x3FF0000000000000ULL,	/* 1.0 */
	0x3FF0000000000000ULL,	/* 1.0 */
	0x0000000000000000ULL,	/* 0.0 */
	0x8000000000000000ULL,	/* -0.0 */
	0x3FD0000000000000ULL,	/* 0.25 */
	0x4000000000000000ULL,	/* 2.0 */
	0xBFD0000000000000ULL,	/* -0.25 */
	0xC000000000000000ULL,	/* -2.0 */
	0xBFD0000000000000ULL,	/* -0.25 */
	0x4000000000000000ULL,	/* -2.0 */
	0x3FD0000000000000ULL,	/* 0.25 */
	0xC000000000000000ULL,	/* -2.0 */
	0x0000000000000000ULL		/* 0.0 */
};

const uint64_t z_output[N] = {
	0xFFFFFFFFFFFFFFFFULL,	/* nan */
	0x7FFF000000000000ULL,	/* nan */
	0x7FFFFFFFFFFFFFFFULL,	/* nan */
	0x7FF0000000000000ULL,	/* inf */
	0xFFFF000000000000ULL,	/* nan */
	0x7FFFFFFFFFFFFFFFULL,	/* nan */
	0x7FF0000000000000ULL,	/* inf */
	0x0000000000000000ULL,	/* 0.0 */
	0x8000000000000000ULL,	/* -0.0 */
	0x0000000000000000ULL,	/* 0.0 */
	0x8000000000000000ULL,	/* -0.0 */
	0x3FE0000000000000ULL,	/* 0.5 */
	0x3FE0000000000000ULL,	/* 0.5 */
	0x3FE0000000000000ULL,	/* 0.5 */
	0x3FE0000000000000ULL,	/* 0.5 */
	0xBFE0000000000000ULL,	/* -0.5 */
	0xBFE0000000000000ULL,	/* -0.5 */
	0xBFE0000000000000ULL,	/* -0.5 */
	0xBFE0000000000000ULL,	/* -0.5 */
	0x0000000000000000ULL		/* 0.0 */
};


void fprint_u64_lr (FILE* F, uint64_t u64) {
	fputc(u64 >> 56, F);
	fputc(u64 >> 48, F);
	fputc(u64 >> 40, F);
	fputc(u64 >> 32, F);
	fputc(u64 >> 24, F);
	fputc(u64 >> 16, F);
	fputc(u64 >> 8, F);
	fputc(u64, F);
}
void fprint_u64_rl (FILE* F, uint64_t u64) {
	fputc(u64, F);
	fputc(u64 >> 8, F);
	fputc(u64 >> 16, F);
	fputc(u64 >> 24, F);
	fputc(u64 >> 32, F);
	fputc(u64 >> 40, F);
	fputc(u64 >> 48, F);
	fputc(u64 >> 56, F);
}


int main() {

	FILE* inF = fopen("input_vectors.bin", "wb");
	FILE* outF = fopen("output_vectors.bin", "wb");
	if(inF==NULL || outF==NULL) return 1;

	for(unsigned i=0; i<N; i++) {
		fprint_u64_lr(inF, a_input[i]);
		fprint_u64_lr(inF, b_input[i]);
		fprint_u64_lr(outF, z_output[i]);
	}

	return 0;
}

