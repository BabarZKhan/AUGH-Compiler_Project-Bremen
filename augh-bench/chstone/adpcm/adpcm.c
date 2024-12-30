
#include <augh_annot.h>

#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)

	#include <augh.h>

	uint32_t stdin;
	uint32_t stdout;

	static inline uint32_t read32() {
		uint32_t buf;
		augh_read(stdin, &buf);
		return buf;
	}
	static inline void write32(uint32_t val) {
		augh_write(stdout, &val);
	}

#endif

#if defined(AUGH_SYNTHESIS) && defined(STDIO8)

	#include "../stdio8_defs.h"

	#define read32 read32_be
	#define write32 write32_be

#endif

#ifdef GCC_COMPIL

	#include <stdio.h>
	#include <stdint.h>
	#include <stdbool.h>

	static inline uint32_t read32_lr() {
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = getchar(); u32 = (u32 << 8) | u8; }
		return u32;
	}
	static inline uint32_t read32_rl() {
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = getchar(); u32 = (((uint32_t)u8) << 24) | (u32 >> 24); }
		return u32;
	}
	#define read32 read32_lr

	static inline void write32_lr(uint32_t u32) {
		for(unsigned i=0; i<4; i++) { uint8_t u8 = (u32 >> 24) & 0xFF; u32 = u32 << 8; putchar(u8); }
	}
	static inline void write32_rl(uint32_t u32) {
		for(unsigned i=0; i<4; i++) { uint8_t u8 = u32 & 0xFF; u32 = u32 >> 8; putchar(u8); }
	}
	#define write32 write32_lr

#endif



/* G722 C code */

/* filtez - compute predictor output signal (zero section) */
/* input: bpl1-6 and dlt1-6, output: szl */

// int *bpl, int *dlt
int filtez_i, filtez_bpl_idx, filtez_dlt_idx;
long filtez_zl;
int filtez_result;
#define filtez(bpl,dlt) do { \
	filtez_bpl_idx = 0; \
	filtez_dlt_idx = 0; \
	filtez_zl = bpl[filtez_bpl_idx++] * dlt[filtez_dlt_idx++]; \
	for (filtez_i = 1; filtez_i < 6; filtez_i++) { \
		filtez_zl += bpl[filtez_bpl_idx++] * dlt[filtez_dlt_idx++]; \
	} \
	filtez_result = filtez_zl >> 14;	/* x2 here */ \
} while(0)

/* upzero - inputs: dlt, dlti[0-5], bli[0-5], outputs: updated bli[0-5] */
/* also implements delay of bli and update of dlti from dlt */

// int dlt, int *dlti, int *bli
int upzero_i, upzero_wd2, upzero_wd3;
#define upzero(dlt,dlti,bli) do { \
	/*if dlt is zero, then no sum into bli */ \
	if (dlt == 0) { \
		for (upzero_i = 0; upzero_i < 6; upzero_i++) { \
			bli[upzero_i] = ((255 * bli[upzero_i]) >> 8);  /* leak factor of 255/256 */ \
		} \
	} \
	else { \
		for (upzero_i = 0; upzero_i < 6; upzero_i++) { \
			upzero_wd2 = dlt * dlti[upzero_i] >= 0 ? 128 : -128; \
			upzero_wd3 = ((255 * bli[upzero_i]) >> 8);  /* leak factor of 255/256 */ \
			bli[upzero_i] = upzero_wd2 + upzero_wd3; \
		} \
	} \
	/* implement delay line for dlt */ \
	dlti[5] = dlti[4]; \
	dlti[4] = dlti[3]; \
	dlti[3] = dlti[2]; \
	dlti[2] = dlti[1]; \
	dlti[1] = dlti[0]; \
	dlti[0] = dlt; \
} while(0)

/* variables for transimit quadrature mirror filter here */
int tqmf[24];

/* QMF filter coefficients:
scaled by a factor of 4 compared to G722 CCITT recomendation */
const int h[24] = {
	12, -44, -44, 212, 48, -624, 128, 1448,
	-840, -3220, 3804, 15504, 15504, 3804, -3220, -840,
	1448, 128, -624, 48, 212, -44, -44, 12
};

int xl, xh;

/* variables for receive quadrature mirror filter here */
int accumc[11], accumd[11];

