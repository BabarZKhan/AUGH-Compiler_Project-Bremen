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
/*
 *  Transformation: JPEG -> BMP
 *  
 *  @(#) $Id: jpeg2bmp.c,v 1.2 2003/07/18 10:19:21 honda Exp $ 
 */

int jpeg2bmp_main () {
	int ci;
	unsigned char *c;
	unsigned i, j;

	#ifdef AUGH_SYNTHESIS

	// Read the buffer
	for (i = 0; i < JPEGSIZE; i++) {		JpegFileBuf[i] = read8();
	}

	// This reads the buffer JpegFileBuf
	jpeg_read();

	// Output image
	for (i = 0; i < RGB_NUM; i++) {
		for (j = 0; j < BMP_OUT_SIZE; j++) {
			write8(OutData_comp_buf[i][j]);
		}
	}
	// Also output image dimensions for test vectors
	write8(OutData_image_width);
	write8(OutData_image_height);

	#endif

	#ifdef GCC_EXEC

	/*
	 * Store input data in buffer
	 */
	c = JpegFileBuf;
	for (i = 0; i < JPEGSIZE; i++) {		ci = hana_jpg[i];
		*c++ = ci;
	}

	// This reads the buffer JpegFileBuf
	jpeg_read();

	for (i = 0; i < RGB_NUM; i++) {
		for (j = 0; j < BMP_OUT_SIZE; j++) {
			if (OutData_comp_buf[i][j] != hana_bmp[i][j]) {
				main_result++;
			}
		}
	}
	if (OutData_image_width != out_width) {
		main_result++;
	}
	if (OutData_image_height != out_length) {
		main_result++;
	}

	#endif

	return (0);
}
