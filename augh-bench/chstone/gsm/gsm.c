
#include <augh_annot.h>

#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)

	#include <augh.h>

	uint16_t stdin;
	uint16_t stdout;

	static inline uint16_t read16() {
		uint16_t u16;
		augh_read(stdin, &u16);
		return u16;
	}
	static inline void write16(uint16_t u16) {
		augh_write(stdout, &u16);
	}

#endif

#if defined(AUGH_SYNTHESIS) && defined(STDIO8)

	#include "../stdio8_defs.h"

	#define read16 read16_be
	#define write16 write16_be

#endif

#ifdef GCC_COMPIL

	#include <stdio.h>
	#include <stdint.h>

	// Read bytes left to right
	static inline int16_t fifo_read16_lr() {
		uint16_t u16 = 0;
		for(unsigned i=0; i<2; i++) { uint8_t u8 = getchar(); u16 = (u16 << 8) | u8; }
		return u16;
	}

	// Read bytes right to left
	static inline int16_t fifo_read16_rl() {
		uint16_t u16 = 0;
		for(unsigned i=0; i<2; i++) { uint8_t u8 = getchar(); u16 = ((uint16_t)u8 << 8) | (u16 >> 8); }
		return u16;
	}

	#define read16 fifo_read16_lr

	static inline void fifo_write16_lr(uint16_t u16) {
		for(unsigned i=0; i<2; i++) {
			uint8_t u8 = (u16 >> 8) & 0xFF;
			u16 = u16 << 8;
			putchar(u8);
		}
	}
	static inline void fifo_write16_rl(uint16_t u16) {
		for(unsigned i=0; i<2; i++) {
			uint8_t u8 = u16 & 0xFF;
			u16 = u16 >> 8;
			putchar(u8);
		}
	}

	#define write16 fifo_write16_lr

#endif



#define N 160
#define M 8

#define word      int16_t
#define longword  int32_t

//#define	MIN_WORD	((-32767)-1)
#define	MIN_WORD	(-32768)
#define	MAX_WORD	( 32767)

#define	SASR(x, by)	((x) >> (by))

#define GSM_MULT_R(a, b)	gsm_mult_r(a, b)
#define GSM_MULT(a, b)		gsm_mult(a, b)
#define GSM_ADD(a, b)		gsm_add(a, b)
#define GSM_ABS(a)		gsm_abs(a)

word s[N];
word LARc[M];


static inline word saturate(longword x) {
	word result;
	result = x < MIN_WORD ? MIN_WORD : x > MAX_WORD ? MAX_WORD: x;
	return result;
}

static inline word gsm_add (word a, word b) {
  longword sum = (longword) a + (longword) b;
  word result = saturate(sum);
  return result;
}

static inline word gsm_mult (word a, word b) {
	word result;
	longword temp;
	if (a <= MIN_WORD && b <= MIN_WORD) result = MAX_WORD;
	else {
		temp = a * b;
		result = SASR (temp, 15);
	}
  return result;
}

static inline word gsm_mult_r (word a, word b) {
  longword prod;
  if (b <= MIN_WORD && a <= MIN_WORD) prod = MAX_WORD;
  else {
		augh_branch_prob = 100;  // User annotation
		prod = a * b;
		prod += 16384;
		prod >>= 15;
		prod = prod & 0xFFFF;
	}
	return prod;
}

static inline word gsm_abs (word a) {
	word result;
	result = a < 0 ? (a <= MIN_WORD ? MAX_WORD : -a) : a;
  return result;
}

static inline word gsm_div (word num, word denum) {
  longword L_num;
  longword L_denum;
  word div;
  int k;

  L_num = num;
  L_denum = denum;
  div = 0;
  k = 15;
  /* The parameter num sometimes becomes zero.
   * Although this is explicitly guarded against in 4.2.5,
   * we assume that the result should then be zero as well.
   */

  if (num == 0) {
		augh_branch_prob_m = 125;  // User annotation
		div = 0;
	}
	else {
		while(k!=0) {
			augh_iter_nb_m = 13125;  // User annotation
			k--;
			div <<= 1;
			L_num <<= 1;
			if (L_num >= L_denum) {
				augh_branch_prob = 39;  // User annotation
				L_num -= L_denum;
				div++;
			}
		}
	}

  return div;
}

