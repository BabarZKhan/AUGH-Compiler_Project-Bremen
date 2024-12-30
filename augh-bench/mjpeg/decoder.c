
// Definitions to force an environment.
//#define AUGH_SYNTHESIS
//#define GCC_COMPIL
//#define APES_COMPIL

#include <augh_annot.h>

#ifdef AUGH_SYNTHESIS

	#include <augh.h>

	#define VPRINTF(...)
	#define IPRINTF(...)

	uint8_t stdin;
	uint8_t stdout;

	#ifdef ASKSEND
		#define ASKSEND_MASK_DO  0x01  // 0 means input data is asked
		#define ASKSEND_MASK_NB  0x02  // 0 means only one. Otherwise, the next byte indicates the nb.
		static inline uint8_t read8() {
			uint8_t u8 = 0;
			augh_write(stdout, &u8);  // Ask for a byte of data
			augh_read(stdin, &u8);    // Receive the data
			return u8;
		}
		static inline void write8(uint8_t u8) {
			uint8_t u8ask = ASKSEND_MASK_DO;
			augh_write(stdout, &u8ask);  // Indicate it's data being sent
			augh_write(stdout, &u8);     // Send the data
		}
	#else
		static inline uint8_t read8() { uint8_t u8; augh_read(stdin, &u8); return u8; }
		static inline void write8(uint8_t u8) { augh_write(stdout, &u8); }
	#endif

#endif

#ifdef GCC_COMPIL  // Standard compilation with gcc

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdint.h>
	#include <stdbool.h>

	//#define VERBOSE
	#ifdef VERBOSE
		#define VPRINTF(format, ...) printf ("[%.40s] " format, __FUNCTION__, ## __VA_ARGS__)
	#else
		#define VPRINTF(format, ...)
	#endif

	//#define INFO
	#ifdef INFO
		#define IPRINTF(format, ...)  printf ("[%.40s] " format, __FUNCTION__, ## __VA_ARGS__)
	#else
		#define IPRINTF(format, ...)
	#endif

	static inline uint8_t read8() {
		int z = getchar();
		if(z==EOF) exit(EXIT_SUCCESS);
		return z;
	}
	static inline void write8(uint8_t u8) { putchar(u8); }

#endif

