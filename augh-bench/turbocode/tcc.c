
#include <augh_annot.h>

#ifdef AUGH_SYNTHESIS

	#include <augh.h>

	uint8_t stdin;
	uint16_t stdout;

	static inline uint8_t read8() { uint8_t u8; augh_read(stdin, &u8); return u8; }
	static inline void write16(uint16_t u16) { augh_write(stdout, &u16); }

#endif

#ifdef GCC_COMPIL  // For compilation with gcc
	#include <stdio.h>
	#include <stdint.h>
	#include <stdbool.h>

	static inline void write8(uint8_t u8) {
		putchar(u8);
	}
	static inline uint8_t read8() {
		uint8_t u8 = getchar();
		return u8;
	}

	static inline void write16_be(uint16_t u16) {
		uint8_t i;
		uint8_t u8;
		for(i=0; i<2; i++) { u8 = (u16 >> 8) & 0xFF; u16 = u16 << 8; write8(u8); }
	}
	static inline void write16_le(uint16_t u16) {
		uint8_t i;
		uint8_t u8;
		for(i=0; i<2; i++) { u8 = u16 & 0xFF; u16 = u16 >> 8; write8(u8); }
	}

	#define write16 write16_be

#endif



#define H_TREILLIS  8
#define PMAX        4

#define N            752
#define NB_ITERATION 5
#define L_PROLOGUE   16

//typedef enum {forward, backward} way_t;
#define forward 0
#define backward 1
#define way_t int

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MAX_JAC(a,b) (MAX(a,b))

typedef struct{
	int8_t xa;    // systematic information bit 0
	int8_t xb;
	int8_t y1_0;  // first redundancy for decoder 0
	int8_t y1_1;  // first redundancy for decoder 1
	int8_t y2_0;
	int8_t y2_1;
} decoder_input;

//typedef enum {none, flipz, flipall} t_flip;
#define none 0
#define flipz 1
#define flipall 2
#define t_flip int

// Utility function to compute the remainder in integer division
uint16_t augh_div16r (uint16_t a, uint16_t b) {
	uint16_t cur_a = 0;
	unsigned i;

	for(i=0; i<16; i++) {
		cur_a = (cur_a << 1) | (a >> 15);
		a <<= 1;
		if(cur_a >= b) cur_a -= b;
	}

	return cur_a;
}

const int8_t preced_forward[32]  = {0,2,5,7,1,3,4,6,6,4,3,1,7,5,2,0,1,3,4,6,0,2,5,7,7,5,2,0,6,4,3,1}; // uses 3 bits
const int8_t preced_backward[32] = {0,4,1,5,6,2,7,3,7,3,6,2,1,5,0,4,4,0,5,1,2,6,3,7,3,7,2,6,5,1,4,0}; // uses 3 bits
const int8_t red1_forward[32]    = {0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,1,1,1,1,0,0}; // uses 1 bit
const int8_t red2_forward[32]    = {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1}; // uses 1 bit
const int8_t red1_backward[32]   = {0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0}; // uses 1 bit
const int8_t red2_backward[32]   = {0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0}; // uses 1 bit

int32_t k_etat;

