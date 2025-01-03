
#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)

	#include <augh.h>

	uint8_t stdin;
	uint8_t stdout;

	static inline uint8_t read8() {
		uint8_t u8;
		augh_read(stdin, &u8);
		return u8;
	}
	static inline void write8(uint8_t u8) {
		augh_write(stdout, &u8);
	}

#endif

#if defined(AUGH_SYNTHESIS) && defined(STDIO8)

	#include "../stdio8_defs.h"

#endif

#ifdef GCC_COMPIL  // Standard compilation with gcc
	#include <stdio.h>
	#include <stdint.h>

	static inline uint8_t read8() {
		return getchar();
	}
	static inline void write8(uint8_t u8) {
		putchar(u8);
	}

#endif



int type;
int nb;
int round;
int key[32];
int statemt[32];
int word[4][120];

const int Sbox[16][16] = {
  {0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76},
  {0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0},
  {0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15},
  {0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75},
  {0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84},
  {0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf},
  {0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8},
  {0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2},
  {0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73},
  {0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb},
  {0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79},
  {0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08},
  {0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a},
  {0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e},
  {0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf},
  {0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16}
};
const int invSbox[16][16] = {
  {0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb},
  {0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb},
  {0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e},
  {0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25},
  {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92},
  {0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84},
  {0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06},
  {0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b},
  {0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73},
  {0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e},
  {0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b},
  {0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4},
  {0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f},
  {0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef},
  {0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61},
  {0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d}
};
const int Rcon0[30] = {
  0x01, 0x02, 0x04, 0x08,
  0x10, 0x20, 0x40, 0x80,
  0x1b, 0x36, 0x6c, 0xd8,
  0xab, 0x4d, 0x9a, 0x2f,
  0x5e, 0xbc, 0x63, 0xc6,
  0x97, 0x35, 0x6a, 0xd4,
  0xb3, 0x7d, 0xfa, 0xef,
  0xc5, 0x91,
};



static inline int SubByte (int in) {
  return Sbox[(in / 16)][(in % 16)];
}

static inline int KeySchedule (int type) {
  int nk, nb, round;
  int i, j, temp[4];
	unsigned j_remain_nk;  // Contains j % nk
	unsigned j_quot_nk;  // Contains j / nk

  switch (type) {
    case 128128:
      nk = 4;
      nb = 4;
      round = 10;
      break;
    case 128192:
      nk = 4;
      nb = 6;
      round = 12;
      break;
    case 128256:
      nk = 4;
      nb = 8;
      round = 14;
      break;
    case 192128:
      nk = 6;
      nb = 4;
      round = 12;
      break;
    case 192192:
      nk = 6;
      nb = 6;
      round = 12;
      break;
    case 192256:
      nk = 6;
      nb = 8;
      round = 14;
      break;
    case 256128:
      nk = 8;
      nb = 4;
      round = 14;
      break;
    case 256192:
      nk = 8;
      nb = 6;
      round = 14;
      break;
    case 256256:
      nk = 8;
      nb = 8;
      round = 14;
      break;
    default: break;
//      return -1;
  }

	/* 0 word */
  for (j = 0; j < nk; ++j)
    for (i = 0; i < 4; ++i)
      word[i][j] = key[i + j * 4];

	/* expanded key is generated */
  j_remain_nk = 0;
  j_quot_nk = 1;
  for (j = nk; j < nb * (round + 1); ++j) {

		/* RotByte */
		if (j_remain_nk == 0) {
			temp[0] = SubByte (word[1][j - 1]) ^ Rcon0[j_quot_nk - 1];
			temp[1] = SubByte (word[2][j - 1]);
			temp[2] = SubByte (word[3][j - 1]);
			temp[3] = SubByte (word[0][j - 1]);
		}
		if (j_remain_nk != 0) {
			temp[0] = word[0][j - 1];
			temp[1] = word[1][j - 1];
			temp[2] = word[2][j - 1];
			temp[3] = word[3][j - 1];
		}
		if (nk > 6 && j_remain_nk == 4) {
			for (i = 0; i < 4; ++i) temp[i] = SubByte (temp[i]);
		}
		for (i = 0; i < 4; ++i) word[i][j] = word[i][j - nk] ^ temp[i];

		// Compute j % nk
		j_remain_nk++;
		if(j_remain_nk>=nk) {
			j_remain_nk = 0;
			j_quot_nk ++;
		}

  }
  return 0;
}



