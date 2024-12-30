#include "algo.h"

#define MAX_JAC(a,b) (MAX(a,b))


void CALC_MET_NOEUD (decoder_input input, uint8_t dec, way_t sens,
		int16_t gamma_intr[4],int16_t met_ext[4*H_TREILLIS],
		int16_t met_max_noeud[H_TREILLIS]){

	int8_t preced_forward[32]={0,2,5,7,1,3,4,6,6,4,3,1,7,5,2,0,1,3,4,6,0,2,5,7,7,5,2,0,6,4,3,1}; //sur 3 bits
	int8_t preced_backward[32]={0,4,1,5,6,2,7,3,7,3,6,2,1,5,0,4,4,0,5,1,2,6,3,7,3,7,2,6,5,1,4,0}; // sur 3 bits
	int8_t red1_forward[32]={0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,1,1,1,1,0,0}; // sur 1 bit
	int8_t red2_forward[32]={0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1}; // sur 1 bit
	int8_t red1_backward[32]={0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0}; // sur 1 bit
	int8_t red2_backward[32]={0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0}; // sur 1 bit

	int16_t met_cum_noeud[4*H_TREILLIS];
	int8_t met_trans_y1[2], met_trans_y2[2];
	/* métriques de transition */
	uint8_t k_etat, etat_prec, k_trans;
	int8_t red1, red2;
	int16_t  maxi =0;

	met_trans_y1[0]= (dec==1 ? -input.y1_1 : -input.y1_0);
	met_trans_y1[1]= (dec==1 ? input.y1_1 : input.y1_0);
	met_trans_y2[0]= (dec==1 ? -input.y2_1 : -input.y2_0);
	met_trans_y2[1]= (dec==1 ? input.y2_1 : input.y2_0);

	for (k_etat=0;k_etat < H_TREILLIS;k_etat++)  {
		for (k_trans=0;k_trans <= 3;k_trans++){
			etat_prec=(sens==forward ?preced_forward[H_TREILLIS*k_trans+k_etat]:preced_backward[H_TREILLIS*k_trans+k_etat]);
			red1=(sens==forward ? red1_forward[H_TREILLIS*k_trans+etat_prec]:red1_backward[H_TREILLIS*k_trans+etat_prec]);
			red2=(sens==forward ? red2_forward[H_TREILLIS*k_trans+etat_prec]:red2_backward[H_TREILLIS*k_trans+etat_prec]);
			met_ext[k_trans*H_TREILLIS+k_etat]=met_max_noeud[etat_prec] + met_trans_y1[red1] + met_trans_y2[red2];
			met_cum_noeud[k_trans*H_TREILLIS+k_etat]=met_ext[k_trans*H_TREILLIS+k_etat] + gamma_intr[k_trans];
		}
	}
	for (k_etat=0;k_etat < H_TREILLIS;k_etat++)
	{
		met_max_noeud[k_etat]= MAX_JAC(MAX_JAC(met_cum_noeud[k_etat],met_cum_noeud[8+k_etat]),
				MAX_JAC(met_cum_noeud[16+k_etat],met_cum_noeud[24+k_etat]));

		if (met_max_noeud[k_etat]> maxi)
			maxi = met_max_noeud[k_etat];
	}
	for (k_etat=0;k_etat < H_TREILLIS;k_etat++)
		met_max_noeud[k_etat]-=maxi;

}


void CALC_INFO_EXT (int16_t met_ext_forward[4*H_TREILLIS],
		int16_t mem_met_backward[H_TREILLIS],
		int16_t Z_out_f[4])
{
	int32_t i, k_etat, Z_max[H_TREILLIS/2];

	for (i=0;i<=3;i++)
	{
		/*Max pour i sur l'ensemble des états*/
		for (k_etat=0;k_etat<H_TREILLIS/2;k_etat++)
			Z_max[k_etat] = MAX_JAC(met_ext_forward[i*8+k_etat]+mem_met_backward[k_etat],
					met_ext_forward[i*8+k_etat+4]+mem_met_backward[k_etat+4]);

		Z_max[0] = MAX_JAC(Z_max[0],Z_max[1]);
		Z_max[1] = MAX_JAC(Z_max[2],Z_max[3]);
		Z_out_f[i] = MAX_JAC(Z_max[0],Z_max[1]);
	}
}


void CALC_DECISION (int16_t gamma_intr[4], int16_t Z[4],int16_t dec[2])
{
	int32_t i;
	int32_t somme[4];

	for (i=0;i<=3;i++)
	{
		/*Calcul des informations sur la décision i*/
		somme[i] = Z[i]+gamma_intr[i];
	}

	/* Elaboration de la décision pondérée sur A */
	dec[0] = MAX(somme[2],somme[3])-MAX(somme[0],somme[1]);

	/* Elaboration de la décision pondérée sur B */
	dec[1] = MAX(somme[1],somme[3])-MAX(somme[0],somme[2]);
}


