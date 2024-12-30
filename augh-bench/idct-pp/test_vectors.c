
#include <stdio.h>
#include <stdint.h>


void fprint_u32_lr (FILE* F, uint32_t u32) {
	fputc(u32 >> 24, F);
	fputc(u32 >> 16, F);
	fputc(u32 >> 8, F);
	fputc(u32, F);
}
void fprint_u32_rl (FILE* F, uint32_t u32) {
	fputc(u32, F);
	fputc(u32 >> 8, F);
	fputc(u32 >> 16, F);
	fputc(u32 >> 24, F);
}


int main() {
	uint32_t seed = 0xABCDEF01;
	uint32_t factor = 17;

	for(unsigned i=0; i<64; i++) {
		fprint_u32_lr(stdout, seed & 0x000000FF);
		seed *= factor;
	}

	return 0;
}