static inline void ByteSub_ShiftRow (int nb) {
  int temp;

  switch (nb) {
    case 4:
      temp = Sbox[statemt[1] >> 4][statemt[1] & 0xf];
      statemt[1] = Sbox[statemt[5] >> 4][statemt[5] & 0xf];
      statemt[5] = Sbox[statemt[9] >> 4][statemt[9] & 0xf];
      statemt[9] = Sbox[statemt[13] >> 4][statemt[13] & 0xf];
      statemt[13] = temp;

      temp = Sbox[statemt[2] >> 4][statemt[2] & 0xf];
      statemt[2] = Sbox[statemt[10] >> 4][statemt[10] & 0xf];
      statemt[10] = temp;
      temp = Sbox[statemt[6] >> 4][statemt[6] & 0xf];
      statemt[6] = Sbox[statemt[14] >> 4][statemt[14] & 0xf];
      statemt[14] = temp;

      temp = Sbox[statemt[3] >> 4][statemt[3] & 0xf];
      statemt[3] = Sbox[statemt[15] >> 4][statemt[15] & 0xf];
      statemt[15] = Sbox[statemt[11] >> 4][statemt[11] & 0xf];
      statemt[11] = Sbox[statemt[7] >> 4][statemt[7] & 0xf];
      statemt[7] = temp;

      statemt[0] = Sbox[statemt[0] >> 4][statemt[0] & 0xf];
      statemt[4] = Sbox[statemt[4] >> 4][statemt[4] & 0xf];
      statemt[8] = Sbox[statemt[8] >> 4][statemt[8] & 0xf];
      statemt[12] = Sbox[statemt[12] >> 4][statemt[12] & 0xf];
      break;
    case 6:
      temp = Sbox[statemt[1] >> 4][statemt[1] & 0xf];
      statemt[1] = Sbox[statemt[5] >> 4][statemt[5] & 0xf];
      statemt[5] = Sbox[statemt[9] >> 4][statemt[9] & 0xf];
      statemt[9] = Sbox[statemt[13] >> 4][statemt[13] & 0xf];
      statemt[13] = Sbox[statemt[17] >> 4][statemt[17] & 0xf];
      statemt[17] = Sbox[statemt[21] >> 4][statemt[21] & 0xf];
      statemt[21] = temp;

      temp = Sbox[statemt[2] >> 4][statemt[2] & 0xf];
      statemt[2] = Sbox[statemt[10] >> 4][statemt[10] & 0xf];
      statemt[10] = Sbox[statemt[18] >> 4][statemt[18] & 0xf];
      statemt[18] = temp;
      temp = Sbox[statemt[6] >> 4][statemt[6] & 0xf];
      statemt[6] = Sbox[statemt[14] >> 4][statemt[14] & 0xf];
      statemt[14] = Sbox[statemt[22] >> 4][statemt[22] & 0xf];
      statemt[22] = temp;

      temp = Sbox[statemt[3] >> 4][statemt[3] & 0xf];
      statemt[3] = Sbox[statemt[15] >> 4][statemt[15] & 0xf];
      statemt[15] = temp;
      temp = Sbox[statemt[7] >> 4][statemt[7] & 0xf];
      statemt[7] = Sbox[statemt[19] >> 4][statemt[19] & 0xf];
      statemt[19] = temp;
      temp = Sbox[statemt[11] >> 4][statemt[11] & 0xf];
      statemt[11] = Sbox[statemt[23] >> 4][statemt[23] & 0xf];
      statemt[23] = temp;

      statemt[0] = Sbox[statemt[0] >> 4][statemt[0] & 0xf];
      statemt[4] = Sbox[statemt[4] >> 4][statemt[4] & 0xf];
      statemt[8] = Sbox[statemt[8] >> 4][statemt[8] & 0xf];
      statemt[12] = Sbox[statemt[12] >> 4][statemt[12] & 0xf];
      statemt[16] = Sbox[statemt[16] >> 4][statemt[16] & 0xf];
      statemt[20] = Sbox[statemt[20] >> 4][statemt[20] & 0xf];
      break;
    case 8:
      temp = Sbox[statemt[1] >> 4][statemt[1] & 0xf];
      statemt[1] = Sbox[statemt[5] >> 4][statemt[5] & 0xf];
      statemt[5] = Sbox[statemt[9] >> 4][statemt[9] & 0xf];
      statemt[9] = Sbox[statemt[13] >> 4][statemt[13] & 0xf];
      statemt[13] = Sbox[statemt[17] >> 4][statemt[17] & 0xf];
      statemt[17] = Sbox[statemt[21] >> 4][statemt[21] & 0xf];
      statemt[21] = Sbox[statemt[25] >> 4][statemt[25] & 0xf];
      statemt[25] = Sbox[statemt[29] >> 4][statemt[29] & 0xf];
      statemt[29] = temp;

      temp = Sbox[statemt[2] >> 4][statemt[2] & 0xf];
      statemt[2] = Sbox[statemt[14] >> 4][statemt[14] & 0xf];
      statemt[14] = Sbox[statemt[26] >> 4][statemt[26] & 0xf];
      statemt[26] = Sbox[statemt[6] >> 4][statemt[6] & 0xf];
      statemt[6] = Sbox[statemt[18] >> 4][statemt[18] & 0xf];
      statemt[18] = Sbox[statemt[30] >> 4][statemt[30] & 0xf];
      statemt[30] = Sbox[statemt[10] >> 4][statemt[10] & 0xf];
      statemt[10] = Sbox[statemt[22] >> 4][statemt[22] & 0xf];
      statemt[22] = temp;

      temp = Sbox[statemt[3] >> 4][statemt[3] & 0xf];
      statemt[3] = Sbox[statemt[19] >> 4][statemt[19] & 0xf];
      statemt[19] = temp;
      temp = Sbox[statemt[7] >> 4][statemt[7] & 0xf];
      statemt[7] = Sbox[statemt[23] >> 4][statemt[23] & 0xf];
      statemt[23] = temp;
      temp = Sbox[statemt[11] >> 4][statemt[11] & 0xf];
      statemt[11] = Sbox[statemt[27] >> 4][statemt[27] & 0xf];
      statemt[27] = temp;
      temp = Sbox[statemt[15] >> 4][statemt[15] & 0xf];
      statemt[15] = Sbox[statemt[31] >> 4][statemt[31] & 0xf];
      statemt[31] = temp;

      statemt[0] = Sbox[statemt[0] >> 4][statemt[0] & 0xf];
      statemt[4] = Sbox[statemt[4] >> 4][statemt[4] & 0xf];
      statemt[8] = Sbox[statemt[8] >> 4][statemt[8] & 0xf];
      statemt[12] = Sbox[statemt[12] >> 4][statemt[12] & 0xf];
      statemt[16] = Sbox[statemt[16] >> 4][statemt[16] & 0xf];
      statemt[20] = Sbox[statemt[20] >> 4][statemt[20] & 0xf];
      statemt[24] = Sbox[statemt[24] >> 4][statemt[24] & 0xf];
      statemt[28] = Sbox[statemt[28] >> 4][statemt[28] & 0xf];
      break;
   }

}

