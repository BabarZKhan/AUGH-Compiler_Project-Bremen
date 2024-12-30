/*
+--------------------------------------------------------------------------+
| CHStone : a suite of benchmark programs for C-based High-Level Synthesis |
| ======================================================================== |
|                                                                          |
| * Collected and Modified : Y. Hara, H. Tomiyama, S. Honda,               |
|                            H. Takada and K. Ishii                        |
|                            Nagoya University, Japan                      |
|                                                                          |
| * Remark :                                                               |
|    1. This source code is modified to unify the formats of the benchmark |
|       programs in CHStone.                                               |
|    2. Test vectors are added for CHStone.                                |
|    3. If "main_result" is 0 at the end of the program, the program is    |
|       correctly executed.                                                |
|    4. Please follow the copyright of each benchmark program.             |
+--------------------------------------------------------------------------+
*/
/*
 * Copyright (C) 2008
 * Y. Hara, H. Tomiyama, S. Honda, H. Takada and K. Ishii
 * Nagoya University, Japan
 * All rights reserved.
 *
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis. The authors disclaims any and all warranties,
 * whether express, implied, or statuary, including any implied warranties or
 * merchantability or of fitness for a particular purpose. In no event shall the
 * copyright-holder be liable for any incidental, punitive, or consequential damages
 * of any kind whatsoever arising from the use of these programs. This disclaimer
 * of warranty extends to the user of these programs and user's customers, employees,
 * agents, transferees, successors, and assigns.
 *
 */

#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)

	#include <augh.h>

	int64_t stdin;
	int64_t stdout;

	static inline uint64_t read64() {
		uint64_t u64 = 0;
		fifo_read(stdin, &u64);
		return u64;
	}

	static inline void write64(uint64_t u64) {
		fifo_write(stdout, &u64);
	}

#endif

#ifdef GCC_COMPIL

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdint.h>

	// Read bytes left to right
	static inline uint64_t fifo_read64_lr() {
		uint64_t u64 = 0;
		for(unsigned i=0; i<8; i++) {
			int u8 = getchar();
			if(u8==EOF) exit(0);
			u64 = (u64 << 8) | u8;
		}
		return u64;
	}

	// Read bytes right to left
	static inline uint64_t fifo_read64_rl() {
		uint64_t u64 = 0;
		for(unsigned i=0; i<8; i++) {
			int u8 = getchar();
			if(u8==EOF) exit(0);
			u64 = ((uint64_t)u8 << 56) | (u64 >> 8);
		}
		return u64;
	}

	#define read64  fifo_read64_lr

	static inline void fifo_write64_lr(uint64_t u64) {
		for(unsigned i=0; i<8; i++) {
			uint8_t u8 = (u64 >> 56) & 0xFF;
			u64 = u64 << 8;
			//printf("%02x", u8);
			putchar(u8);
		}
	}

	#define write64  fifo_write64_lr

#endif

#include "softfloat.c"

#if defined(AUGH_SYNTHESIS)
void augh_main ()
#else
int main ()
#endif
{
	int main_result;
	int i;
	float64 result, x1, x2;

	// Infinite loop
	do {

		x1 = read64();
		x2 = read64();

		result = float64_add (x1, x2);

		write64(result);

	} while(1);

	#if !defined(AUGH_SYNTHESIS)
	return  0;
	#endif
}

