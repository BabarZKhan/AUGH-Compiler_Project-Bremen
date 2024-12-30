// FIXME Get updates from upstream CHStone about #ifdef RAND_VAL

#include <augh_annot.h>

#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)

	#include <augh.h>

	uint8_t  stdin;
	uint32_t stdout;

	static inline uint8_t fifo_read8() {
		uint8_t u8;
		augh_read(stdin, &u8);
		return u8;
	}

	#define read8 fifo_read8

	#define write32(data) augh_write(stdout, &data)

#endif

#if defined(AUGH_SYNTHESIS) && defined(STDIO8)

	#include "../stdio8_defs.h"

	#define write32 write32_be

#endif

#ifdef GCC_COMPIL

	#include <stdio.h>
	#include <stdint.h>

	static inline uint8_t read8() {
		return getchar();
	}

#endif



#define Num 2048

/*
+--------------------------------------------------------------------------+
| * Test Vectors (added for CHStone)                                       |
|     inRdbfr, inPMV, inPMV : input data                                   |
|     outPMV, outmvfs : expected output data                               |
+--------------------------------------------------------------------------+
*/

#define ERROR (-1)
#define SEQUENCE_END_CODE       0x1B7
#define MV_FIELD 0

const int inPMV[2][2][2] =  { {{45, 207}, {70, 41}}, {{4, 180}, {120, 216}} };
const int inmvfs[2][2] =    { {232, 200}, {32, 240} };
const int outPMV[2][2][2] = { {{1566, 206}, {70, 41}}, {{1566, 206}, {120, 216}} };
const int outmvfs[2][2] =   { {0, 200}, {0, 240} };

int PMV[2][2][2];
int dmvector[2];
int motion_vertical_field_select[2][2];

int evalue;

int System_Stream_Flag;

unsigned char ld_Rdbfr[2048];
unsigned int ld_Rdidx, ld_Rdmax;
unsigned int ld_Bfr;
int ld_Incnt;



/* initialize buffer, call once before first getbits or showbits */
static inline int read (unsigned start, unsigned n) {
	unsigned i;
	for(i=0; i<n; i++) ld_Rdbfr[start+i] = read8();
	return n;
}

void Fill_Buffer () {
	unsigned Buffer_Level;

	Buffer_Level = read (0, 2048);
	ld_Rdidx = 0;

	if (System_Stream_Flag) ld_Rdmax -= 2048;

	/* end of the bitstream file */
	if (Buffer_Level < 2048) {
		augh_branch_prob = 0;  // User annotation

		/* just to be safe */
		if (Buffer_Level < 0) Buffer_Level = 0;

		/* pad until the next to the next 32-bit word boundary */
		while (Buffer_Level & 3) ld_Rdbfr[Buffer_Level++] = 0;

		/* pad the buffer with sequence end codes */
		while(Buffer_Level < 2048) {
			ld_Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE >> 24;
			ld_Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE >> 16;
			ld_Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE >> 8;
			ld_Rdbfr[Buffer_Level++] = SEQUENCE_END_CODE & 0xff;
		}
	}
}

static inline unsigned Show_Bits (unsigned N) {
	unsigned s = 32;

	// Doesn't work with AUGH...
	//s = (s - N) % 32;

	// Doesn't work with AUGH...
	//s = (s - (N%32)) % 32;

	// Works with AUGH!!!
	s = s - (N % 32);
	s = s % 32;

	return ld_Bfr >> s;
}

/* advance by n bits */

void Flush_Buffer (unsigned N) {
	int Incnt;

	ld_Bfr <<= N;
	ld_Incnt -= N;
	Incnt = ld_Incnt;

	if (Incnt > 24) {
		augh_branch_prob_m = 572;  // User annotation
		return;
	}

	if (ld_Rdidx < 2044) {
		augh_branch_prob = 66;  // User annotation
		do {
			augh_iter_nb = 13;  // User annotation
			ld_Bfr |= ld_Rdbfr[ld_Rdidx++] << (24 - Incnt);
			Incnt += 8;
		} while (Incnt <= 24);
	}
	else {
		//ugh_branch_prob = 33;  // User annotation
		do {
			augh_iter_nb = 4;  // User annotation
			if (ld_Rdidx >= 2048) {
				augh_branch_prob = 25;  // User annotation
				Fill_Buffer ();
			}
			ld_Bfr |= ld_Rdbfr[ld_Rdidx++] << (24 - Incnt);
			Incnt += 8;
		} while (Incnt <= 24);
	}
	ld_Incnt = Incnt;
}

/* return next n bits (right adjusted) */

static inline unsigned int Get_Bits (int N) {
	unsigned int Val = Show_Bits (N);
	Flush_Buffer (N);
	return Val;
}

/* return next bit (could be made faster than Get_Bits(1)) */

static inline unsigned int Get_Bits1 () {
	return Get_Bits (1);
}