static inline void InversShiftRow_ByteSub (int nb) {
  int temp;

  switch (nb) {
    case 4:
      temp = invSbox[statemt[13] >> 4][statemt[13] & 0xf];
      statemt[13] = invSbox[statemt[9] >> 4][statemt[9] & 0xf];
      statemt[9] = invSbox[statemt[5] >> 4][statemt[5] & 0xf];
      statemt[5] = invSbox[statemt[1] >> 4][statemt[1] & 0xf];
      statemt[1] = temp;

      temp = invSbox[statemt[14] >> 4][statemt[14] & 0xf];
      statemt[14] = invSbox[statemt[6] >> 4][statemt[6] & 0xf];
      statemt[6] = temp;
      temp = invSbox[statemt[2] >> 4][statemt[2] & 0xf];
      statemt[2] = invSbox[statemt[10] >> 4][statemt[10] & 0xf];
      statemt[10] = temp;

      temp = invSbox[statemt[15] >> 4][statemt[15] & 0xf];
      statemt[15] = invSbox[statemt[3] >> 4][statemt[3] & 0xf];
      statemt[3] = invSbox[statemt[7] >> 4][statemt[7] & 0xf];
      statemt[7] = invSbox[statemt[11] >> 4][statemt[11] & 0xf];
      statemt[11] = temp;

      statemt[0] = invSbox[statemt[0] >> 4][statemt[0] & 0xf];
      statemt[4] = invSbox[statemt[4] >> 4][statemt[4] & 0xf];
      statemt[8] = invSbox[statemt[8] >> 4][statemt[8] & 0xf];
      statemt[12] = invSbox[statemt[12] >> 4][statemt[12] & 0xf];
      break;
    case 6:
      temp = invSbox[statemt[21] >> 4][statemt[21] & 0xf];
      statemt[21] = invSbox[statemt[17] >> 4][statemt[17] & 0xf];
      statemt[17] = invSbox[statemt[13] >> 4][statemt[13] & 0xf];
      statemt[13] = invSbox[statemt[9] >> 4][statemt[9] & 0xf];
      statemt[9] = invSbox[statemt[5] >> 4][statemt[5] & 0xf];
      statemt[5] = invSbox[statemt[1] >> 4][statemt[1] & 0xf];
      statemt[1] = temp;

      temp = invSbox[statemt[22] >> 4][statemt[22] & 0xf];
      statemt[22] = invSbox[statemt[14] >> 4][statemt[14] & 0xf];
      statemt[14] = invSbox[statemt[6] >> 4][statemt[6] & 0xf];
      statemt[6] = temp;
      temp = invSbox[statemt[18] >> 4][statemt[18] & 0xf];
      statemt[18] = invSbox[statemt[10] >> 4][statemt[10] & 0xf];
      statemt[10] = invSbox[statemt[2] >> 4][statemt[2] & 0xf];
      statemt[2] = temp;

      temp = invSbox[statemt[15] >> 4][statemt[15] & 0xf];
      statemt[15] = invSbox[statemt[3] >> 4][statemt[3] & 0xf];
      statemt[3] = temp;
      temp = invSbox[statemt[19] >> 4][statemt[19] & 0xf];
      statemt[19] = invSbox[statemt[7] >> 4][statemt[7] & 0xf];
      statemt[7] = temp;
      temp = invSbox[statemt[23] >> 4][statemt[23] & 0xf];
      statemt[23] = invSbox[statemt[11] >> 4][statemt[11] & 0xf];
      statemt[11] = temp;

      statemt[0] = invSbox[statemt[0] >> 4][statemt[0] & 0xf];
      statemt[4] = invSbox[statemt[4] >> 4][statemt[4] & 0xf];
      statemt[8] = invSbox[statemt[8] >> 4][statemt[8] & 0xf];
      statemt[12] = invSbox[statemt[12] >> 4][statemt[12] & 0xf];
      statemt[16] = invSbox[statemt[16] >> 4][statemt[16] & 0xf];
      statemt[20] = invSbox[statemt[20] >> 4][statemt[20] & 0xf];
      break;
    case 8:
      temp = invSbox[statemt[29] >> 4][statemt[29] & 0xf];
      statemt[29] = invSbox[statemt[25] >> 4][statemt[25] & 0xf];
      statemt[25] = invSbox[statemt[21] >> 4][statemt[21] & 0xf];
      statemt[21] = invSbox[statemt[17] >> 4][statemt[17] & 0xf];
      statemt[17] = invSbox[statemt[13] >> 4][statemt[13] & 0xf];
      statemt[13] = invSbox[statemt[9] >> 4][statemt[9] & 0xf];
      statemt[9] = invSbox[statemt[5] >> 4][statemt[5] & 0xf];
      statemt[5] = invSbox[statemt[1] >> 4][statemt[1] & 0xf];
      statemt[1] = temp;

      temp = invSbox[statemt[30] >> 4][statemt[30] & 0xf];
      statemt[30] = invSbox[statemt[18] >> 4][statemt[18] & 0xf];
      statemt[18] = invSbox[statemt[6] >> 4][statemt[6] & 0xf];
      statemt[6] = invSbox[statemt[26] >> 4][statemt[26] & 0xf];
      statemt[26] = invSbox[statemt[14] >> 4][statemt[14] & 0xf];
      statemt[14] = invSbox[statemt[2] >> 4][statemt[2] & 0xf];
      statemt[2] = invSbox[statemt[22] >> 4][statemt[22] & 0xf];
      statemt[22] = invSbox[statemt[10] >> 4][statemt[10] & 0xf];
      statemt[10] = temp;

      temp = invSbox[statemt[31] >> 4][statemt[31] & 0xf];
      statemt[31] = invSbox[statemt[15] >> 4][statemt[15] & 0xf];
      statemt[15] = temp;
      temp = invSbox[statemt[27] >> 4][statemt[27] & 0xf];
      statemt[27] = invSbox[statemt[11] >> 4][statemt[11] & 0xf];
      statemt[11] = temp;
      temp = invSbox[statemt[23] >> 4][statemt[23] & 0xf];
      statemt[23] = invSbox[statemt[7] >> 4][statemt[7] & 0xf];
      statemt[7] = temp;
      temp = invSbox[statemt[19] >> 4][statemt[19] & 0xf];
      statemt[19] = invSbox[statemt[3] >> 4][statemt[3] & 0xf];
      statemt[3] = temp;

      statemt[0] = invSbox[statemt[0] >> 4][statemt[0] & 0xf];
      statemt[4] = invSbox[statemt[4] >> 4][statemt[4] & 0xf];
      statemt[8] = invSbox[statemt[8] >> 4][statemt[8] & 0xf];
      statemt[12] = invSbox[statemt[12] >> 4][statemt[12] & 0xf];
      statemt[16] = invSbox[statemt[16] >> 4][statemt[16] & 0xf];
      statemt[20] = invSbox[statemt[20] >> 4][statemt[20] & 0xf];
      statemt[24] = invSbox[statemt[24] >> 4][statemt[24] & 0xf];
      statemt[28] = invSbox[statemt[28] >> 4][statemt[28] & 0xf];
      break;
  }

}