/* outputs of decode() */
int xout1, xout2;

int xs, xd;

/* variables for encoder (hi and lo) here */

int il, szl, spl, sl, el;

const int qq4_code4_table[16] = {
	0, -20456, -12896, -8968, -6288, -4240, -2584, -1200,
	20456, 12896, 8968, 6288, 4240, 2584, 1200, 0
};


const int qq6_code6_table[64] = {
	-136, -136, -136, -136, -24808, -21904, -19008, -16704,
	-14984, -13512, -12280, -11192, -10232, -9360, -8576, -7856,
	-7192, -6576, -6000, -5456, -4944, -4464, -4008, -3576,
	-3168, -2776, -2400, -2032, -1688, -1360, -1040, -728,
	24808, 21904, 19008, 16704, 14984, 13512, 12280, 11192,
	10232, 9360, 8576, 7856, 7192, 6576, 6000, 5456,
	4944, 4464, 4008, 3576, 3168, 2776, 2400, 2032,
	1688, 1360, 1040, 728, 432, 136, -432, -136
};

int delay_bpl[6];

int delay_dltx[6];

const int wl_code_table[16] = {
	-60, 3042, 1198, 538, 334, 172, 58, -30,
	3042, 1198, 538, 334, 172, 58, -30, -60
};

const int ilb_table[32] = {
	2048, 2093, 2139, 2186, 2233, 2282, 2332, 2383,
	2435, 2489, 2543, 2599, 2656, 2714, 2774, 2834,
	2896, 2960, 3025, 3091, 3158, 3228, 3298, 3371,
	3444, 3520, 3597, 3676, 3756, 3838, 3922, 4008
};

int nbl;			/* delay line */
int al1, al2;
int plt, plt1, plt2;
int dlt;
int rlt, rlt1, rlt2;

/* decision levels - pre-multiplied by 8, 0 to indicate end */
const int decis_levl[30] = {
	280, 576, 880, 1200, 1520, 1864, 2208, 2584,
	2960, 3376, 3784, 4240, 4696, 5200, 5712, 6288,
	6864, 7520, 8184, 8968, 9752, 10712, 11664, 12896,
	14120, 15840, 17560, 20456, 23352, 32767
};

int detl;

/* quantization table 31 long to make quantl look-up easier,
last entry is for mil=30 case when wd is max */
const int quant26bt_pos[31] = {
	61, 60, 59, 58, 57, 56, 55, 54,
	53, 52, 51, 50, 49, 48, 47, 46,
	45, 44, 43, 42, 41, 40, 39, 38,
	37, 36, 35, 34, 33, 32, 32
};

/* quantization table 31 long to make quantl look-up easier,
last entry is for mil=30 case when wd is max */
const int quant26bt_neg[31] = {
	63, 62, 31, 30, 29, 28, 27, 26,
	25, 24, 23, 22, 21, 20, 19, 18,
	17, 16, 15, 14, 13, 12, 11, 10,
	9, 8, 7, 6, 5, 4, 4
};


int deth;
int sh;				/* this comes from adaptive predictor */
int eh;

const int qq2_code2_table[4] = {
	-7408, -1616, 7408, 1616
};

const int wh_code_table[4] = {
	798, -214, 798, -214
};


int dh, ih;
int nbh, szh;
int sph, ph, yh, rh;

int delay_dhx[6];

int delay_bph[6];

int ah1, ah2;
int ph1, ph2;
int rh1, rh2;

/* variables for decoder here */
int ilr, rl;
int dec_deth, dec_detl, dec_dlt;

int dec_del_bpl[6];

int dec_del_dltx[6];

int dec_plt, dec_plt1, dec_plt2;
int dec_szl, dec_spl, dec_sl;
int dec_rlt1, dec_rlt2, dec_rlt;
int dec_al1, dec_al2;
int dl;
int dec_nbl, dec_dh, dec_nbh;

/* variables used in filtez */
int dec_del_bph[6];

int dec_del_dhx[6];

int dec_szh;
/* variables used in filtep */
int dec_rh1, dec_rh2;
int dec_ah1, dec_ah2;
int dec_ph, dec_sph;

int dec_sh;

int dec_ph1, dec_ph2;

