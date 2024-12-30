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

#ifdef AUGH_SYNTHESIS

#include <augh.h>

int8_t stdin;
int8_t stdout;

static inline uint8_t read8() {
	uint8_t u8 = 0;
	fifo_read(stdin, &u8);
	return u8;
}

static inline void write8(uint8_t u8) {
	fifo_write(stdout, &u8);
}

#define printf(...) do{}while(0)
#define exit(...) do{}while(0)

#include "global.h"
#include "decode.h"
#include "init.h"
#include "marker.c"
#include "chenidct.c"
#include "huffman.c"
#include "decode.c"
#include "jfif_read.c"
#include "jpeg2bmp.c"


void augh_main() {
	do {
		jpeg2bmp_main();
	} while(1);
}

#endif

#ifdef GCC_EXEC

#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "decode.h"
#include "init.h"
#include "marker.c"
#include "chenidct.c"
#include "huffman.c"
#include "decode.c"
#include "jfif_read.c"
#include "jpeg2bmp.c"


int main () {
	main_result = 0;

	jpeg2bmp_main ();

	printf ("%d\n", main_result);

	return main_result;
}

#endif