#ifdef APES_COMPIL  // For APES

	#include <stdio.h>
	#include <stdint.h>
	#include <stdbool.h>

	#include <KahnProcessNetwork/KahnProcessNetwork.h>

	#ifdef VERBOSE
	#define VPRINTF(format, ...)  \
		printf ("[%.40s] " format, __FUNCTION__, ## __VA_ARGS__)
	#else
		#define VPRINTF(format, ...)
	#endif

	#ifdef INFO
	#define IPRINTF(format, ...)  \
		printf ("[%.40s] " format, __FUNCTION__, ## __VA_ARGS__)
	#else
		#define IPRINTF(format, ...)
	#endif

	kpn_channel_t decoder_channel[2];

	static inline uint8_t read8() { uint8_t u8; kpn_channel_read(decoder_channel[0], &u8, 1); return u8; }
	static inline void write8(uint8_t u8) { kpn_channel_write(decoder_channel[1], &u8, 1); }

	// The name of the main function
	#define augh_main decoder

	uint8_t picture[300*200*2];  // 2 bytes per pixel
	static inline void write_picture_byte(uint32_t address, uint8_t val) {
		picture[address] = val;
	}

	extern uint16_t SOF_section_width;          /* Image width in pixels */
	extern uint16_t SOF_section_height;         /* Image height in pixels */

	static inline void write_picture_full() {
		kpn_channel_write(decoder_channel[1], picture, SOF_section_width * SOF_section_height * 2);
	}

#endif



//======================================================
// MJpeg header
//======================================================

#ifndef MJPEG_H
#define MJPEG_H


#define MCU_sx 8
#define MCU_sy 8

#define HUFF_DC			0
#define HUFF_AC			1
#define HUFF_EOB    0x00
#define HUFF_ZRL    0xF0

/* Memory size of * HTables */
#define MAX_SIZE_AC 162
#define MAX_SIZE_DC 14

#define M_SOF0	0xc0		/* Baseline DCT */
#define M_SOF1	0xc1		/* Extended sequential DCT */
#define M_SOF2	0xc2		/* Progressive DCT */
#define M_SOF3	0xc3		/* Lossless (sequential) */
#define M_DHT		0xc4		/* Define Huffman tables */
#define M_SOF5	0xc5		/* Differential sequential DCT */
#define M_SOF6	0xc6		/* Differential progressive DCT */
#define M_SOF7	0xc7		/* Differential lossless */
#define M_JPG		0xc8		/* JPEG extensions */
#define M_SOF9	0xc9		/* Extended sequential DCT */
#define M_SOF10	0xca		/* Progressive DCT */
#define M_SOF11	0xcb		/* Lossless (sequential) */
#define M_DAC		0xcc		/* Define arithmetic conditioning table */
#define M_SOF13	0xcd		/* Differential sequential DCT */
#define M_SOF14	0xce		/* Differential progressive DCT */
#define M_SOF15	0xcf		/* Differential lossless */
#define M_RST0	0xd0		/* Restart */
#define M_RST1	0xd1		/* Restart */
#define M_RST2	0xd2		/* Restart */
#define M_RST3	0xd3		/* Restart */
#define M_RST4	0xd4		/* Restart */
#define M_RST5	0xd5		/* Restart */
#define M_RST6	0xd6		/* Restart */
#define M_RST7	0xd7		/* Restart */
#define M_SOI		0xd8		/* Start of image */
#define M_EOI		0xd9		/* End Of Image */
#define M_SOS		0xda		/* Start Of Scan */
#define M_DQT		0xdb		/* Define quantization tables */
#define M_DNL		0xdc		/* Define number of lines */
#define M_DRI		0xdd		/* Define restart interval */
#define M_DHP		0xde		/* Define hierarchical progression */
#define M_EXP		0xdf		/* Expand reference image(s) */
#define M_APP0	0xe0		/* Application marker, used for JFIF */
#define M_APP1	0xe1		/* Application marker */
#define M_APP2	0xe2		/* Application marker */
#define M_APP3	0xe3		/* Application marker */
#define M_APP4	0xe4		/* Application marker */
#define M_APP5	0xe5		/* Application marker */
#define M_APP6	0xe6		/* Application marker */
#define M_APP7	0xe7		/* Application marker */
#define M_APP8	0xe8		/* Application marker */
#define M_APP9	0xe9		/* Application marker */
#define M_APP10	0xea		/* Application marker */
#define M_APP11	0xeb		/* Application marker */
#define M_APP12	0xec		/* Application marker */
#define M_APP13	0xed		/* Application marker */
#define M_APP14	0xee		/* Application marker, used by Adobe */
#define M_APP15	0xef		/* Application marker */
#define M_JPG0	0xf0		/* Reserved for JPEG extensions */
#define M_JPG13	0xfd		/* Reserved for JPEG extensions */
#define M_COM		0xfe		/* Comment */
#define M_TEM		0x01		/* Temporary use */
#define M_ERROR	0x100		/* Dummy marker, internal use only */
#define M_SMS		0xff		/* Start Marker Segment */

#define M_MARKER(m)		(0xff << 8 | m)

#define GOOD_NODE_FLAG		0x100
#define GOOD_LEAF_FLAG		0x200
#define BAD_LEAF_FLAG		0x300
#define SPECIAL_FLAG		0x000
#define HUFF_FLAG_MSK		0x300

#define HUFF_FLAG(c)		((c) & HUFF_FLAG_MSK)
#define HUFF_VALUE(c)		((unsigned char)( (c) & (~HUFF_FLAG_MSK) ))

typedef unsigned short jpeg_marker_t;

typedef struct
{
  int32_t ValPtr[16];
  int32_t MinCode[16];
  int32_t MaxCode[16];
  uint8_t * table;
}
huff_table_t;

typedef struct
{
  uint8_t bit_count;
  uint8_t window;
  int32_t pred[3];
  huff_table_t * table[2][3];
}
scan_desc_t;

typedef struct {
  uint16_t length;		/* Length of APP0 Field */
  char identifier[5];	/* "JFIF" (zero terminated) Id * String */
  uint8_t version[2];	/* JFIF Format Revision */
  uint8_t units;			/* Units used for Resolution */
  uint16_t xdensity;	/* Horizontal Resolution */
  uint16_t ydensity;	/* Vertical Resolution */
  uint8_t xthumbnail;	/* Horizontal Pixel Count */
  uint8_t ythumbnail;	/* Vertical Pixel Count */
} __attribute__ ((__packed__))
jfif_header_t;

typedef struct
{
  uint16_t length;			/* Length of DQT section */
  uint8_t  pre_quant;	/* 4bits high : precision (0=8bits, * 1=16bits) */
} __attribute__ ((__packed__))
DQT_section_t;

typedef struct {
  uint8_t index;		/* Component index */
  uint8_t HV;				/* H(4bits high) and V(4bits low) sampling * factors */
  uint8_t q_table;	/* Quantization table associated */
} __attribute__ ((__packed__))
SOF_component_t;

typedef struct {
  uint16_t length;				/* Length of SOF section */
  uint8_t data_precision;	/* Data bit precision (usually 8, * for baseline JPEG) */
  uint16_t height;				/* Image height in pixels */
  uint16_t width;					/* Image width in pixels */
  uint8_t n;							/* n = Number of components (eg. 3 for * RGB, 1 for Grayscale) */
} __attribute__ ((__packed__))
SOF_section_t;

typedef struct
{
  uint16_t length;		/* Length in byte of this section, * including this data. */
  uint8_t huff_info;	/* Huffman Table information : */
  /* bit 0..3 : index (0..3, otherwise error) */
  /* bit 4 : type (0=DC, 1=AC) */
  /* bit 5..7 : not used, must be 0 */
} __attribute__ ((__packed__))
DHT_section_t;

typedef struct {
  uint8_t index;
  uint8_t acdc;
} __attribute__ ((__packed__))
SOS_component_t;

typedef struct {
  uint16_t length;	/* Length in byte of this section, * including this data. */
  uint8_t   n;		/* N = Number of components */
} __attribute__ ((__packed__))
SOS_section_t;

#endif



//======================================================
// Miscellaneous FIFO in/out
//======================================================


#if defined(AUGH_SYNTHESIS) && defined(ASKSEND)

	static inline void write32_lr(uint32_t u32) {
		uint8_t i, u8;
		// Send protocol stuff
		u8 = ASKSEND_MASK_DO | ASKSEND_MASK_NB;
		augh_write(stdout, &u8);
		u8 = 4;
		augh_write(stdout, &u8);
		// Send the data
		for(i=0; i<4; i++) { u8 = (u32 >> 24) & 0xFF; u32 = u32 << 8; augh_write(stdout, &u8); }
	}
	static inline void write32_rl(uint32_t u32) {
		uint8_t i, u8;
		// Send protocol stuff
		u8 = ASKSEND_MASK_DO | ASKSEND_MASK_NB;
		augh_write(stdout, &u8);
		u8 = 4;
		augh_write(stdout, &u8);
		// Send the data
		for(i=0; i<4; i++) { u8 = u32 & 0xFF; u32 = u32 >> 8; augh_write(stdout, &u8); }
	}

	static inline uint16_t read16_be() {
		uint8_t u8;
		uint16_t u16;
		// Send protocol stuff
		u8 = ASKSEND_MASK_NB;
		augh_write(stdout, &u8);
		u8 = 2;
		augh_write(stdout, &u8);
		// Receive the data
		augh_read(stdin, &u8);
		u16 = u8;
		augh_read(stdin, &u8);
		u16 = (u16<<8) | u8;
		return u16;
	}

	static inline void write16_be(uint16_t u16) {
		uint8_t u8;
		// Send protocol stuff
		u8 = ASKSEND_MASK_DO | ASKSEND_MASK_NB;
		augh_write(stdout, &u8);
		u8 = 2;
		augh_write(stdout, &u8);
		// Send the data
		u8 = u16 >> 8;
		augh_write(stdout, &u8);
		u8 = u16;
		augh_write(stdout, &u8);
	}

	unsigned augh_vread_max255_i;
	unsigned augh_vread_max255_end;
	uint8_t  augh_vread_max255_u8;
	#define augh_vread_max255(data,beg,nb) do { \
		augh_vread_max255_u8 = ASKSEND_MASK_NB; \
		augh_write(stdout, &augh_vread_max255_u8); \
		augh_vread_max255_u8 = nb; \
		augh_write(stdout, &augh_vread_max255_u8); \
		augh_vread_max255_end = (beg)+(nb); \
		for(augh_vread_max255_i=beg; augh_vread_max255_i<augh_vread_max255_end; augh_vread_max255_i++) { \
			augh_read(stdin, &augh_vread_max255_u8); \
			data[augh_vread_max255_i] = augh_vread_max255_u8; \
		} \
	}while(0)

	unsigned ugh_vread_nb;
	unsigned ugh_vread_nb_cur;
	unsigned ugh_vread_beg_cur;
	#define ugh_read_vector(data,beg,nb) do { \
		ugh_vread_nb = nb; \
		ugh_vread_beg_cur = beg; \
		while(ugh_vread_nb>0) { \
			if(ugh_vread_nb <= 255) ugh_vread_nb_cur = nb; else ugh_vread_nb_cur = 255; \
			augh_vread_max255(data, ugh_vread_beg_cur, ugh_vread_nb_cur); \
			ugh_vread_nb -= ugh_vread_nb_cur; \
			ugh_vread_beg_cur += ugh_vread_nb_cur; \
		} \
	}while(0)

	static inline void ugh_read_skip(unsigned nb) {
		unsigned i, nb_loc;
		uint8_t u8;
		while(nb>0) {
			if(nb <= 255) nb_loc = nb; else nb_loc = 255;
			// Send protocol stuff
			u8 = ASKSEND_MASK_NB;
			augh_write(stdout, &u8);
			u8 = nb_loc;
			augh_write(stdout, &u8);
			// Receive the data
			for(i=0; i<nb_loc; i++) augh_read(stdin, &u8);
			nb -= nb_loc;
		}
	}

