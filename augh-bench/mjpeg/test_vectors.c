
#include <stdio.h>
#include <stdint.h>


#define M_SOI		0xd8		/* Start of image */
#define M_EOI		0xd9		/* End Of Image */
#define M_SMS		0xff		/* Start Marker Segment */


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
	int c1, c2;

	do {
		c1 = getchar();
		if(c1==EOF) return -1;
		if(c1!=M_SMS) continue;
		c2 = getchar();
		if(c2==EOF) return -1;
		if(c2==M_SOI) {
			putchar(c1);
			putchar(c2);
			break;
		}
	}while(1);

	do {
		c1 = getchar();
		if(c1==EOF) return -1;
		putchar(c1);
		if(c1!=M_SMS) continue;
		c2 = getchar();
		if(c2==EOF) return -1;
		putchar(c2);
		if(c2==M_EOI) break;
	}while(1);

	return 0;
}