// void CALC_MET_NOEUD (decoder_input input, uint8_t dec, way_t sens, int16_t gamma_intr[4], int16_t met_ext[4*H_TREILLIS], int16_t met_max_noeud[H_TREILLIS])
int16_t met_cum_noeud[4*H_TREILLIS];
int8_t met_trans_y1[2], met_trans_y2[2];
// Transition metrics
uint8_t etat_prec, k_trans;
int8_t red1, red2;
int16_t maxi;
int16_t tmp1, tmp2;
#define CALC_MET_NOEUD(input, dec, sens, gamma_intr, met_ext, met_max_noeud) do { \
	maxi = 0; \
	met_trans_y1[0] = dec==1 ? -input.y1_1 : -input.y1_0; \
	met_trans_y1[1] = dec==1 ? input.y1_1 : input.y1_0; \
	met_trans_y2[0] = dec==1 ? -input.y2_1 : -input.y2_0; \
	met_trans_y2[1] = dec==1 ? input.y2_1 : input.y2_0; \
	for (k_etat=0; k_etat < H_TREILLIS; k_etat++) { \
		for (k_trans=0; k_trans <= 3; k_trans++) { \
			etat_prec = sens==forward ? preced_forward[H_TREILLIS*k_trans+k_etat] : preced_backward[H_TREILLIS*k_trans+k_etat]; \
			red1 = sens==forward ? red1_forward[H_TREILLIS*k_trans+etat_prec] : red1_backward[H_TREILLIS*k_trans+etat_prec]; \
			red2 = sens==forward ? red2_forward[H_TREILLIS*k_trans+etat_prec] : red2_backward[H_TREILLIS*k_trans+etat_prec]; \
			met_ext[k_trans*H_TREILLIS+k_etat] = met_max_noeud[etat_prec] + met_trans_y1[red1] + met_trans_y2[red2]; \
			met_cum_noeud[k_trans*H_TREILLIS+k_etat] = met_ext[k_trans*H_TREILLIS+k_etat] + gamma_intr[k_trans]; \
		} \
	} \
	for (k_etat=0; k_etat < H_TREILLIS; k_etat++) { \
		tmp1 = MAX_JAC(met_cum_noeud[k_etat], met_cum_noeud[8+k_etat]); \
		tmp2 = MAX_JAC(met_cum_noeud[16+k_etat], met_cum_noeud[24+k_etat]); \
		met_max_noeud[k_etat] = MAX_JAC(tmp1, tmp2); \
		if (met_max_noeud[k_etat]>maxi) maxi = met_max_noeud[k_etat]; \
	} \
	for (k_etat=0; k_etat < H_TREILLIS; k_etat++) met_max_noeud[k_etat] -= maxi; \
} while(0)

// void CALC_INFO_EXT(int16_t met_ext_forward[4*H_TREILLIS], int16_t mem_met_backward[H_TREILLIS], int16_t Z_out_f[4])
int32_t k_etat, Z_max[H_TREILLIS/2];
#define CALC_INFO_EXT(met_ext_forward, mem_met_backward, Z_out_f) do { \
	for (i=0; i<=3; i++) { \
		/* Max for i on all states */ \
		for (k_etat=0; k_etat<H_TREILLIS/2; k_etat++) { \
			Z_max[k_etat] = MAX_JAC( \
				met_ext_forward[i*8+k_etat]   + mem_met_backward[k_etat], \
				met_ext_forward[i*8+k_etat+4] + mem_met_backward[k_etat+4] \
			); \
		} \
		Z_max[0] = MAX_JAC(Z_max[0], Z_max[1]); \
		Z_max[1] = MAX_JAC(Z_max[2], Z_max[3]); \
		Z_out_f[i] = MAX_JAC(Z_max[0], Z_max[1]); \
	} \
} while(0)

// void CALC_DECISION(int16_t gamma_intr[4], int16_t Z[4], int16_t dec[2])
int32_t i;
int32_t somme[4];
#define CALC_DECISION(gamma_intr, Z, dec) do { \
	/* Compute information on decision i */ \
	for (i=0; i<=3; i++) somme[i] = Z[i] + gamma_intr[i]; \
	/* Elaboration of weighted decision about A */ \
	dec[0] = MAX(somme[2], somme[3]) - MAX(somme[0], somme[1]); \
	/* Elaboration of weighted decision about B */ \
	dec[1] = MAX(somme[1], somme[3]) - MAX(somme[0], somme[2]); \
} while(0)