#else

	static inline void write32_lr(uint32_t u32) {
		uint8_t i;
		uint8_t u8;
		for(i=0; i<4; i++) { u8 = (u32 >> 24) & 0xFF; u32 = u32 << 8; write8(u8); }
	}
	static inline void write32_rl(uint32_t u32) {
		uint8_t i;
		uint8_t u8;
		for(i=0; i<4; i++) { u8 = u32 & 0xFF; u32 = u32 >> 8; write8(u8); }
	}

	static inline uint16_t read16_be() {
		uint8_t b0;
		uint16_t buf;
		b0 = read8();
		buf = b0;
		b0 = read8();
		buf = (buf<<8) | b0;
		return buf;
	}

	static inline void write16_be(uint16_t u16) {
		uint8_t u8;
		u8 = u16 >> 8;
		write8(u8);
		u8 = u16;
		write8(u8);
	}

	unsigned ugh_vread_iter;
	unsigned ugh_vread_iter_end;
	#define ugh_read_vector(data,beg,nb) do { \
		ugh_vread_iter_end = (beg)+(nb); \
		for(ugh_vread_iter=beg; ugh_vread_iter<ugh_vread_iter_end; ugh_vread_iter++) data[ugh_vread_iter] = read8(); \
	}while(0)

	static inline void ugh_read_skip(unsigned nb) {
		unsigned iter;
		for(iter=0; iter<nb; iter++) read8();
	}

#endif

static inline void skip_segment() {
	uint16_t segment_size = read16_be();
	segment_size -= 2;
	ugh_read_skip(segment_size);
}

static inline void write_picture_byte(uint32_t address, uint8_t val) {
	write32_lr(address);
	write8(val);
}
static inline void write_picture_full() {}



//======================================================
// Utility functions
//======================================================

static inline uint32_t uint32ceil8 (uint32_t N) {
	return (N+7) / 8;
}

static inline int32_t reformat (uint32_t S, int32_t good) {
	int32_t St = 0;
	int32_t output = 0;
	if (good == 0) {}
	else {
		St = 1 << (good - 1);	/* 2^(good-1) */
		if (S < St) output = S + 1 + ((-1) << good);
		else output = S;
	}
	return output;
}

unsigned memset_iter;
#define array_set(dest,beg,nb,val) do { \
	for(memset_iter=beg; memset_iter<beg+nb; memset_iter++) dest[memset_iter] = val; \
}while(0)



//======================================================
// IDCT
//======================================================

int32_t idct_z1[8], idct_z2[8], idct_Yc[8];

int idct_k, idct_l;
int32_t idct_r1;

// This is the actual code
#define PLUSHIFT(x) ((((x)>>13)+1)>>1)

// These are tests because the above macro makes things ugly
//#define PLUSHIFT(x) ((((x)>>13)+1))
//#define PLUSHIFT(x) ((x)>>14)
//#define PLUSHIFT(x) ((x)+1)
//#define PLUSHIFT(x) (x)

// Access 1D arrays with 2D coordinates
//#define LINCOORD(x,y) (((x)<<3)+(y))
//#define LINCOORD(x,y) (((x)<<3)|(y))
#define LINCOORD8(x,y) (((x)<<3)|((y)&0x7))

