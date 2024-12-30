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
/*************************************************************
Copyright (C) 1990, 1991, 1993 Andy C. Hung, all rights reserved.
PUBLIC DOMAIN LICENSE: Stanford University Portable Video Research
Group. If you use this software, you agree to the following: This
program package is purely experimental, and is licensed "as is".
Permission is granted to use, modify, and distribute this program
without charge for any purpose, provided this license/ disclaimer
notice appears in the copies.  No warranty or maintenance is given,
either expressed or implied.  In no event shall the author(s) be
liable to you or a third party for any special, incidental,
consequential, or other damages, arising out of the use or inability
to use the program for any purpose (or the loss of data), even if we
have been advised of such possibilities.  Any public reference or
advertisement of this source code should refer to it as the Portable
Video Research Group (PVRG) code, and not by any author(s) (or
Stanford University) name.
*************************************************************/

/*
************************************************************
decode.c (original: transform.c)

This file contains the reference DCT, the zig-zag and quantization
algorithms.

************************************************************
*/
/*
 *  Decoder
 *
 *  @(#) $Id: decode.c,v 1.2 2003/07/18 10:19:21 honda Exp $
 */
void ChenIDct (unsigned yidx);

int rgb_buf[4][RGB_NUM][DCTSIZE2];

const int zigzag_index[64] =	/* Is zig-zag map for matrix -> scan array */
{ 0, 1, 5, 6, 14, 15, 27, 28,
	2, 4, 7, 13, 16, 26, 29, 42,
	3, 8, 12, 17, 25, 30, 41, 43,
	9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};


/*
 * IZigzagMatrix() performs an inverse zig-zag translation on the
 * input imatrix and places the output in omatrix.
 *
 * Output is in array QuantBuff
 * Input is from array HuffBuff -> parameter is first dimension
 */
void IZigzagMatrix (unsigned in_idx) {
	int i;
	int out_idx = 0;

	for (i = 0; i < DCTSIZE2; i++) {	
		QuantBuff[out_idx++] = HuffBuff[in_idx][zigzag_index[i]];
	}
}


/*
 * IQuantize() takes an input matrix and does an inverse quantization
 * and puts the output int qmatrix.
 *
 * Input is from array QuantBuff, rewritten in-place.
 * Weights are from array p_jinfo_quant_tbl_quantval -> parameter is first dimension
 */
void IQuantize (unsigned qidx) {
	unsigned i;

	for (i = 0; i < DCTSIZE2; i++) {
		QuantBuff[i] = QuantBuff[i] * p_jinfo_quant_tbl_quantval[qidx][i];
	}
}


/*
 * PostshiftIDctMatrix() adds 128 (2048) to all 64 elements of an 8x8 matrix.
 * This results in strictly positive values for all pixel coefficients.
 *
 * First parameter is index in array IDCTBuff -> first dimension
 */
void PostshiftIDctMatrix (unsigned idx1, int shift) {
	unsigned i;
	for (i = 0; i < DCTSIZE2; i++) {
		IDCTBuff[idx1][i] += shift;
	}
}


/*
 * BoundIDctMatrix bounds the inverse dct matrix so that no pixel has a
 * value greater than 255 (4095) or less than 0.
 *
 * First parameter is first dimension of array IDCTBuff
 */
void BoundIDctMatrix (unsigned idx1, int Bound) {
	unsigned i;
	for (i = 0; i < DCTSIZE2; i++) {
		if (IDCTBuff[idx1][i] < 0) {
			IDCTBuff[idx1][i] = 0;
		}
		else if (IDCTBuff[idx1][i] > Bound) {
			IDCTBuff[idx1][i] = Bound;
		}
	}
}


// First parameter is first dimension in array OutData_comp_buf[][] (output)
//   It is also second dimension in array rgb_buf[][][] (input)
// Second parameter is first dimension in array rgb_buf[][][] (input)
void WriteOneBlock (unsigned outidx, unsigned indim1, int width, int height, int voffs, int hoffs) {
	int i, e;
	unsigned inidx = 0;

	/* Find vertical buffer offs. */
	for (i = voffs; i < voffs + DCTSIZE; i++) {
		if (i < height) {
			int diff;
			diff = width * i;
			for (e = hoffs; e < hoffs + DCTSIZE; e++) {
				if (e < width) {
					OutData_comp_buf[outidx][diff + e] = rgb_buf[indim1][outidx][inidx++];
				}
				else {
				 break;
				}
			}
		}
		else {
			break;
		}
	}
}


/*
 * WriteBlock() writes an array of data in the integer array pointed to
 * by store out to the driver specified by the IOB.  The integer array is
 * stored in row-major form, that is, the first row of (8) elements, the
 * second row of (8) elements....
 * ONLY for MCU 1:1:1
 *
 * First parameter is first dimension in array OutData_comp_buf[][] (output)
 * It is also second dimension in array rgb_buf[][][] (input)
 */
void WriteBlock (int i) {
	int voffs, hoffs;

	/*
	 * Get vertical offsets
	 */
	voffs = OutData_comp_vpos[i] * DCTSIZE;
	hoffs = OutData_comp_hpos[i] * DCTSIZE;

	/*
	 * Write block
	 */
	WriteOneBlock (i, 0, p_jinfo_image_width, p_jinfo_image_height, voffs, hoffs);

	/*
	 *  Add positions
	 */
	OutData_comp_hpos[i]++;
	OutData_comp_vpos[i]++;

	if (OutData_comp_hpos[i] < p_jinfo_MCUWidth) {
		OutData_comp_vpos[i]--;
	}
	else {
		OutData_comp_hpos[i] = 0;		/* If at end of image (width) */
	}
}

