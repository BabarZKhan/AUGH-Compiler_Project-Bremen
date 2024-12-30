
#include <stdio.h>
#include <stdint.h>


int main() {
	uint32_t seed = 0xABCDEF01;
	uint32_t factor = 17;

	// The key
	for(unsigned i=0; i<16; i++) {
		putchar(seed & 0x0FF);
		seed *= factor;
	}
	// The statement
	for(unsigned i=0; i<16; i++) {
		putchar(seed & 0x0FF);
		seed *= factor;
	}

	return 0;
}

