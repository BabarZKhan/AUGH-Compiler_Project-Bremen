
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
	FILE* F;

	// The inputs
	F = fopen("input_vectors.bin", "wb");

	// Key input
	fprint_u32_lr(F, 0x00000001);
	fprint_u32_lr(F, 0x1b1a1918);
	fprint_u32_lr(F, 0x13121110);
	fprint_u32_lr(F, 0x0b0a0908);
	fprint_u32_lr(F, 0x03020100);

	// Encrypt
	fprint_u32_lr(F, 0x00000002);
	fprint_u32_lr(F, 0x656b696c);
	fprint_u32_lr(F, 0x20646e75);

	// Decrypt
	fprint_u32_lr(F, 0x00000004);
	fprint_u32_lr(F, 0x44c8fc20);
	fprint_u32_lr(F, 0xb9dfa07a);

	fclose(F);

	// The outputs
	F = fopen("output_vectors.bin", "wb");

	fprint_u32_lr(F, 0x44c8fc20);
	fprint_u32_lr(F, 0xb9dfa07a);

	fprint_u32_lr(F, 0x656b696c);
	fprint_u32_lr(F, 0x20646e75);

	fclose(F);

	return 0;
}