#define IDCT_ID(input,k) do { \
	/* Stage 1 */ \
	idct_z1[1] = input[LINCOORD8(k,0)] - input[LINCOORD8(k,4)]; \
	idct_z1[0] = input[LINCOORD8(k,0)] + input[LINCOORD8(k,4)]; \
	idct_z1[2] = PLUSHIFT(8867  * input[LINCOORD8(k,2)]) - PLUSHIFT(21407 * input[LINCOORD8(k,6)]); \
	idct_z1[3] = PLUSHIFT(21407 * input[LINCOORD8(k,2)]) - PLUSHIFT(8867  * input[LINCOORD8(k,6)]); \
	idct_z1[4] = input[LINCOORD8(k,1)] - input[LINCOORD8(k,7)]; \
	idct_z1[7] = input[LINCOORD8(k,1)] + input[LINCOORD8(k,7)]; \
	idct_z1[5] = PLUSHIFT(23170 * input[LINCOORD8(k,3)]); \
	idct_z1[6] = PLUSHIFT(23170 * input[LINCOORD8(k,5)]); \
	/* Stage 2 */ \
	idct_z2[3] = idct_z1[0] - idct_z1[3]; \
	idct_z2[0] = idct_z1[0] + idct_z1[3]; \
	idct_z2[2] = idct_z1[1] - idct_z1[2]; \
	idct_z2[1] = idct_z1[1] + idct_z1[2]; \
	idct_z2[6] = idct_z1[4] - idct_z1[6]; \
	idct_z2[4] = idct_z1[4] + idct_z1[6]; \
	idct_z2[5] = idct_z1[7] - idct_z1[5]; \
	idct_z2[7] = idct_z1[7] + idct_z1[5]; \
	/* Stage 3 */ \
	idct_z1[0] = idct_z2[0]; \
	idct_z1[1] = idct_z2[1]; \
	idct_z1[2] = idct_z2[2]; \
	idct_z1[3] = idct_z2[3]; \
	idct_z1[4] = PLUSHIFT(13623 * idct_z2[4]) - PLUSHIFT(9102  * idct_z2[7]); \
	idct_z1[7] = PLUSHIFT(9102  * idct_z2[4]) + PLUSHIFT(13623 * idct_z2[7]); \
	idct_z1[5] = PLUSHIFT(16069 * idct_z2[5]) - PLUSHIFT(3196  * idct_z2[6]); \
	idct_z1[6] = PLUSHIFT(3196  * idct_z2[5]) + PLUSHIFT(16069 * idct_z2[6]); \
	/* Stage 4 */ \
	input[LINCOORD8(k,7)] = idct_z1[0] - idct_z1[7]; \
	input[LINCOORD8(k,0)] = idct_z1[0] + idct_z1[7]; \
	input[LINCOORD8(k,6)] = idct_z1[1] - idct_z1[6]; \
	input[LINCOORD8(k,1)] = idct_z1[1] + idct_z1[6]; \
	input[LINCOORD8(k,5)] = idct_z1[2] - idct_z1[5]; \
	input[LINCOORD8(k,2)] = idct_z1[2] + idct_z1[5]; \
	input[LINCOORD8(k,4)] = idct_z1[3] - idct_z1[4]; \
	input[LINCOORD8(k,3)] = idct_z1[3] + idct_z1[4]; \
}while(0)

// input is an int32_t array, 64 cells
// input is an uint8_t array, 64 cells
#define IDCT_2D(input,output) do { \
	for(idct_k=0; idct_k<8; idct_k++) { \
		for(idct_l=0; idct_l<8; idct_l++) input[LINCOORD8(idct_k,idct_l)] = input[LINCOORD8(idct_k,idct_l)] << 3; \
		IDCT_ID(input, idct_k); \
	} \
	for(idct_l=0; idct_l<8; idct_l++) { \
		for(idct_k=0; idct_k<8; idct_k++) idct_Yc[idct_k] = input[LINCOORD8(idct_k,idct_l)]; \
		IDCT_ID(idct_Yc, 0); \
		/* Descale */ \
		for(idct_k=0; idct_k<8; idct_k++) { \
			idct_r1 = ((idct_Yc[idct_k] + 0x20) >> 6) + 128; \
			if(idct_r1<0)        output[LINCOORD8(idct_k,idct_l)] = 0; \
			else if(idct_r1>255) output[LINCOORD8(idct_k,idct_l)] = 255; \
			else                 output[LINCOORD8(idct_k,idct_l)] = idct_r1; \
		} \
	} \
}while(0)


uint32_t idct_computer_i, idct_computer_c;
int32_t  idct_computer_input[64];
uint8_t  idct_computer_output[64];

#define LINCOORD64(x,y) (((x)<<6)|((y)&0x3F))

// input is an int32_t array
// output is an uint8_t array
#define idct_computer(flit_size,input,output) do { \
	for(idct_computer_i=0; idct_computer_i<flit_size; idct_computer_i++) { \
		augh_iter_nb = 96; /* User annotation */ \
		for(idct_computer_c=0; idct_computer_c<64; idct_computer_c++) { \
			idct_computer_input[idct_computer_c] = input[LINCOORD64(idct_computer_i, idct_computer_c)]; \
		} \
		IDCT_2D(idct_computer_input, idct_computer_output); \
		for(idct_computer_c=0; idct_computer_c<64; idct_computer_c++) { \
			output[LINCOORD64(idct_computer_i, idct_computer_c)] = idct_computer_output[idct_computer_c]; \
		} \
	} \
} while(0)



//======================================================
// Define the ZZ inversion table
//======================================================

const uint8_t G_ZZ[64] = {
	0,  1,  8,  16, 9,  2,  3,  10,
	17, 24, 32, 25, 18, 11, 4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13, 6,  7,  14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

uint32_t iqzz_index;

// int32_t in[64], int32_t out[64], uint8_t table[64]
#define iqzz_block(in,out,out_offset,table) do { \
	for(iqzz_index=0; iqzz_index<64; iqzz_index++) out[out_offset + G_ZZ[iqzz_index]] = in[iqzz_index] * table[iqzz_index]; \
} while(0)



//======================================================
// All data structures
//======================================================
// The usual C structures are not handled by UGH... sigh

// The JFIF Header

uint16_t jfif_header_length;	      /* Length of APP0 Field */
uint8_t  jfif_header_identifier[5]; /* "JFIF" (zero terminated) Id * String */
uint8_t  jfif_header_version[2];    /* JFIF Format Revision */
uint8_t  jfif_header_units;         /* Units used for Resolution */
uint16_t jfif_header_xdensity;      /* Horizontal Resolution */
uint16_t jfif_header_ydensity;      /* Vertical Resolution */
uint8_t  jfif_header_xthumbnail;    /* Horizontal Pixel Count */
uint8_t  jfif_header_ythumbnail;    /* Vertical Pixel Count */