/*
 * 4:1:1
 *
 * First parameter is second dimension in array rgb_buf[][][] (input)
 * It is also the first dimension in array OutData_comp_buf[][] (output)
 */
void Write4Blocks (unsigned i) {
	int voffs, hoffs;

	/*
	 * OX
	 * XX
	 */
	voffs = OutData_comp_vpos[i] * DCTSIZE;
	hoffs = OutData_comp_hpos[i] * DCTSIZE;
	WriteOneBlock (i, 0, p_jinfo_image_width, p_jinfo_image_height, voffs, hoffs);

	/*
	 * XO
	 * XX
	 */
	hoffs += DCTSIZE;
	WriteOneBlock (i, 1, p_jinfo_image_width, p_jinfo_image_height, voffs, hoffs);

	/*
	 * XX
	 * OX
	 */
	voffs += DCTSIZE;
	hoffs -= DCTSIZE;
	WriteOneBlock (i, 2, p_jinfo_image_width, p_jinfo_image_height, voffs, hoffs);


	/*
	 * XX
	 * XO
	 */
	hoffs += DCTSIZE;
	WriteOneBlock (i, 3, p_jinfo_image_width, p_jinfo_image_height, voffs, hoffs);

	/*
	 * Add positions
	 */
	OutData_comp_hpos[i] += 2;
	OutData_comp_vpos[i] += 2;


	if (OutData_comp_hpos[i] < p_jinfo_MCUWidth) {
		OutData_comp_vpos[i] -= 2;
	}
	else {
		OutData_comp_hpos[i] = 0;		/* If at end of image (width) */
	}
}


/*
 * Transform from Yuv into RGB
 *
 * First parameter is first dimension in array rgb_buf (output)
 * Parameters 2, 3, 4 are first dimension from array IDCTBuff (input)
 */
void YuvToRgb (int p, unsigned yidx, unsigned uidx, unsigned vidx) {
	int r, g, b;
	int y, u, v;
	int i;

	for (i = 0; i < DCTSIZE2; i++) {
		y = IDCTBuff[yidx][i];
		u = IDCTBuff[uidx][i] - 128;
		v = IDCTBuff[vidx][i] - 128;

		r = (y * 256 + v * 359 + 128) >> 8;
		g = (y * 256 - u * 88 - v * 182 + 128) >> 8;
		b = (y * 256 + u * 454 + 128) >> 8;

		if (r < 0)
			r = 0;
		else if (r > 255)
			r = 255;

		if (g < 0)
			g = 0;
		else if (g > 255)
			g = 255;

		if (b < 0)
			b = 0;
		else if (b > 255)
			b = 255;

		rgb_buf[p][0][i] = r;
		rgb_buf[p][1][i] = g;
		rgb_buf[p][2][i] = b;
	}
}


/*
 * Decode one block
 * Input is for array HuffBuff -> index in first dimension
 * Output is for array IDCTBuff -> index in first dimension
 */
void decode_block (int comp_no, unsigned out_buf_idx, unsigned in_buf_idx) {
	DecodeHuffMCU (in_buf_idx, comp_no);

	IZigzagMatrix (in_buf_idx);

	IQuantize (p_jinfo_comps_info_quant_tbl_no[comp_no] + 1);

	ChenIDct (out_buf_idx);

	PostshiftIDctMatrix (out_buf_idx, IDCT_SHIFT);

	BoundIDctMatrix (out_buf_idx, IDCT_BOUNT);
}


void decode_start () {
	int i;
	int CurrentMCU = 0;

	/* Read buffer */
	CurHuffReadBuf_idx = i_jinfo_jpeg_data;

	/*
	 * Initial value of DC element is 0
	 */
	for (i = 0; i < NUM_COMPONENT; i++) {
		HuffBuff[i][0] = 0;
	}

	/*
	 * Set the size of image to output buffer
	 */
	OutData_image_width = p_jinfo_image_width;
	OutData_image_height = p_jinfo_image_height;

	/*
	 * Initialize output buffer
	 */
	for (i = 0; i < RGB_NUM; i++) {
		OutData_comp_vpos[i] = 0;
		OutData_comp_hpos[i] = 0;
	}

	if (p_jinfo_smp_fact == SF1_1_1) {
		printf ("Decode 1:1:1 NumMCU = %d\n", p_jinfo_NumMCU);

		/*
		 * 1_1_1
		 */
		while (CurrentMCU < p_jinfo_NumMCU) {

			for (i = 0; i < NUM_COMPONENT; i++) {
				decode_block (i, i, i);
			}

			YuvToRgb (0, 0, 1, 2);
			/*
			 * Write
			 */
			for (i = 0; i < RGB_NUM; i++) {
				WriteBlock (i);
			}
			CurrentMCU++;
		}

	}
	else {
		printf ("Decode 4:1:1 NumMCU = %d\n", p_jinfo_NumMCU);
		/*
		 * 4_1_1
		 */
		while (CurrentMCU < p_jinfo_NumMCU) {
			/*
			 * Decode Y element
			 * Decoding Y, U and V elements should be sequentially conducted for the use of Huffman table
			 */

			for (i = 0; i < 4; i++) {
				decode_block (0, i, 0);
			}

			/* Decode U */
			decode_block (1, 4, 1);

			/* Decode V */
			decode_block (2, 5, 2);

			/* Transform from Yuv into RGB */

			for (i = 0; i < 4; i++) {
				YuvToRgb (i, i, 4, 5);
			}

			for (i = 0; i < RGB_NUM; i++) {
				Write4Blocks (i);
			}

			CurrentMCU += 4;
		}
	}
}