static inline int MixColumn_AddRoundKey (int nb, int n) {
  int ret[8 * 4], j;
  register int x;

  for (j = 0; j < nb; ++j) {

		ret[j * 4] = (statemt[j * 4] << 1);
		if ((ret[j * 4] >> 8) == 1) ret[j * 4] ^= 283;
		x = statemt[1 + j * 4];
		x ^= (x << 1);
		if ((x >> 8) == 1) ret[j * 4] ^= (x ^ 283);
		else ret[j * 4] ^= x;
		ret[j * 4] ^=
		statemt[2 + j * 4] ^ statemt[3 + j * 4] ^ word[0][j + nb * n];

		ret[1 + j * 4] = (statemt[1 + j * 4] << 1);
		if ((ret[1 + j * 4] >> 8) == 1) ret[1 + j * 4] ^= 283;
		x = statemt[2 + j * 4];
		x ^= (x << 1);
		if ((x >> 8) == 1) ret[1 + j * 4] ^= (x ^ 283);
		else ret[1 + j * 4] ^= x;
		ret[1 + j * 4] ^=
		statemt[3 + j * 4] ^ statemt[j * 4] ^ word[1][j + nb * n];

		ret[2 + j * 4] = (statemt[2 + j * 4] << 1);
		if ((ret[2 + j * 4] >> 8) == 1) ret[2 + j * 4] ^= 283;
		x = statemt[3 + j * 4];
		x ^= (x << 1);
		if ((x >> 8) == 1) ret[2 + j * 4] ^= (x ^ 283);
		else ret[2 + j * 4] ^= x;
		ret[2 + j * 4] ^=
		statemt[j * 4] ^ statemt[1 + j * 4] ^ word[2][j + nb * n];

		ret[3 + j * 4] = (statemt[3 + j * 4] << 1);
		if ((ret[3 + j * 4] >> 8) == 1) ret[3 + j * 4] ^= 283;
		x = statemt[j * 4];
		x ^= (x << 1);
		if ((x >> 8) == 1) ret[3 + j * 4] ^= (x ^ 283);
		else ret[3 + j * 4] ^= x;
		ret[3 + j * 4] ^=
		statemt[1 + j * 4] ^ statemt[2 + j * 4] ^ word[3][j + nb * n];
  }
  for (j = 0; j < nb; ++j) {
		statemt[j * 4] = ret[j * 4];
		statemt[1 + j * 4] = ret[1 + j * 4];
		statemt[2 + j * 4] = ret[2 + j * 4];
		statemt[3 + j * 4] = ret[3 + j * 4];
	}

  return 0;
}