void read_jfif_header() {
	jfif_header_length = read16_be();
	ugh_read_vector(jfif_header_identifier, 0, 5);
	ugh_read_vector(jfif_header_version, 0, 2);
	jfif_header_units = read8();
	jfif_header_xdensity = read16_be();
	jfif_header_ydensity = read16_be();
	jfif_header_xthumbnail = read8();
	jfif_header_ythumbnail = read8();
}

// The DQT section

uint16_t DQT_section_length;    /* Length of DQT section */
uint8_t  DQT_section_pre_quant; /* 4bits high : precision (0=8bits, * 1=16bits) */

void read_DQT_section() {
	DQT_section_length = read16_be();
	DQT_section_pre_quant = read8();
}

// The SOF section

uint16_t SOF_section_length;         /* Length of SOF section */
uint8_t  SOF_section_data_precision; /* Data bit precision (usually 8, * for baseline JPEG) */
uint16_t SOF_section_height;         /* Image height in pixels */
uint16_t SOF_section_width;          /* Image width in pixels */
uint8_t  SOF_section_n;              /* n = Number of components (eg. 3 for * RGB, 1 for Grayscale) */

void read_SOF_section() {
	SOF_section_length = read16_be();
	SOF_section_data_precision = read8();
	SOF_section_height = read16_be();
	SOF_section_width = read16_be();
	SOF_section_n = read8();
}

// The SOF components

uint8_t SOF_component_index[3];   /* Component index */
uint8_t SOF_component_HV[3];      /* H(4bits high) and V(4bits low) sampling * factors */
uint8_t SOF_component_q_table[3]; /* Quantization table associated */

static inline void read_SOF_component(int nb) {
	int iter;
	for(iter=0; iter<nb; iter++) {
		augh_iter_nb = 3; /* User annotation */
		SOF_component_index[iter] = read8();
		SOF_component_HV[iter] = read8();
		SOF_component_q_table[iter] = read8();
	}
}

// The DHT section

uint16_t DHT_section_length;    /* Length in byte of this section, * including this data. */
uint8_t  DHT_section_huff_info; /* Huffman Table information */

void read_DHT_section() {
	DHT_section_length = read16_be();
	DHT_section_huff_info = read8();
}


// The SOS section

uint16_t SOS_section_length; /* Length in byte of this section, * including this data. */
uint8_t  SOS_section_n;      /* N = Number of components */

void read_SOS_section() {
	SOS_section_length = read16_be();
	SOS_section_n = read8();
}

// The SOS component

uint8_t SOS_component_index[3];
uint8_t SOS_component_acdc[3];

static inline void read_SOS_component(int nb) {
	int iter;
	for(iter=0; iter<nb; iter++) {
		augh_iter_nb = 3; /* User annotation */
		SOS_component_index[iter] = read8();
		SOS_component_acdc[iter] = read8();
	}
}

// The scan_desc structure

uint8_t scan_desc_bit_count;
uint8_t scan_desc_window;
int32_t scan_desc_pred[3];

int32_t scan_desc_table_ac_ValPtr[3][16];
int32_t scan_desc_table_ac_MinCode[3][16];
int32_t scan_desc_table_ac_MaxCode[3][16];
uint8_t scan_desc_table_ac_table[3][MAX_SIZE_AC];

int32_t scan_desc_table_dc_ValPtr[3][16];
int32_t scan_desc_table_dc_MinCode[3][16];
int32_t scan_desc_table_dc_MaxCode[3][16];
uint8_t scan_desc_table_dc_table[3][MAX_SIZE_DC];

// The huffman tables

int32_t tables_ac_ValPtr[4][16];
int32_t tables_ac_MinCode[4][16];
int32_t tables_ac_MaxCode[4][16];
uint8_t tables_ac_table[4][MAX_SIZE_AC];

int32_t tables_dc_ValPtr[4][16];
int32_t tables_dc_MinCode[4][16];
int32_t tables_dc_MaxCode[4][16];
uint8_t tables_dc_table[4][MAX_SIZE_DC];

static inline void copy_tables_to_scan_desc(unsigned nb) {
	unsigned iter, k;
	unsigned index_ac, index_dc;
	for(iter=0; iter<nb; iter++) {
		augh_iter_nb = 3; /* User annotation */
		scan_desc_pred[iter] = 0;
		index_ac = (SOS_component_acdc[iter] >> 4) & 0x0f;
		index_dc = SOS_component_acdc[iter] & 0x0f;
		for(k=0; k<16; k++) {
			// AC tables
			scan_desc_table_ac_ValPtr[iter][k] = tables_ac_ValPtr[index_ac][k];
			scan_desc_table_ac_MinCode[iter][k] = tables_ac_MinCode[index_ac][k];
			scan_desc_table_ac_MaxCode[iter][k] = tables_ac_MaxCode[index_ac][k];
			// DC tables
			scan_desc_table_dc_ValPtr[iter][k] = tables_dc_ValPtr[index_dc][k];
			scan_desc_table_dc_MinCode[iter][k] = tables_dc_MinCode[index_dc][k];
			scan_desc_table_dc_MaxCode[iter][k] = tables_dc_MaxCode[index_dc][k];
		}
		for(k=0; k<MAX_SIZE_AC; k++) scan_desc_table_ac_table[iter][k] = tables_ac_table[index_ac][k];
		for(k=0; k<MAX_SIZE_DC; k++) scan_desc_table_dc_table[iter][k] = tables_dc_table[index_dc][k];
	}
}

// The main buffers

uint8_t DECODED_FLIT[100*64];  // flit_size*64 (empirically flit_size = 96)
int32_t MCU[64];

// The pre-computed values

unsigned val_YH_YV;  // YH * YV



//======================================================
// The "Builder"
//======================================================

#define MCU_INDEX(ptr, index) (ptr + ((index) * MCU_sx * MCU_sy))
#define MCU_LINE(ptr,n) (ptr + ((n) * MCU_sx))

#define FB_Y_LINE(ptr,n) (ptr + ((n) * MCU_sx * NB_MCU_X))
#define FB_Y_INDEX(ptr,x,y) \
  (ptr + ((y) * MCU_sy * MCU_sx * NB_MCU_X) + ((x) * MCU_sx))