/* NOTE: #define constants such as MACROBLOCK_QUANT are upper case
	 as per C programming convention. However, the MPEG document
	 (ISO/IEC 13818-2) lists them in all lower case (e.g. Annex B) */

/* NOTE: the VLC tables are in a flash format---a transformation
	 of the tables in Annex B to a form more convenient towards
	 parallel (more than one-bit-at-a-time) decoding */


/* Table B-10, motion_code, codes 0001 ... 01xx */
const char MVtab0[8][2] = {
	{ERROR, 0}, {3, 3}, {2, 2}, {2, 2},
	{1, 1}, {1, 1}, {1, 1}, {1, 1}
};

/* Table B-10, motion_code, codes 0000011 ... 000011x */
const char MVtab1[8][2] = {
	{ERROR, 0}, {ERROR, 0}, {ERROR, 0}, {7, 6},
	{6, 6}, {5, 6}, {4, 5}, {4, 5}
};

/* Table B-10, motion_code, codes 0000001100 ... 000001011x */
const char MVtab2[12][2] = {
	{16, 9}, {15, 9}, {14, 9}, {13, 9}, {12, 9}, {11, 9},
	{10, 8}, {10, 8}, {9, 8}, {9, 8}, {8, 8}, {8, 8}
};


int Get_motion_code () {
	int code;
	int result;

	if (Get_Bits1 ()) {
		augh_branch_prob = 50;  // User annotation
		return 0;
	}

	code = Show_Bits (9);
	if (code >= 64) {
		augh_branch_prob = 0;  // User annotation
		code >>= 6;
		Flush_Buffer (MVtab0[code][1]);
		result = Get_Bits1 ()? -MVtab0[code][0] : MVtab0[code][0];
	}
	else {
		if (code >= 24) {
			augh_branch_prob = 100;  // User annotation
			code >>= 3;
			Flush_Buffer (MVtab1[code][1]);
			result = Get_Bits1 ()? -MVtab1[code][0] : MVtab1[code][0];
		}
		else {
			code -= 12;
			if(code < 0) result = 0;
			else {
				Flush_Buffer (MVtab2[code][1]);
				result = Get_Bits1 ()? -MVtab2[code][0] : MVtab2[code][0];
			}
		}
	}

	return result;
}

/* get differential motion vector (for dual prime prediction) */
int Get_dmvector () {
	int result = 0;
	unsigned c = Get_Bits (1);
	if(c!=0) {
		c = Get_Bits (1);
		result = c!=0 ? -1 : 1;
	}
	return result;
}



/* calculate motion vector component */
/* ISO/IEC 13818-2 section 7.6.3.1: Decoding the motion vectors */
/* Note: the arithmetic here is more elegant than that which is shown
	 in 7.6.3.1.  The end results (PMV[][][]) should, however, be the same.  */

void decode_motion_vector (
	PMV_idx1, PMV_idx2, PMV_idx3, r_size, motion_code, motion_residual, full_pel_vector
)
	unsigned PMV_idx1, PMV_idx2, PMV_idx3;
	int r_size, motion_code, motion_residual;
	int full_pel_vector;	/* MPEG-1 (ISO/IEC 11172-1) support */
{
	int lim, vec;

	r_size = r_size % 32;
	lim = 16 << r_size;
	vec = full_pel_vector ? (PMV[PMV_idx1][PMV_idx2][PMV_idx3] >> 1) : PMV[PMV_idx1][PMV_idx2][PMV_idx3];

	if (motion_code > 0) {
		vec += ((motion_code - 1) << r_size) + motion_residual + 1;
		if (vec >= lim) vec -= lim + lim;
	}
	else if (motion_code < 0) {
		vec -= ((-motion_code - 1) << r_size) + motion_residual + 1;
		if (vec < -lim) vec += lim + lim;
	}
	PMV[PMV_idx1][PMV_idx2][PMV_idx3] = full_pel_vector ? (vec << 1) : vec;
}

/* get and decode motion vector and differential motion vector
	 for one prediction */
void motion_vector (
	PMV_idx1, PMV_idx2, h_r_size, v_r_size, dmv, mvscale, full_pel_vector
)
	unsigned PMV_idx1, PMV_idx2;
	int h_r_size, v_r_size;
	int dmv;			/* MPEG-2 only: get differential motion vectors */
	int mvscale;		/* MPEG-2 only: field vector in frame pic */
	int full_pel_vector;	/* MPEG-1 only */
{
	int motion_code;
	int motion_residual;

	/* horizontal component */
	/* ISO/IEC 13818-2 Table B-10 */
	motion_code = Get_motion_code ();

	if(h_r_size != 0 && motion_code != 0) {
		augh_branch_prob = 100;  // User annotation
		motion_residual = Get_Bits (h_r_size);
	}
	else motion_residual = 0;

	decode_motion_vector (PMV_idx1, PMV_idx2, 0, h_r_size, motion_code, motion_residual, full_pel_vector);

	if (dmv) dmvector[0] = Get_dmvector ();

	/* vertical component */
	motion_code = Get_motion_code ();

	if(v_r_size != 0 && motion_code != 0) {
		augh_branch_prob = 0;  // User annotation
		motion_residual = Get_Bits (v_r_size);
	}
	else motion_residual = 0;

	if (mvscale) PMV[PMV_idx1][PMV_idx2][1] >>= 1;		/* DIV 2 */

	decode_motion_vector (PMV_idx1, PMV_idx2, 1, v_r_size, motion_code, motion_residual, full_pel_vector);

	if (mvscale) PMV[PMV_idx1][PMV_idx2][1] <<= 1;

	if (dmv) dmvector[1] = Get_dmvector ();
}