const int32_t mat_ent[N] = {1,20,263,282,77,96,339,358,153,172,415,434,229,248,491,510,305,324,567,586,381,400,643,662,457,476,719,738,533,552,43,62,609,628,119,138,685,704,195,214,9,28,271,290,85,104,347,366,161,180,423,442,237,256,499,518,313,332,575,594,389,408,651,670,465,484,727,746,541,560,51,70,617,636,127,146,693,712,203,222,17,36,279,298,93,112,355,374,169,188,431,450,245,264,507,526,321,340,583,602,397,416,659,678,473,492,735,2,549,568,59,78,625,644,135,154,701,720,211,230,25,44,287,306,101,120,363,382,177,196,439,458,253,272,515,534,329,348,591,610,405,424,667,686,481,500,743,10,557,576,67,86,633,652,143,162,709,728,219,238,33,52,295,314,109,128,371,390,185,204,447,466,261,280,523,542,337,356,599,618,413,432,675,694,489,508,751,18,565,584,75,94,641,660,151,170,717,736,227,246,41,60,303,322,117,136,379,398,193,212,455,474,269,288,531,550,345,364,607,626,421,440,683,702,497,516,7,26,573,592,83,102,649,668,159,178,725,744,235,254,49,68,311,330,125,144,387,406,201,220,463,482,277,296,539,558,353,372,615,634,429,448,691,710,505,524,15,34,581,600,91,110,657,676,167,186,733,0,243,262,57,76,319,338,133,152,395,414,209,228,471,490,285,304,547,566,361,380,623,642,437,456,699,718,513,532,23,42,589,608,99,118,665,684,175,194,741,8,251,270,65,84,327,346,141,160,403,422,217,236,479,498,293,312,555,574,369,388,631,650,445,464,707,726,521,540,31,50,597,616,107,126,673,692,183,202,749,16,259,278,73,92,335,354,149,168,411,430,225,244,487,506,301,320,563,582,377,396,639,658,453,472,715,734,529,548,39,58,605,624,115,134,681,700,191,210,5,24,267,286,81,100,343,362,157,176,419,438,233,252,495,514,309,328,571,590,385,404,647,666,461,480,723,742,537,556,47,66,613,632,123,142,689,708,199,218,13,32,275,294,89,108,351,370,165,184,427,446,241,260,503,522,317,336,579,598,393,412,655,674,469,488,731,750,545,564,55,74,621,640,131,150,697,716,207,226,21,40,283,302,97,116,359,378,173,192,435,454,249,268,511,530,325,344,587,606,401,420,663,682,477,496,739,6,553,572,63,82,629,648,139,158,705,724,215,234,29,48,291,310,105,124,367,386,181,200,443,462,257,276,519,538,333,352,595,614,409,428,671,690,485,504,747,14,561,580,71,90,637,656,147,166,713,732,223,242,37,56,299,318,113,132,375,394,189,208,451,470,265,284,527,546,341,360,603,622,417,436,679,698,493,512,3,22,569,588,79,98,645,664,155,174,721,740,231,250,45,64,307,326,121,140,383,402,197,216,459,478,273,292,535,554,349,368,611,630,425,444,687,706,501,520,11,30,577,596,87,106,653,672,163,182,729,748,239,258,53,72,315,334,129,148,391,410,205,224,467,486,281,300,543,562,357,376,619,638,433,452,695,714,509,528,19,38,585,604,95,114,661,680,171,190,737,4,247,266,61,80,323,342,137,156,399,418,213,232,475,494,289,308,551,570,365,384,627,646,441,460,703,722,517,536,27,46,593,612,103,122,669,688,179,198,745,12,255,274,69,88,331,350,145,164,407,426,221,240,483,502,297,316,559,578,373,392,635,654,449,468,711,730,525,544,35,54,601,620,111,130,677,696,187,206};
const int32_t mat_des[N] = {277,0,107,582,673,396,503,226,317,40,147,622,713,436,543,266,357,80,187,662,1,476,583,306,397,120,227,702,41,516,623,346,437,160,267,742,81,556,663,386,477,200,307,30,121,596,703,426,517,240,347,70,161,636,743,466,557,280,387,110,201,676,31,506,597,320,427,150,241,716,71,546,637,360,467,190,281,4,111,586,677,400,507,230,321,44,151,626,717,440,547,270,361,84,191,666,5,480,587,310,401,124,231,706,45,520,627,350,441,164,271,746,85,560,667,390,481,204,311,34,125,600,707,430,521,244,351,74,165,640,747,470,561,284,391,114,205,680,35,510,601,324,431,154,245,720,75,550,641,364,471,194,285,8,115,590,681,404,511,234,325,48,155,630,721,444,551,274,365,88,195,670,9,484,591,314,405,128,235,710,49,524,631,354,445,168,275,750,89,564,671,394,485,208,315,38,129,604,711,434,525,248,355,78,169,644,751,474,565,288,395,118,209,684,39,514,605,328,435,158,249,724,79,554,645,368,475,198,289,12,119,594,685,408,515,238,329,52,159,634,725,448,555,278,369,92,199,674,13,488,595,318,409,132,239,714,53,528,635,358,449,172,279,2,93,568,675,398,489,212,319,42,133,608,715,438,529,252,359,82,173,648,3,478,569,292,399,122,213,688,43,518,609,332,439,162,253,728,83,558,649,372,479,202,293,16,123,598,689,412,519,242,333,56,163,638,729,452,559,282,373,96,203,678,17,492,599,322,413,136,243,718,57,532,639,362,453,176,283,6,97,572,679,402,493,216,323,46,137,612,719,442,533,256,363,86,177,652,7,482,573,296,403,126,217,692,47,522,613,336,443,166,257,732,87,562,653,376,483,206,297,20,127,602,693,416,523,246,337,60,167,642,733,456,563,286,377,100,207,682,21,496,603,326,417,140,247,722,61,536,643,366,457,180,287,10,101,576,683,406,497,220,327,50,141,616,723,446,537,260,367,90,181,656,11,486,577,300,407,130,221,696,51,526,617,340,447,170,261,736,91,566,657,380,487,210,301,24,131,606,697,420,527,250,341,64,171,646,737,460,567,290,381,104,211,686,25,500,607,330,421,144,251,726,65,540,647,370,461,184,291,14,105,580,687,410,501,224,331,54,145,620,727,450,541,264,371,94,185,660,15,490,581,304,411,134,225,700,55,530,621,344,451,174,265,740,95,570,661,384,491,214,305,28,135,610,701,424,531,254,345,68,175,650,741,464,571,294,385,108,215,690,29,504,611,334,425,148,255,730,69,544,651,374,465,188,295,18,109,584,691,414,505,228,335,58,149,624,731,454,545,268,375,98,189,664,19,494,585,308,415,138,229,704,59,534,625,348,455,178,269,744,99,574,665,388,495,218,309,32,139,614,705,428,535,258,349,72,179,654,745,468,575,298,389,112,219,694,33,508,615,338,429,152,259,734,73,548,655,378,469,192,299,22,113,588,695,418,509,232,339,62,153,628,735,458,549,272,379,102,193,668,23,498,589,312,419,142,233,708,63,538,629,352,459,182,273,748,103,578,669,392,499,222,313,36,143,618,709,432,539,262,353,76,183,658,749,472,579,302,393,116,223,698,37,512,619,342,433,156,263,738,77,552,659,382,473,196,303,26,117,592,699,422,513,236,343,66,157,632,739,462,553,276,383,106,197,672,27,502,593,316,423,146,237,712,67,542,633,356,463,186};