#define FB_UV_LINE(ptr,n) (ptr + (((n) * MCU_sx * NB_MCU_X) >> 1))
#define FB_U_INDEX(ptr,x,y) \
  (ptr + (((y) * MCU_sy * SOF_section_width) >> 1) +  \
   (((x) * MCU_sx >> 1)) + (SOF_section_width * SOF_section_height))

#define FB_V_INDEX(ptr,x,y) \
  (ptr + (((y) * MCU_sy * SOF_section_width >> 1)) + (((x) * MCU_sx >> 1))  \
   + ((SOF_section_width * SOF_section_height * 3) >> 1))


static inline void builder(uint32_t YV, uint32_t YH, uint32_t flit_size) {
	// index in the DECODED_FLIT
	uint32_t CELLS_SRC;
	uint32_t Y_SRC, Y_DST;
	uint32_t U_SRC, U_DST;
	uint32_t V_SRC, V_DST;
	uint32_t uv_line_src, uv_line_dst;

	uint16_t NB_MCU_X = 0, NB_MCU_Y = 0;
	uint16_t NB_CELLS = 0;

	uint32_t flit_bytes = 0;
  static uint32_t LB_X = 0, LB_Y = 0;
	uint32_t y_line_dst, y_line_src;

  int flit_index;
	int cell_y_index;
	int cell_x_index;
	int line_index;
	int i;

	// Pre-computed values
	unsigned for_limit;     // Original expression : MCU_sx / (3 - YH)
	unsigned val_3_min_HY;  // Original expression : 3 - YH

	switch(YH) {
		case 1 :
			for_limit = MCU_sx >> 1;
			val_3_min_HY = 2;
			break;
		case 2 :
		default :
			for_limit = MCU_sx;
			val_3_min_HY = 1;
			break;
		//case 4 : MCU_sx / (3 - YH) FIXME don't know what happens here ?
	}

	flit_bytes = flit_size * MCU_sx * MCU_sy;

	NB_MCU_X = uint32ceil8(SOF_section_width);
	NB_MCU_Y = uint32ceil8(SOF_section_height);
	NB_CELLS = val_YH_YV + 2;

  for (flit_index = 0; flit_index < flit_size; flit_index += NB_CELLS) {
		augh_iter_nb = 32;  // User annotation
		CELLS_SRC = MCU_INDEX(0, flit_index);

		for (cell_y_index = 0; cell_y_index < YV; cell_y_index += 1) {
			augh_iter_nb = 1;  // User annotation

			// The Y component
			for (cell_x_index = 0; cell_x_index < YH; cell_x_index += 1) {
				augh_iter_nb = 1;  // User annotation
				Y_SRC = MCU_INDEX(CELLS_SRC, (YH * cell_y_index + cell_x_index));
				Y_DST = FB_Y_INDEX(0, LB_X + cell_x_index, LB_Y + cell_y_index);

				for (line_index = 0; line_index < MCU_sy; line_index += 1) {
					y_line_src = MCU_LINE(Y_SRC, line_index);
					y_line_dst = FB_Y_LINE(Y_DST, line_index);
					write_picture_byte(y_line_dst+0, DECODED_FLIT[y_line_src+0]);
					write_picture_byte(y_line_dst+1, DECODED_FLIT[y_line_src+1]);
					write_picture_byte(y_line_dst+2, DECODED_FLIT[y_line_src+2]);
					write_picture_byte(y_line_dst+3, DECODED_FLIT[y_line_src+3]);
					write_picture_byte(y_line_dst+4, DECODED_FLIT[y_line_src+4]);
					write_picture_byte(y_line_dst+5, DECODED_FLIT[y_line_src+5]);
					write_picture_byte(y_line_dst+6, DECODED_FLIT[y_line_src+6]);
					write_picture_byte(y_line_dst+7, DECODED_FLIT[y_line_src+7]);
					y_line_dst += 8;
					y_line_src += 8;
				}
			}

			// The U component
			U_SRC = MCU_INDEX(CELLS_SRC, val_YH_YV);
			U_DST = FB_U_INDEX(0, LB_X, LB_Y + cell_y_index);

			for (line_index = 0; line_index < MCU_sy; line_index += 1) {
				uv_line_src = MCU_LINE(U_SRC, line_index);
				uv_line_dst = FB_UV_LINE(U_DST, line_index);

				for (i = 0; i < for_limit; i += 1) {
					augh_iter_nb = 4;  // User annotation
					write_picture_byte(uv_line_dst + i, DECODED_FLIT[uv_line_src + i * val_3_min_HY]);
				}
			}

			// The V component
			V_SRC = MCU_INDEX(CELLS_SRC, (val_YH_YV + 1));
			V_DST = FB_V_INDEX(0, LB_X, LB_Y + cell_y_index);

			for (line_index = 0; line_index < MCU_sy; line_index += 1) {
				uv_line_src = MCU_LINE(V_SRC, line_index);
				uv_line_dst = FB_UV_LINE(V_DST, line_index);

				for (i = 0; i < for_limit; i += 1) {
					augh_iter_nb = 4;  // User annotation
					write_picture_byte(uv_line_dst + i, DECODED_FLIT[uv_line_src + i * val_3_min_HY]);
				}
			}
		}  // Loop on cell_y_index

    // Original expression : LB_X = (LB_X + YH) % NB_MCU_X;
    LB_X += YH;
    if(LB_X >= NB_MCU_X) LB_X -= NB_MCU_X;

    if (LB_X == 0) {

      // Original expression = LB_Y = (LB_Y + YV) % NB_MCU_Y;
      LB_Y += YV;
      if(LB_Y >= NB_MCU_Y) LB_Y -= NB_MCU_Y;

      if (LB_Y == 0) {
        // TODO: Send the picture to someone !
      }
    }
  }
}


//======================================================
// Miscellaneous decoder functions
//======================================================

