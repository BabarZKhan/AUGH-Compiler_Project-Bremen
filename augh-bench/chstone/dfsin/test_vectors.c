
#include <stdio.h>
#include <stdint.h>


/*
+--------------------------------------------------------------------------+
| * Test Vectors (added for CHStone)                                       |
|     test_in : input data                                                 |
|     test_out : expected output data                                      |
+--------------------------------------------------------------------------+
*/
#define N 36
const uint64_t test_in[N] = {
	0x0000000000000000ULL,	/*      0  */
	0x3fc65717fced55c1ULL,	/*   PI/18 */
	0x3fd65717fced55c1ULL,	/*   PI/9  */
	0x3fe0c151fdb20051ULL,	/*   PI/6  */
	0x3fe65717fced55c1ULL,	/*  2PI/9  */
	0x3febecddfc28ab31ULL,	/*  5PI/18 */
	0x3ff0c151fdb20051ULL,	/*   PI/3  */
	0x3ff38c34fd4fab09ULL,	/*  7PI/18 */
	0x3ff65717fced55c1ULL,	/*  4PI/9  */
	0x3ff921fafc8b0079ULL,	/*   PI/2  */
	0x3ffbecddfc28ab31ULL,	/*  5PI/9  */
	0x3ffeb7c0fbc655e9ULL,	/* 11PI/18 */
	0x4000c151fdb20051ULL,	/*  2PI/3  */
	0x400226c37d80d5adULL,	/* 13PI/18 */
	0x40038c34fd4fab09ULL,	/*  7PI/9  */
	0x4004f1a67d1e8065ULL,	/*  5PI/6  */
	0x40065717fced55c1ULL,	/*  8PI/9  */
	0x4007bc897cbc2b1dULL,	/* 17PI/18 */
	0x400921fafc8b0079ULL,	/*   PI    */
	0x400a876c7c59d5d5ULL,	/* 19PI/18 */
	0x400becddfc28ab31ULL,	/* 10PI/9  */
	0x400d524f7bf7808dULL,	/*  7PI/6  */
	0x400eb7c0fbc655e9ULL,	/* 11PI/9  */
	0x40100e993dca95a3ULL,	/* 23PI/18 */
	0x4010c151fdb20051ULL,	/*  8PI/6  */
	0x4011740abd996affULL,	/* 25PI/18 */
	0x401226c37d80d5adULL,	/* 13PI/9  */
	0x4012d97c3d68405bULL,	/*  3PI/2  */
	0x40138c34fd4fab09ULL,	/* 14PI/9  */
	0x40143eedbd3715b7ULL,	/* 29PI/18 */
	0x4014f1a67d1e8065ULL,	/* 15PI/9  */
	0x4015a45f3d05eb13ULL,	/* 31PI/18 */
	0x40165717fced55c1ULL,	/* 16PI/9  */
	0x401709d0bcd4c06fULL,	/* 33PI/18 */
	0x4017bc897cbc2b1dULL,	/* 17PI/9  */
	0x40186f423ca395cbULL   /* 35PI/18 */
};

const uint64_t test_out[N] = {
	0x0000000000000000ULL,	/*  0.000000 */
	0x3fc63a1a335aadcdULL,	/*  0.173648 */
	0x3fd5e3a82b09bf3eULL,	/*  0.342020 */
	0x3fdfffff91f9aa91ULL,	/*  0.500000 */
	0x3fe491b716c242e3ULL,	/*  0.642787 */
	0x3fe8836f672614a6ULL,	/*  0.766044 */
	0x3febb67ac40b2bedULL,	/*  0.866025 */
	0x3fee11f6127e28adULL,	/*  0.939693 */
	0x3fef838b6adffac0ULL,	/*  0.984808 */
	0x3fefffffe1cbd7aaULL,	/*  1.000000 */
	0x3fef838bb0147989ULL,	/*  0.984808 */
	0x3fee11f692d962b4ULL,	/*  0.939693 */
	0x3febb67b77c0142dULL,	/*  0.866026 */
	0x3fe883709d4ea869ULL,	/*  0.766045 */
	0x3fe491b81d72d8e8ULL,	/*  0.642788 */
	0x3fe00000ea5f43c8ULL,	/*  0.500000 */
	0x3fd5e3aa4e0590c5ULL,	/*  0.342021 */
	0x3fc63a1d2189552cULL,	/*  0.173648 */
	0x3ea6aedffc454b91ULL,	/*  0.000001 */
	0xbfc63a1444ddb37cULL,	/* -0.173647 */
	0xbfd5e3a4e68f8f3eULL,	/* -0.342019 */
	0xbfdffffd494cf96bULL,	/* -0.499999 */
	0xbfe491b61cb9a3d3ULL,	/* -0.642787 */
	0xbfe8836eb2dcf815ULL,	/* -0.766044 */
	0xbfebb67a740aae32ULL,	/* -0.866025 */
	0xbfee11f5912d2157ULL,	/* -0.939692 */
	0xbfef838b1ac64afcULL,	/* -0.984808 */
	0xbfefffffc2e5dc8fULL,	/* -1.000000 */
	0xbfef838b5ea2e7eaULL,	/* -0.984808 */
	0xbfee11f7112dae27ULL,	/* -0.939693 */
	0xbfebb67c2c31cb4aULL,	/* -0.866026 */
	0xbfe883716e6fd781ULL,	/* -0.766045 */
	0xbfe491b9cd1b5d56ULL,	/* -0.642789 */
	0xbfe000021d0ca30dULL,	/* -0.500001 */
	0xbfd5e3ad0a69caf7ULL,	/* -0.342021 */
	0xbfc63a23c48863ddULL   /* -0.173649 */
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
		fprint_u64_lr(inF, test_in[i]);
		fprint_u64_lr(outF, test_out[i]);
	}

	return 0;
}