const unsigned char bitoff[256] = {
  8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * the number of left shifts needed to normalize the 32 bit
 * variable L_var1 for positive values on the interval
 *
 * with minimum of
 * minimum of 1073741824  (01000000000000000000000000000000) and
 * maximum of 2147483647  (01111111111111111111111111111111)
 *
 *
 * and for negative values on the interval with
 * minimum of -2147483648 (-10000000000000000000000000000000) and
 * maximum of -1073741824 ( -1000000000000000000000000000000).
 *
 * in order to normalize the result, the following
 * operation must be done: L_norm_var1 = L_var1 << norm( L_var1 );
 *
 * (That's 'ffs', only from the left, not the right..)
 */

static inline word gsm_norm (longword a) {
	word result;
	longword z;

  if (a <= -1073741824) {
		result = 0;
	}
  else {
		augh_branch_prob = 100;  // User annotation
		if (a < 0) a = ~a;

		result = a & 0xffff0000 ?
			(a & 0xff000000 ? -1 + bitoff[0xFF & (a >> 24)] : 7 + bitoff[0xFF & (a >> 16)])
			: (a & 0xff00 ? 15 + bitoff[0xFF & (a >> 8)] : 23 + bitoff[0xFF & a]);

	}

	return result;
}



/*
 *  4.2.4 .. 4.2.7 LPC ANALYSIS SECTION
 */

/* 4.2.4 */


/*

	word * s          [0..159]  IN/OUT
	longword * L_ACF  [0..8]    OUT

 *  The goal is to compute the array L_ACF[k].  The signal s[i] must
 *  be scaled in order to avoid an overflow situation.
 */

longword L_ACF[9];

void Autocorrelation () {
  register int k, i;

  word temp;
  word smax;
  word scalauto, n;
  word idx_sp;
  word sl;
	word z;

  /*  Search for the maximum.
   */
  smax = 0;
  for (k = 0; k <= 159; k++) {
		temp = GSM_ABS (s[k]);
		if (temp > smax) smax = temp;
	}

  /*  Computation of the scaling factor.
   */
  if (smax == 0) scalauto = 0;
  else {
		scalauto = 4 - gsm_norm ((longword) smax << 16);  /* sub(4,..) */
	}

  if (scalauto > 0 && scalauto <= 4) {
		augh_branch_prob = 100;  // User annotation
		n = scalauto;
		for (k = 0; k <= 159; k++) {
			s[k] = GSM_MULT_R (s[k], 16384 >> (n - 1));
		}
	}

  /*  Compute the L_ACF[..].
   */
  {
    idx_sp = 0;
    sl = s[idx_sp];

#define STEP(k)	 L_ACF[k] += ((longword)sl * s[ idx_sp - (k) ]);

#define NEXTI	 sl = s[++idx_sp]

    for (k = 8; k >= 0; k--) L_ACF[k] = 0;

    STEP (0);
    NEXTI;
    STEP (0);
    STEP (1);
    NEXTI;
    STEP (0);
    STEP (1);
    STEP (2);
    NEXTI;
    STEP (0);
    STEP (1);
    STEP (2);
    STEP (3);
    NEXTI;
    STEP (0);
    STEP (1);
    STEP (2);
    STEP (3);
    STEP (4);
    NEXTI;
    STEP (0);
    STEP (1);
    STEP (2);
    STEP (3);
    STEP (4);
    STEP (5);
    NEXTI;
    STEP (0);
    STEP (1);
    STEP (2);
    STEP (3);
    STEP (4);
    STEP (5);
    STEP (6);
    NEXTI;
    STEP (0);
    STEP (1);
    STEP (2);
    STEP (3);
    STEP (4);
    STEP (5);
    STEP (6);
    STEP (7);

    for (i = 8; i <= 159; i++) {

			NEXTI;

			STEP (0);
			STEP (1);
			STEP (2);
			STEP (3);
			STEP (4);
			STEP (5);
			STEP (6);
			STEP (7);
			STEP (8);
		}

    for (k = 8; k >= 0; k--) L_ACF[k] <<= 1;

  }
  /*   Rescaling of the array s[0..159]
   */
  if (scalauto > 0) {
		augh_branch_prob = 100;  // User annotation
    for (k = 159; k >= 0; k--) s[k] <<= scalauto;
	}
}

/* 4.2.5 */

//	longword * L_ACF /* 0...8        IN      */ ,
//	register word * r /* 0...7        OUT     */

void Reflection_coefficients () {
  register int i, m, n;
  register word temp;
  word ACF[9];			/* 0..8 */
  word P[9];			/* 0..8 */
  word K[9];			/* 2..8 */
  unsigned idx_r = 0;  // Index in the array LARc

  /*  Schur recursion with 16 bits arithmetic.
   */

  if (L_ACF[0] == 0) {
		augh_branch_prob = 0;  // User annotation
		for (i = 8; i > 0; i--) LARc[i] = 0;
		return;
	}

	temp = gsm_norm (L_ACF[0]);
	for (i = 0; i <= 8; i++) ACF[i] = SASR (L_ACF[i] << temp, 16);

	// Initialize array P[..] and K[..] for the recursion.

	for (i = 1; i <= 7; i++) K[i] = ACF[i];
	for (i = 0; i <= 8; i++) P[i] = ACF[i];

	// Compute reflection coefficients
	for (n = 1; n <= 8; n++, idx_r++) {
		augh_iter_nb = 8;  // User annotation

		temp = GSM_ABS (P[1]);

		if (P[0] < temp) {
			augh_branch_prob = 0;  // User annotation
			for (i = n; i <= 8; i++) LARc[idx_r++] = 0;
			return;
		}

		LARc[idx_r] = gsm_div (temp, P[0]);

		if (P[1] > 0) LARc[idx_r] = -LARc[idx_r];		/* r[n] = sub(0, r[n]) */
		if (n == 8) {
			augh_branch_prob_m = 125;  // User annotation
			return;
		}

		// Schur recursion
		temp = GSM_MULT_R (P[1], LARc[idx_r]);
		P[0] = GSM_ADD (P[0], temp);

		for (m = 1; m <= 8 - n; m++) {
			augh_iter_nb = 4;  // User annotation

			temp = GSM_MULT_R (K[m], LARc[idx_r]);
			P[m] = GSM_ADD (P[m + 1], temp);

			temp = GSM_MULT_R (P[m + 1], LARc[idx_r]);
			K[m] = GSM_ADD (K[m], temp);
		}
	}
}

/* 4.2.6 */

/*
 *  The following scaling for r[..] and LAR[..] has been used:
 *
 *  r[..]   = integer( real_r[..]*32768. ); -1 <= real_r < 1.
 *  LAR[..] = integer( real_LAR[..] * 16384 );
 *  with -1.625 <= real_LAR <= 1.625
 */
void Transformation_to_Log_Area_Ratios () {
  register word temp;
  register int i;
  unsigned idx_r = 0;  // Index in the array LARc

  /* Computation of the LAR[0..7] from the r[0..7]
   */
  for (i = 1; i <= 8; i++, idx_r++) {
		temp = GSM_ABS (LARc[idx_r]);

		if (temp < 22118) {
			temp >>= 1;
		}
		else {
			if (temp < 31130) {
				temp -= 11059;
			}
			else {
				temp -= 26112;
				temp <<= 2;
			}
		}

		LARc[idx_r] = LARc[idx_r] < 0 ? -temp : temp;
	}
}

/* 4.2.7 */

void Quantization_and_coding () {
  register word temp;
	unsigned idx_LAR = 0;  // Index in the array LARc

  /*  This procedure needs four tables; the following equations
   *  give the optimum scaling for the constants:
   *
   *  A[0..7] = integer( real_A[0..7] * 1024 )
   *  B[0..7] = integer( real_B[0..7] *  512 )
   *  MAC[0..7] = maximum of the LARc[0..7]
   *  MIC[0..7] = minimum of the LARc[0..7]
   */

#	undef STEP
#	define	STEP( A, B, MAC, MIC ) \
	temp = GSM_MULT(A, LARc[idx_LAR]); \
	temp = GSM_ADD(temp, B); \
	temp = GSM_ADD(temp, 256); \
	temp = SASR(temp, 9); \
	if(temp > (word)MAC) LARc[idx_LAR] = MAC - MIC; \
	else if(temp < (word)MIC) LARc[idx_LAR] = 0; else LARc[idx_LAR] = temp - (word)MIC; \
	idx_LAR++;

  STEP (20480, 0, 31, -32);
  STEP (20480, 0, 31, -32);
  STEP (20480, 2048, 15, -16);
  STEP (20480, -2560, 15, -16);

  STEP (13964, 94, 7, -8);
  STEP (15360, -1792, 7, -8);
  STEP (8534, -341, 3, -4);
  STEP (9036, -1144, 3, -4);

#	undef	STEP
}

void Gsm_LPC_Analysis () {
  Autocorrelation();
  Reflection_coefficients();
  Transformation_to_Log_Area_Ratios();
  Quantization_and_coding();
}



#ifdef AUGH_SYNTHESIS
void augh_main ()
#endif
#ifdef GCC_COMPIL
int main ()
#endif
{
	int i;

	#ifdef ASKSEND
	read8();
	#endif

	do {

		for (i = 0; i < N; i++) s[i] = read16();
		for (i = 0; i < 9; i++) L_ACF[i] = 0;
		for (i = 0; i < M; i++) LARc[i] = 0;

		Gsm_LPC_Analysis();

		for (i = 0; i < N; i++) write16(s[i]);
		for (i = 0; i < M; i++) write16(LARc[i]);

		#ifdef GCC_COMPIL
		break;
		#endif
	} while(1);

	#ifdef GCC_COMPIL
	return 0;
	#endif
}