static inline void load_huffman_table_ac (unsigned index_table) {
	uint8_t buffer = 0;
	int32_t size = 0, max = 0;
	int32_t LeavesN = 0, LeavesT = 0, i = 0;
	int32_t AuxCode = 0;

	for (i = 0; i < 16; i++) {
		buffer = read8();
		LeavesN = buffer;
		tables_ac_ValPtr[index_table][i] = LeavesT;
		tables_ac_MinCode[index_table][i] = AuxCode * 2;
		AuxCode = tables_ac_MinCode[index_table][i] + LeavesN;
		tables_ac_MaxCode[index_table][i] = (LeavesN) ? (AuxCode - 1) : (-1);
		LeavesT += LeavesN;
	}

	size = DHT_section_length - 2 - 1 - 16;
	if(LeavesT > MAX_SIZE_AC) {
		max = MAX_SIZE_AC;
		VPRINTF("WARNING: Truncating Table by %lu symbols\r\n", LeavesT - max);
	}
	else max = LeavesT;

	ugh_read_vector(tables_ac_table[index_table], 0, max);
	ugh_read_skip(LeavesT - max);
}
static inline void load_huffman_table_dc (unsigned index_table) {
	uint8_t buffer = 0;
	int32_t size = 0, max = 0;
	int32_t LeavesN = 0, LeavesT = 0, i = 0;
	int32_t AuxCode = 0;

	for (i = 0; i < 16; i++) {
		buffer = read8();
		LeavesN = buffer;
		tables_dc_ValPtr[index_table][i] = LeavesT;
		tables_dc_MinCode[index_table][i] = AuxCode * 2;
		AuxCode = tables_dc_MinCode[index_table][i] + LeavesN;
		tables_dc_MaxCode[index_table][i] = (LeavesN) ? (AuxCode - 1) : (-1);
		LeavesT += LeavesN;
	}

	size = DHT_section_length - 2 - 1 - 16;
	if(LeavesT > MAX_SIZE_DC) {
		max = MAX_SIZE_DC;
		VPRINTF("WARNING: Truncating Table by %lu symbols\r\n", LeavesT - max);
	}
	else max = LeavesT;

	ugh_read_vector(tables_dc_table[index_table], 0, max);
	ugh_read_skip(LeavesT - max);
}

static inline uint32_t get_bits (uint8_t number) {
	int32_t i = 0, newbit = 0;
	uint32_t result = 0;
	uint8_t wwindow = 0, aux = 0;

	if (number == 0) {}
	else {
		for (i = 0; i < number; i++) {
			augh_iter_nb = 1;  // User annotation

			if (scan_desc_bit_count == 0) {
				augh_branch_prob_m = 125;  // User annotation
				wwindow = read8();
				scan_desc_bit_count = 8;
				if (wwindow == 0xFF) aux = read8();
			}
			else wwindow = scan_desc_window;

			newbit = (wwindow >> 7) & 1;
			scan_desc_window = wwindow << 1;
			scan_desc_bit_count -= 1;
			result = (result << 1) | newbit;
		}
	}

	return result;
}

static inline uint8_t get_symbol_ac (uint32_t component) {
	uint8_t temp = 0;
	int32_t code = 0;
	uint32_t length = 0, index = 0;
	uint8_t output = 0;

	for (length = 0; length < 16; length++) {
		augh_iter_nb_m = 3293;  // User annotation
		temp = get_bits (1);
		code = (2 * code) | temp;
		if (code <= scan_desc_table_ac_MaxCode[component][length]) {
			augh_branch_prob_m = 303;  // User annotation
			index = scan_desc_table_ac_ValPtr[component][length] + code - scan_desc_table_ac_MinCode[component][length];
			if (index < MAX_SIZE_AC) output = scan_desc_table_ac_table[component][index];
			break;
		}
	}

	return output;
}
static inline uint8_t get_symbol_dc (uint32_t component) {
	uint8_t temp = 0;
	int32_t code = 0;
	uint32_t length = 0, index = 0;
	uint8_t output = 0;

	for (length = 0; length < 16; length++) {
		augh_iter_nb_m = 1883;  // User annotation
		temp = get_bits (1);
		code = (2 * code) | temp;
		if (code <= scan_desc_table_dc_MaxCode[component][length]) {
			augh_branch_prob_m = 530;  // User annotation
			index = scan_desc_table_dc_ValPtr[component][length] + code - scan_desc_table_dc_MinCode[component][length];
			if (index < MAX_SIZE_DC) output = scan_desc_table_dc_table[component][index];
			break;
		}
	}

	return output;
}

static inline void unpack_block (uint32_t index) {
	uint32_t temp = 0, i = 0, run = 0, cat = 0;
	int32_t value = 0;
	uint8_t symbol = 0;

	array_set(MCU, 0, 64, 0);
	symbol = get_symbol_dc(index);
	temp = get_bits (symbol);

	value = reformat (temp, symbol);
	value += scan_desc_pred[index];
	scan_desc_pred[index] = value;

	MCU[0] = value;

	for (i = 1; i < 64; i++) {
		augh_iter_nb_m = 3400;
		symbol = get_symbol_ac(index);

		if (symbol == HUFF_EOB) {
			augh_branch_prob_m = 292;  // User annotation
			break;
		}
		else if (symbol == HUFF_ZRL) {
			i += 15;
		}
		else {
			augh_branch_prob = 100;  // User annotation
			cat = symbol & 15;
			run = (symbol >> 4) & 15;
			i += run;
			temp = get_bits (cat);
			value = reformat(temp, cat);
			MCU[i] = value;
		}
	}
}



//======================================================
// The main decoder function
//======================================================