static inline int32_t SF(int32_t valeur){
	return (3*valeur)>>2;
}

// void interleaver(int16_t gamma_intr[4], decoder_input input, int16_t z_in[4], t_flip flip)
int32_t A, B, Z[4];
#define interleaver(gamma_intr, input, z_in, flip) do { \
	Z[0] = SF(z_in[0]); \
	Z[3] = SF(z_in[3]); \
	if(flip==none) { Z[1] = SF(z_in[1]); Z[2] = SF(z_in[2]); } \
	else           { Z[2] = SF(z_in[1]); Z[1] = SF(z_in[2]); } \
	if(flip==flipall) { A = input.xb; B = input.xa; } \
	else              { B = input.xb; A = input.xa; } \
	gamma_intr[0] = -A - B + Z[0]; /* A=0 B=0 */ \
	gamma_intr[1] = -A + B + Z[1]; /* A=0 B=1 */ \
	gamma_intr[2] =  A - B + Z[2]; /* A=1 B=0 */ \
	gamma_intr[3] =  A + B + Z[3]; /* A=1 B=1 */ \
} while(0)

decoder_input input[N];
int16_t dec_out[N][2];

void DECODAGE() {
	uint8_t k_etat;  // 3 bits
	int16_t z[2][N][4];
	int16_t gamma_intr_f[4], gamma_intr_b[4];
	int16_t met_ext_backward[4*H_TREILLIS], met_ext_forward[4*H_TREILLIS];
	int16_t met_max_backward[2][PMAX][H_TREILLIS], met_max_forward[2][PMAX][H_TREILLIS];
	int16_t mem_met_backward[2][N][H_TREILLIS], mem_met_forward[2][N][H_TREILLIS];
	int16_t k,j;
	uint16_t k_f, k_b;
	uint16_t Lmin, nb_Lmax, Nhalf;
	int8_t proc;
	int32_t dec;
	uint16_t longueur[PMAX], pos[PMAX+1];
	uint16_t dualdec, dualkf, naturalorder_f;
	uint32_t dualkb, naturalorder_b;
	int32_t k_it;
	t_flip flip_f, flip_b;

	Nhalf = N / 2;
	Lmin = Nhalf / PMAX;
	nb_Lmax = Nhalf % PMAX;

	pos[0] = 0;
	for (k=0; k<PMAX; k++) {
		if(k<nb_Lmax) longueur[k] = Lmin+1;
		else longueur[k] = Lmin;
		pos[k+1] = pos[k] + 2*longueur[k];
	}

	for (k=0; k<N; k++) {
		for (dec=0; dec<=1; dec++) {
			for (j=0; j<=3; j++) z[dec][k][j] = 0;
		}
	}

	for (k_it=0; k_it<NB_ITERATION; k_it++) {

		for (dec=0; dec<=1; dec++) {
			for (proc=0; proc<PMAX; proc++) {
				for (k_etat=0; k_etat < H_TREILLIS; k_etat++){
					met_max_forward[dec][proc][k_etat] = 0;
					met_max_backward[dec][proc][k_etat] = 0;
				}
			}
		}

		for (dec=1; dec>=0; dec--) {
			for (proc=0; proc<PMAX; proc++) {
				for (k=-L_PROLOGUE; k<2*(Lmin+1); k++) {
					augh_iter_nb = 165;  // User annotation

					if (k<2*longueur[proc])	{
						augh_branch_prob = 99;  // User annotation

						k_f = augh_div16r(pos[proc] + k + N, N);
						k_b = augh_div16r(pos[proc] + 2*longueur[proc] - k - 1, N);

						if (k<longueur[proc] && k>=0) {
							augh_branch_prob = 46;  // User annotation
							for(k_etat=0; k_etat < H_TREILLIS; k_etat++) {
								mem_met_forward[dec][k_f][k_etat] = met_max_forward[dec][proc][k_etat];
								mem_met_backward[dec][k_b][k_etat] = met_max_backward[dec][proc][k_etat];
							}
						}
						if(dec==1) {
							dualdec = 0;
							dualkf = mat_ent[k_f];
							naturalorder_f = dualkf;
							flip_f = (k_f%2==1) ? none : flipall;

							dualkb = mat_ent[k_b];
							naturalorder_b = dualkb;
							flip_b = (k_b%2==1) ? none : flipall;
						}
						else {
							dualdec = 1;
							dualkf = mat_des[k_f];
							naturalorder_f = k_f;
							flip_f = (dualkf%2==1) ? none : flipz;

							dualkb = mat_des[k_b];
							naturalorder_b = k_b;
							flip_b = (dualkb%2==1) ? none : flipz;
						}

						interleaver(gamma_intr_f, input[naturalorder_f], z[dualdec][dualkf], flip_f);
						interleaver(gamma_intr_b, input[naturalorder_b], z[dualdec][dualkb], flip_b);

						CALC_MET_NOEUD(input[k_f], dec, forward,  gamma_intr_f, met_ext_forward,  met_max_forward[dec][proc]);
						CALC_MET_NOEUD(input[k_b], dec, backward, gamma_intr_b, met_ext_backward, met_max_backward[dec][proc]);

						if (k>=longueur[proc]){
							augh_branch_prob = 46;  // User annotation
							CALC_INFO_EXT (met_ext_forward,  mem_met_backward[dec][k_f], z[dec][k_f]);
							CALC_INFO_EXT (met_ext_backward, mem_met_forward[dec][k_b],  z[dec][k_b]);
							CALC_DECISION (gamma_intr_f, z[dec][k_f], dec_out[k_f]);
							CALC_DECISION (gamma_intr_b, z[dec][k_b], dec_out[k_b]);
						}
					}
				}  // PROLOGUE
			}  // PMAX
		}  // dec

	}  // NB_ITERATION

}



#ifdef AUGH_SYNTHESIS
void augh_main ()
#endif
#ifdef GCC_COMPIL
int main ()
#endif
{
	unsigned i, j;

	do {

		for(i=0; i<N; i++) {
			input[i].xa   = read8();
			input[i].xb   = read8();
			input[i].y1_0 = read8();
			input[i].y1_1 = read8();
			input[i].y2_0 = read8();
			input[i].y2_1 = read8();
		}

		DECODAGE();

		for(i=0; i<N; i++) for(j=0; j<2; j++) write16(dec_out[i][j]);

		#ifdef GCC_COMPIL
		break;
		#endif

	}while(1);

	#ifdef GCC_COMPIL
	return 0;
	#endif
}