static inline int AddRoundKey_InversMixColumn (int nb, int n) {
  int ret[8 * 4], i, j;
  register int x;

  for (j = 0; j < nb; ++j) {
		statemt[j * 4] ^= word[0][j + nb * n];
		statemt[1 + j * 4] ^= word[1][j + nb * n];
		statemt[2 + j * 4] ^= word[2][j + nb * n];
		statemt[3 + j * 4] ^= word[3][j + nb * n];
	}
  for (j = 0; j < nb; ++j) {
    for (i = 0; i < 4; ++i) {

			x = (statemt[i + j * 4] << 1);
			if ((x >> 8) == 1) x ^= 283;
			x ^= statemt[i + j * 4];
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			x ^= statemt[i + j * 4];
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			ret[i + j * 4] = x;

			x = (statemt[(i + 1) % 4 + j * 4] << 1);
			if ((x >> 8) == 1) x ^= 283;
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			x ^= statemt[(i + 1) % 4 + j * 4];
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			x ^= statemt[(i + 1) % 4 + j * 4];
			ret[i + j * 4] ^= x;

			x = (statemt[(i + 2) % 4 + j * 4] << 1);
			if ((x >> 8) == 1) x ^= 283;
			x ^= statemt[(i + 2) % 4 + j * 4];
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			x ^= statemt[(i + 2) % 4 + j * 4];
			ret[i + j * 4] ^= x;

			x = (statemt[(i + 3) % 4 + j * 4] << 1);
			if ((x >> 8) == 1) x ^= 283;
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			x = (x << 1);
			if ((x >> 8) == 1) x ^= 283;
			x ^= statemt[(i + 3) % 4 + j * 4];
			ret[i + j * 4] ^= x;
    }
	}
  for (i = 0; i < nb; ++i) {
		statemt[i * 4] = ret[i * 4];
		statemt[1 + i * 4] = ret[1 + i * 4];
		statemt[2 + i * 4] = ret[2 + i * 4];
		statemt[3 + i * 4] = ret[3 + i * 4];
	}

  return 0;
}