#if defined(GCC_COMPIL) || defined(APES_COMPIL)
int main()
#endif
#ifdef AUGH_SYNTHESIS
void augh_main()
#endif
{
	bool dispatch_info = true;
	bool soi_started = false;

	uint8_t marker0, marker1;
	uint8_t HT_type = 0, HT_index = 0;
	uint8_t DQT_table[4][64], index = 0, QT_index = 0;

	uint16_t nb_MCU = 0, nb_MCU_sx = 0;

  int32_t FLIT[96*64];
	uint32_t YH = 0, YV = 0, flit_size = 0, mcu_size = 0;

	// Initialization
	// Because UGH refuses initialization of global variables...

	scan_desc_bit_count = 0;
	scan_desc_window = 0;

	#ifdef ASKSEND
	read8();
	#endif

  // Computation loop

	while (1) {
		augh_iter_nb = 10;  // User annotation : 10 markers are read in switch()

		marker0 = read8();

		if(marker0 == M_SMS && soi_started==true) {
			augh_branch_prob = 100;
			marker1 = read8();

			switch (marker1) {

				case M_EOI: {
					augh_branch_prob = 10;  // User annotation

					IPRINTF("EOI marker found\r\n");

					#if defined(GCC_COMPIL) || defined(APES_COMPIL)
					return 0;
					#endif

					soi_started = false;
					break;
				}

				case M_SOF0: {
					augh_branch_prob = 10;  // User annotation

					IPRINTF("SOF0 marker found\r\n");

					read_SOF_section();

					IPRINTF("Data precision = %u\r\n", SOF_section_data_precision);
					IPRINTF("Image height   = %u\r\n", SOF_section_height);
					IPRINTF("Image width    = %u\r\n", SOF_section_width);
					IPRINTF("Components     = %u\r\n", SOF_section_n);

					read_SOF_component(SOF_section_n);

					YV = SOF_component_HV[0] & 0x0f;
					YH = (SOF_component_HV[0] >> 4) & 0x0f;

					// Pre-computed value
					val_YH_YV = YH * YV;

					IPRINTF("Subsampling factors : YH=%lu YV=%lu\r\n", YH, YV);

					// Only dispatch the picture info once because all pictures have the same format

					if (dispatch_info==true) {
						// Number of 8x8 blocks in a line
						nb_MCU_sx = uint32ceil8(SOF_section_width);
						IPRINTF("nb_MCU_sx : %lu\r\n", nb_MCU_sx);
						// The first component is oversampled
						// Original expression : flit_size = YV * nb_MCU_sx + (SOF_section_n - 1) * nb_MCU_sx / YH;
						switch(YH) {
							case 1 : flit_size = YV * nb_MCU_sx + (SOF_section_n - 1) * nb_MCU_sx; break;
							case 2 : flit_size = YV * nb_MCU_sx + (SOF_section_n - 1) * (nb_MCU_sx >> 1); break;
							case 4 : flit_size = YV * nb_MCU_sx + (SOF_section_n - 1) * (nb_MCU_sx >> 2); break;
						}
						IPRINTF("Flit size : %lu\r\n", flit_size);
						mcu_size = val_YH_YV + SOF_section_n - 1;
						IPRINTF("MCU size : %lu\r\n", mcu_size);

						dispatch_info = false;
					}

					break;
				}

				case M_DHT: {
					augh_branch_prob = 40;  // User annotation

					IPRINTF("DHT marker found\r\n");

					read_DHT_section();

					HT_index = DHT_section_huff_info & 0x0f;
					HT_type = (DHT_section_huff_info >> 4) & 0x01;

					VPRINTF("Huffman table index is %d\r\n", HT_index);
					VPRINTF("Huffman table type is %s\r\n", HT_type ? "AC" : "DC");

					VPRINTF("Loading Huffman table\r\n");
					if(HT_type==HUFF_AC) load_huffman_table_ac(HT_index);
					else load_huffman_table_dc(HT_index);

					break;
				}

				case M_SOS: {
					augh_branch_prob = 10;  // User annotation
					IPRINTF("SOS marker found\r\n");

					read_SOS_section();

					read_SOS_component(SOS_section_n);
					IPRINTF("Scan with %d components\r\n", SOS_section_n);

					ugh_read_skip(SOS_section_n);

					scan_desc_bit_count = 0;

					copy_tables_to_scan_desc(SOS_section_n);

					nb_MCU = uint32ceil8(SOF_section_height) * uint32ceil8(SOF_section_width);
					IPRINTF("%d MCU to unpack, step is %d\r\n", nb_MCU, YV * nb_MCU_sx);

					for( ; nb_MCU!=0; nb_MCU -= YV * nb_MCU_sx) {
						uint32_t step;
						augh_iter_nb = 18;  // User annotation
						for (step = 0; step < flit_size; step += mcu_size) {
							augh_iter_nb = 32; // User annotation
							for (index = 0; index < val_YH_YV; index++) {
								augh_iter_nb = 1; // User annotation
								unpack_block (0);
								iqzz_block (MCU, FLIT, (step + index) * 64, DQT_table[SOF_component_q_table[0]]);
							}
							for (index =  1; index < SOF_section_n; index++) {
								augh_iter_nb = 2; // User annotation
								unpack_block (index);
								iqzz_block (MCU, FLIT, (step + (val_YH_YV + index - 1)) * 64, DQT_table[SOF_component_q_table[index]]);
							}
						}
						idct_computer(flit_size, FLIT, DECODED_FLIT);
						builder(YV, YH, flit_size);
					}

          // Note : this call could do nothing if the picture is already sent in the builder() function.
          write_picture_full();

          break;
				}

				case M_DQT: {
					augh_branch_prob = 20;  // User annotation

					IPRINTF("DQT marker found\r\n");

					read_DQT_section();

					VPRINTF("Quantization precision is %s\r\n", ((DQT_section_pre_quant >> 4) & 0x0f) ? "16 bits" :	"8 bits");

					QT_index = DQT_section_pre_quant & 0x0f;
					VPRINTF("Quantization table index is %d\r\n", QT_index);

					VPRINTF("Reading quantization table\r\n");
					ugh_read_vector(DQT_table[QT_index], 0, 64);

					break;
				}

				case M_APP0: {
					augh_branch_prob = 10;  // User annotation

					IPRINTF("APP0 marker found\r\n");

					read_jfif_header();

					if (jfif_header_identifier[0] != 'J' ||
              jfif_header_identifier[1] != 'F' ||
							jfif_header_identifier[2] != 'I' ||
              jfif_header_identifier[3] != 'F')
          {
						VPRINTF("Not a JFIF file\r\n");
					}

					break;
				}

				default: {
					IPRINTF("Unused token: 0x%x\r\n", marker1);
					skip_segment ();
					break;
				}
			}
		}
		else if(marker0 == M_SMS) {
			marker1 = read8();
			if(marker1==M_SOI) {
				IPRINTF("SOI marker found\r\n");
				soi_started = true;
			}
		}

	}  // Main infinite loop

	#if defined(GCC_COMPIL) || defined(APES_COMPIL)
	return 0;
	#endif
}