int abs_buffer;
static inline int abs (int n) {
	int m;

	if (n >= 0) m = n;
	else m = -n;
	return m;
}

/* clear all storage locations */

static inline void reset () {
	int i;

	detl = dec_detl = 32;		/* reset to min scale factor */
	deth = dec_deth = 8;
	nbl = al1 = al2 = plt1 = plt2 = rlt1 = rlt2 = 0;
	nbh = ah1 = ah2 = ph1 = ph2 = rh1 = rh2 = 0;
	dec_nbl = dec_al1 = dec_al2 = dec_plt1 = dec_plt2 = dec_rlt1 = dec_rlt2 = 0;
	dec_nbh = dec_ah1 = dec_ah2 = dec_ph1 = dec_ph2 = dec_rh1 = dec_rh2 = 0;

	for (i = 0; i < 6; i++) {
		delay_dltx[i] = 0;
		delay_dhx[i] = 0;
		dec_del_dltx[i] = 0;
		dec_del_dhx[i] = 0;
	}

	for (i = 0; i < 6; i++) {
		delay_bpl[i] = 0;
		delay_bph[i] = 0;
		dec_del_bpl[i] = 0;
		dec_del_bph[i] = 0;
	}

	for (i = 0; i < 24; i++) tqmf[i] = 0;

	for (i = 0; i < 11; i++) {
		accumc[i] = 0;
		accumd[i] = 0;
	}
}

/* filtep - compute predictor output signal (pole section) */
/* input rlt1-2 and al1-2, output spl */

static inline int filtep (int rlt1, int al1, int rlt2, int al2) {
	long int pl, pl2;
	pl = 2 * rlt1;
	pl = (long) al1 * pl;
	pl2 = 2 * rlt2;
	pl += (long) al2 * pl2;
	return ((int) (pl >> 15));
}

/* quantl - quantize the difference signal in the lower sub-band */
static inline int quantl (int el, int detl) {
	int ril, mil;
	long int wd, decis;

	/* abs of difference signal */
	wd = abs (el);
	/* determine mil based on decision levels and detl gain */
	for (mil = 0; mil < 30; mil++) {
		augh_iter_nb_m = 25260;  // User annotation
		decis = (decis_levl[mil] * detl) >> 15L;
		if (wd <= decis) {
			augh_branch_prob_m = 38;  // User annotation
			break;
		}
	}
	/* if mil=30 then wd is less than all decision levels */
	if (el >= 0) ril = quant26bt_pos[mil];
	else         ril = quant26bt_neg[mil];
	return (ril);
}

/* logscl - update log quantizer scale factor in lower sub-band */
/* note that nbl is passed and returned */

static inline int logscl (int il, int nbl) {
	long int wd;
	wd = (nbl * 127) >> 7;  /* leak factor 127/128 */
	nbl = wd + wl_code_table[il >> 2];
	if (nbl < 0) nbl = 0;
	if (nbl > 18432) nbl = 18432;
	return nbl;
}

/* scalel: compute quantizer scale factor in lower or upper sub-band*/

static inline int scalel (int nbl, int shift_constant) {
	int wd1, wd2, wd3;
	wd1 = (nbl >> 6) & 31;
	wd2 = nbl >> 11;
	wd3 = ilb_table[wd1] >> (shift_constant + 1 - wd2);
	return wd3 << 3;
}

/* uppol2 - update second predictor coefficient (pole section) */
/* inputs: al1, al2, plt, plt1, plt2. outputs: apl2 */

static inline int uppol2 (int al1, int al2, int plt, int plt1, int plt2) {
	long int wd2, wd4;
	int apl2;

	wd2 = 4 * al1;
	if (plt * plt1 >= 0) wd2 = -wd2;			/* check same sign */
	wd2 >>= 7;  /* gain of 1/128 */
	wd4 = plt * plt2 >= 0 ? 128 : -128;
	apl2 = wd4 + (127 * (long) al2 >> 7);	/* leak factor of 127/128 */

	/* apl2 is limited to +-.75 */
	if (apl2 > 12288) apl2 = 12288;
	if (apl2 < -12288) apl2 = -12288;
	return (apl2);
}

/* uppol1 - update first predictor coefficient (pole section) */
/* inputs: al1, apl2, plt, plt1. outputs: apl1 */