/* ISO/IEC 13818-2 sections 6.2.5.2, 6.3.17.2, and 7.6.3: Motion vectors */
void motion_vectors (
	s, motion_vector_count, mv_format, h_r_size, v_r_size, dmv, mvscale
)
	int s, motion_vector_count, mv_format, h_r_size, v_r_size, dmv, mvscale;
{
	if (motion_vector_count == 1) {
		augh_branch_prob = 100;  // User annotation
		if (mv_format == MV_FIELD && !dmv) {
			augh_branch_prob = 100;  // User annotation
			motion_vertical_field_select[1][s] =
			motion_vertical_field_select[0][s] = Get_Bits (1);
		}

		motion_vector (0, s, h_r_size, v_r_size, dmv, mvscale, 0);

		/* update other motion vector predictors */
		PMV[1][s][0] = PMV[0][s][0];
		PMV[1][s][1] = PMV[0][s][1];
	}
	else {
		motion_vertical_field_select[0][s] = Get_Bits (1);
		motion_vector (0, s, h_r_size, v_r_size, dmv, mvscale, 0);
		motion_vertical_field_select[1][s] = Get_Bits (1);
		motion_vector (1, s, h_r_size, v_r_size, dmv, mvscale, 0);
	}
}



void Initialize_Buffer () {
	ld_Incnt = 0;
	ld_Rdidx = 2048;
	ld_Rdmax = ld_Rdidx;
	ld_Bfr = 68157440;
	Flush_Buffer (0);		/* fills valid data into bfr */
}


#ifdef AUGH_SYNTHESIS

void augh_main () {
	int i, j, k;
	int main_result;
	int s, motion_vector_count, mv_format, h_r_size, v_r_size, dmv, mvscale;

	#ifdef ASKSEND
	read8();
	#endif

	do {

		main_result = 0;
		evalue = 0;
		System_Stream_Flag = 0;
		s = 0;
		motion_vector_count = 1;
		mv_format = 0;
		h_r_size = 200;
		v_r_size = 200;
		dmv = 0;
		mvscale = 1;

		for (i = 0; i < 2; i++) {
			dmvector[i] = 0;
			for (j = 0; j < 2; j++) {
				motion_vertical_field_select[i][j] = inmvfs[i][j];
				for (k = 0; k < 2; k++) PMV[i][j][k] = inPMV[i][j][k];
			}
		}

		Initialize_Buffer();
		motion_vectors(s, motion_vector_count, mv_format, h_r_size, v_r_size, dmv, mvscale);

		for (i = 0; i < 2; i++) {
			for (j = 0; j < 2; j++) {
				main_result += (motion_vertical_field_select[i][j] != outmvfs[i][j]);
				for (k = 0; k < 2; k++) main_result += (PMV[i][j][k] != outPMV[i][j][k]);
			}
		}

		write32(main_result);

	} while(1);

}

#endif


#ifdef GCC_COMPIL

int main () {
	int i, j, k;
	int main_result;
	int s, motion_vector_count, mv_format, h_r_size, v_r_size, dmv, mvscale;

	main_result = 0;
	evalue = 0;
	System_Stream_Flag = 0;
	s = 0;
	motion_vector_count = 1;
	mv_format = 0;
	h_r_size = 200;
	v_r_size = 200;
	dmv = 0;
	mvscale = 1;

	for (i = 0; i < 2; i++) {
		dmvector[i] = 0;
		for (j = 0; j < 2; j++) {
			motion_vertical_field_select[i][j] = inmvfs[i][j];
			for (k = 0; k < 2; k++) PMV[i][j][k] = inPMV[i][j][k];
		}
	}

	Initialize_Buffer();
	motion_vectors(s, motion_vector_count, mv_format, h_r_size, v_r_size, dmv, mvscale);

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 2; j++) {
			main_result += (motion_vertical_field_select[i][j] != outmvfs[i][j]);
			for (k = 0; k < 2; k++) main_result += (PMV[i][j][k] != outPMV[i][j][k]);
		}
	}

	printf("Main result : %d\n", main_result);

	return 0;
}

#endif