static inline int AddRoundKey (int type, int n) {
  int j, nb;

  switch (type) {
    case 128128:
    case 192128:
    case 256128:
      nb = 4;
      break;
    case 128192:
    case 192192:
    case 256192:
      nb = 6;
      break;
    case 128256:
    case 192256:
    case 256256:
      nb = 8;
      break;
	}
  for (j = 0; j < nb; ++j) {
		statemt[j * 4] ^= word[0][j + nb * n];
		statemt[1 + j * 4] ^= word[1][j + nb * n];
		statemt[2 + j * 4] ^= word[2][j + nb * n];
		statemt[3 + j * 4] ^= word[3][j + nb * n];
	}

  return 0;
}



static inline int encrypt (int type) {
  int i;

  KeySchedule (type);

  switch (type) {
    case 128128:
      round = 0;
      nb = 4;
      break;
    case 192128:
      round = 2;
      nb = 4;
      break;
    case 256128:
      round = 4;
      nb = 4;
      break;
    case 128192:
    case 192192:
      round = 2;
      nb = 6;
      break;
    case 256192:
      round = 4;
      nb = 6;
      break;
    case 128256:
    case 192256:
    case 256256:
      round = 4;
      nb = 8;
      break;
   }

  AddRoundKey (type, 0);

  for (i = 1; i <= round + 9; ++i) {
		ByteSub_ShiftRow (nb);
		MixColumn_AddRoundKey (nb, i);
	}

  ByteSub_ShiftRow (nb);
  AddRoundKey (type, i);

  return 0;
}

static inline int decrypt (int type) {
  int i;

  KeySchedule (type);

  switch (type) {
    case 128128:
      round = 10;
      nb = 4;
      break;
    case 128192:
    case 192192:
      round = 12;
      nb = 6;
      break;
    case 192128:
      round = 12;
      nb = 4;
      break;
    case 128256:
    case 192256:
      round = 14;
      nb = 8;
      break;
    case 256128:
      round = 14;
      nb = 4;
      break;
    case 256192:
      round = 14;
      nb = 6;
      break;
    case 256256:
      round = 14;
      nb = 8;
      break;
   }

  AddRoundKey (type, round);

  InversShiftRow_ByteSub (nb);

  for (i = round - 1; i >= 1; --i) {
		AddRoundKey_InversMixColumn (nb, i);
		InversShiftRow_ByteSub (nb);
	}

  AddRoundKey (type, 0);

  return 0;
}



#ifdef AUGH_SYNTHESIS
void augh_main ()
#endif
#ifdef GCC_COMPIL
int main ()
#endif
{
	int i;
	int style;

	#ifdef ASKSEND
	read8();
	#endif

	// Initialization needed to avoid symbols read but never written
	for(i=0; i<32; i++) statemt[i] = 0;
	for(i=0; i<32; i++) key[i] = 0;

	do {

		for(i=0; i<16; i++) statemt[i] = read8();
		for(i=0; i<16; i++) key[i] = read8();

		style = 128128;

		encrypt (style);
		for(i=0; i<16; i++) write8(statemt[i]);

		decrypt (style);
		for(i=0; i<16; i++) write8(statemt[i]);

		#ifdef GCC_COMPIL
		break;
		#endif

	} while(1);

	#ifdef GCC_COMPIL
	return 0;
	#endif
}