static inline int uppol1 (int al1, int apl2, int plt, int plt1) {
	long int wd2;
	int wd3, apl1;

	wd2 = (al1 * 255) >> 8;  /* leak factor of 255/256 */
	apl1 = plt * plt1 >= 0 ? wd2 + 192 : wd2 - 192;

	/* note: wd3= .9375-.75 is always positive */
	wd3 = 15360 - apl2;		/* limit value */
	if (apl1 > wd3) apl1 = wd3;
	if (apl1 < -wd3) apl1 = -wd3;
	return apl1;
}

/* logsch - update log quantizer scale factor in higher sub-band */
/* note that nbh is passed and returned */

static inline int logsch (int ih, int nbh) {
	int wd;
	wd = (nbh * 127) >> 7;  /* leak factor 127/128 */
	nbh = wd + wh_code_table[ih];
	if (nbh < 0) nbh = 0;
	if (nbh > 22528) nbh = 22528;
	return nbh;
}

/* G722 encode function two ints in, one 8 bit output */

/* put input samples in xin1 = first value, xin2 = second value */
/* returns il and ih stored together */

static inline int encode (int xin1, int xin2) {
	int i;
	unsigned h_idx;  // Index in h
	unsigned tqmf_idx, tqmf_idx1;
	long int xa, xb;
	int decis;

/* transmit quadrature mirror filters implemented here */
	h_idx = 0;
	tqmf_idx = 0;
	xa = tqmf[tqmf_idx++] * h[h_idx++];
	xb = tqmf[tqmf_idx++] * h[h_idx++];
/* main multiply accumulate loop for samples and coefficients */
	for (i = 0; i < 10; i++) {
		xa += tqmf[tqmf_idx++] * h[h_idx++];
		xb += tqmf[tqmf_idx++] * h[h_idx++];
	}
/* final mult/accumulate */
	xa += tqmf[tqmf_idx++] * h[h_idx++];
	xb += tqmf[tqmf_idx] * h[h_idx++];

/* update delay line tqmf */
	tqmf_idx1 = tqmf_idx - 2;
	for (i = 0; i < 22; i++) tqmf[tqmf_idx--] = tqmf[tqmf_idx1--];
	tqmf[tqmf_idx--] = xin1;
	tqmf[tqmf_idx] = xin2;

/* scale outputs */
	xl = (xa + xb) >> 15;
	xh = (xa - xb) >> 15;

/* end of quadrature mirror filter code */

/* starting with lower sub band encoder */

/* filtez - compute predictor output section - zero section */
	filtez (delay_bpl, delay_dltx);
	szl = filtez_result;

/* filtep - compute predictor output signal (pole section) */
	spl = filtep (rlt1, al1, rlt2, al2);

/* compute the predictor output value in the lower sub_band encoder */
	sl = szl + spl;
	el = xl - sl;

/* quantl: quantize the difference signal */
	il = quantl (el, detl);

/* computes quantized difference signal */
/* for invqbl, truncate by 2 lsbs, so mode = 3 */
	dlt = (detl * qq4_code4_table[il >> 2]) >> 15;

/* logscl: updates logarithmic quant. scale factor in low sub band */
	nbl = logscl (il, nbl);

/* scalel: compute the quantizer scale factor in the lower sub band */
/* calling parameters nbl and 8 (constant such that scalel can be scaleh) */
	detl = scalel (nbl, 8);

/* parrec - simple addition to compute recontructed signal for adaptive pred */
	plt = dlt + szl;

/* upzero: update zero section predictor coefficients (sixth order)*/
/* calling parameters: dlt, dlt1, dlt2, ..., dlt6 from dlt */
/*  bpli (linear_buffer in which all six values are delayed */
/* return params:      updated bpli, delayed dltx */
	upzero (dlt, delay_dltx, delay_bpl);

/* uppol2- update second predictor coefficient apl2 and delay it as al2 */
/* calling parameters: al1, al2, plt, plt1, plt2 */
	al2 = uppol2 (al1, al2, plt, plt1, plt2);

/* uppol1 :update first predictor coefficient apl1 and delay it as al1 */
/* calling parameters: al1, apl2, plt, plt1 */
	al1 = uppol1 (al1, al2, plt, plt1);

/* recons : compute recontructed signal for adaptive predictor */
	rlt = sl + dlt;

/* done with lower sub_band encoder; now implement delays for next time*/
	rlt2 = rlt1;
	rlt1 = rlt;
	plt2 = plt1;
	plt1 = plt;

/* high band encode */

	filtez (delay_bph, delay_dhx);
	szh = filtez_result;

	sph = filtep (rh1, ah1, rh2, ah2);

/* predic: sh = sph + szh */
	sh = sph + szh;
/* subtra: eh = xh - sh */
	eh = xh - sh;

/* quanth - quantization of difference signal for higher sub-band */
/* quanth: in-place for speed params: eh, deth (has init. value) */
	if (eh >= 0) {
		ih = 3;			/* 2,3 are pos codes */
	}
	else {
		ih = 1;			/* 0,1 are neg codes */
	}
	decis = (564 * deth) >> 12;
	abs_buffer = abs (eh);
	if (abs_buffer > decis) ih--;			/* mih = 2 case */

/* compute the quantized difference signal, higher sub-band*/
	dh = (deth * qq2_code2_table[ih]) >> 15;

/* logsch: update logarithmic quantizer scale factor in hi sub-band*/
	nbh = logsch (ih, nbh);

/* note : scalel and scaleh use same code, different parameters */
	deth = scalel (nbh, 10);

/* parrec - add pole predictor output to quantized diff. signal */
	ph = dh + szh;

/* upzero: update zero section predictor coefficients (sixth order) */
/* calling parameters: dh, dhi, bphi */
/* return params: updated bphi, delayed dhx */
	upzero (dh, delay_dhx, delay_bph);

/* uppol2: update second predictor coef aph2 and delay as ah2 */
/* calling params: ah1, ah2, ph, ph1, ph2 */
	ah2 = uppol2 (ah1, ah2, ph, ph1, ph2);

/* uppol1:  update first predictor coef. aph2 and delay it as ah1 */
	ah1 = uppol1 (ah1, ah2, ph, ph1);

/* recons for higher sub-band */
	yh = sh + dh;

/* done with higher sub-band encoder, now Delay for next time */
	rh2 = rh1;
	rh1 = yh;
	ph2 = ph1;
	ph1 = ph;

/* multiplex ih and il to get signals together */
	return (il | (ih << 6));
}

/* decode function, result in xout1 and xout2 */

static inline void decode (int input) {
	int i;
	long int xa1, xa2;		/* qmf accumulators */
	unsigned h_idx;
	unsigned ac_idx, ac_idx1, ad_idx, ad_idx1;
	long lgbuf;

/* split transmitted word from input into ilr and ih */
	ilr = input & 0x3f;
	ih = input >> 6;

/* LOWER SUB_BAND DECODER */

/* filtez: compute predictor output for zero section */
	filtez (dec_del_bpl, dec_del_dltx);
	dec_szl = filtez_result;

/* filtep: compute predictor output signal for pole section */
	dec_spl = filtep (dec_rlt1, dec_al1, dec_rlt2, dec_al2);

	dec_sl = dec_spl + dec_szl;

/* compute quantized difference signal for adaptive predic */
	dec_dlt = (dec_detl * qq4_code4_table[ilr >> 2]) >> 15;

/* compute quantized difference signal for decoder output */
	dl = (dec_detl * qq6_code6_table[il]) >> 15;

	rl = dl + dec_sl;

/* logscl: quantizer scale factor adaptation in the lower sub-band */
	dec_nbl = logscl (ilr, dec_nbl);

/* scalel: computes quantizer scale factor in the lower sub band */
	dec_detl = scalel (dec_nbl, 8);

/* parrec - add pole predictor output to quantized diff. signal */
/* for partially reconstructed signal */
	dec_plt = dec_dlt + dec_szl;

/* upzero: update zero section predictor coefficients */
	upzero (dec_dlt, dec_del_dltx, dec_del_bpl);

/* uppol2: update second predictor coefficient apl2 and delay it as al2 */
	dec_al2 = uppol2 (dec_al1, dec_al2, dec_plt, dec_plt1, dec_plt2);

/* uppol1: update first predictor coef. (pole setion) */
	dec_al1 = uppol1 (dec_al1, dec_al2, dec_plt, dec_plt1);

/* recons : compute recontructed signal for adaptive predictor */
	dec_rlt = dec_sl + dec_dlt;

/* done with lower sub band decoder, implement delays for next time */
	dec_rlt2 = dec_rlt1;
	dec_rlt1 = dec_rlt;
	dec_plt2 = dec_plt1;
	dec_plt1 = dec_plt;

/* HIGH SUB-BAND DECODER */

/* filtez: compute predictor output for zero section */
	filtez (dec_del_bph, dec_del_dhx);
	dec_szh = filtez_result;

/* filtep: compute predictor output signal for pole section */
	dec_sph = filtep (dec_rh1, dec_ah1, dec_rh2, dec_ah2);

/* predic:compute the predictor output value in the higher sub_band decoder */
	dec_sh = dec_sph + dec_szh;

/* in-place compute the quantized difference signal */
	dec_dh = (dec_deth * qq2_code2_table[ih]) >> 15L;

/* logsch: update logarithmic quantizer scale factor in hi sub band */
	dec_nbh = logsch (ih, dec_nbh);

/* scalel: compute the quantizer scale factor in the higher sub band */
	dec_deth = scalel (dec_nbh, 10);

/* parrec: compute partially recontructed signal */
	dec_ph = dec_dh + dec_szh;

/* upzero: update zero section predictor coefficients */
	upzero (dec_dh, dec_del_dhx, dec_del_bph);

/* uppol2: update second predictor coefficient aph2 and delay it as ah2 */
	dec_ah2 = uppol2 (dec_ah1, dec_ah2, dec_ph, dec_ph1, dec_ph2);

/* uppol1: update first predictor coef. (pole setion) */
	dec_ah1 = uppol1 (dec_ah1, dec_ah2, dec_ph, dec_ph1);

/* recons : compute recontructed signal for adaptive predictor */
	rh = dec_sh + dec_dh;

/* done with high band decode, implementing delays for next time here */
	dec_rh2 = dec_rh1;
	dec_rh1 = rh;
	dec_ph2 = dec_ph1;
	dec_ph1 = dec_ph;

/* end of higher sub_band decoder */

/* end with receive quadrature mirror filters */
	xd = rl - rh;
	xs = rl + rh;

/* receive quadrature mirror filters implemented here */
	h_idx = 0;
	ac_idx = 0;
	ad_idx = 0;
	xa1 = xd * h[h_idx++];
	xa2 = xs * h[h_idx++];
/* main multiply accumulate loop for samples and coefficients */
	for (i = 0; i < 10; i++) {
		xa1 += accumc[ac_idx++] * h[h_idx++];
		xa2 += accumd[ad_idx++] * h[h_idx++];
	}
/* final mult/accumulate */
	xa1 += accumc[ac_idx] * h[h_idx++];
	xa2 += accumd[ad_idx] * h[h_idx++];

/* scale by 2^14 */
	xout1 = xa1 >> 14;
	xout2 = xa2 >> 14;

/* update delay lines */
	ac_idx1 = ac_idx - 1;
	ad_idx1 = ad_idx - 1;
	for (i = 0; i < 10; i++) {
		accumc[ac_idx--] = accumc[ac_idx1--];
		accumd[ad_idx--] = accumd[ad_idx1--];
	}
	accumc[ac_idx] = xd;
	accumd[ad_idx] = xs;
}



#define SIZE 100
#define IN_END 100

int test_data[SIZE];
int compressed[SIZE];
int result[SIZE];



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

		reset ();

		for (i = 0; i < SIZE; i++) test_data[i] = read32();

		for (i = 0; i < IN_END; i += 2) {
			compressed[i / 2] = encode (test_data[i], test_data[i + 1]);
		}

		for (i = 0; i < IN_END; i += 2) write32(compressed[i / 2]);

		for (i = 0; i < IN_END; i += 2) {
			decode (compressed[i / 2]);
			result[i] = xout1;
			result[i + 1] = xout2;
		}

		for (i = 0; i < IN_END; i++) write32(result[i]);

		#ifdef GCC_COMPIL
		break;
		#endif
	} while(1);

	#ifdef GCC_COMPIL
	return 0;
	#endif
}


