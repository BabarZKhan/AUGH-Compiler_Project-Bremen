#include <augh.h>

/* ---------- 32-bit rotation ---------- */
#define ROTL(n, X) (((X) << n) | ((X) >> (32 - n)))
#define ROTR(n, X) (((X) >> n) | ((X) << (32 - n)))


/* ---------- Fifo definition ---------- */
uint32_t fifo_input;
uint32_t fifo_output;

/* ---------- Global variables definition ---------- */
uint32_t key[4];
uint32_t text_in[2];
uint32_t command;
uint32_t text_out[2];

char simonz[65] = "11011011101011000110010111100000010010001010011100110100001111";


void SimonEncrypt() {
	/* ---------- Definition ---------- */
	unsigned m = 4;   // number of key words
	unsigned T = 44;  // number of rounds
	uint32_t x, y;    // plaintext words
	uint32_t k[70];   // key words
	unsigned i;       // counter

	/* ---------- Key expansion ---------- */
	for (i=0; i<m; i++) {
		k[i]=key[i];
	}
	for (i=m; i<T; i++) {
		uint32_t tmp = ROTR(3, k[i-1]);
		tmp = tmp ^ k[i-3];
		tmp = tmp ^ ROTR(1, tmp);
		k[i] = (~(k[i-m])) ^ tmp ^ (simonz[(i-m) % 62]-'0') ^ 3;
	}

	/* ---------- Encryption ---------- */
	x=text_in[0];
	y=text_in[1];
	for (i=0; i<T; i++) {
		uint32_t tmp = x;
		x = y ^ (ROTL(1,x) & ROTL(8,x)) ^ ROTL(2,x) ^ k[i];
		y = tmp;
	}

	text_out[0]=x;
	text_out[1]=y;
}


void SimonDecrypt() {
	/* ---------- Definition ---------- */
	unsigned m = 4;   // number of key words
	unsigned T = 44;  // number of rounds
	uint32_t x, y;    // plaintext words
	uint32_t k[70];   // key words
	unsigned i;       // counter

	/* ---------- Key expansion ---------- */
	for (i=0; i<m; i++) {
		k[i]=key[i];
	}
	for (i=m; i<T; i++) {
		uint32_t tmp = ROTR(3, k[i-1]);
		tmp = tmp ^ k[i-3];
		tmp = tmp ^ ROTR(1, tmp);
		k[i] = (~(k[i-m])) ^ tmp ^ (simonz[(i-m) % 62]-'0') ^ 3;
	}

	/* ---------- Decryption ---------- */
	x = text_in[0];
	y = text_in[1];

	for (i=0; i<T; i++) {
		uint32_t tmp = y;
		y = (ROTL(1,y) & ROTL(8,y)) ^ ROTL(2,y) ^ x ^ k[T-i-1];
		x = tmp;
	}

	text_out[0]=x;
	text_out[1]=y;
}


void augh_main() {
	unsigned k;
	while (1) {
		/* ---------- Command input ---------- */
		fifo_read(fifo_input, &command);

		/* ---------- Key input ---------- */
		if ((command ^ 0x00000001) == 0x00000000) {
			for (k=0; k<4; k++) {
				fifo_read(fifo_input, &key[3-k]);
			}
		}

		/* ---------- Encryption ---------- */
		else if ((command ^ 0x00000002) == 0x00000000) {
			for (k=0; k<2; k++) {
				fifo_read(fifo_input, &text_in[k]);
			}
			SimonEncrypt();
			for (k=0; k<2; k++) {
				fifo_write(fifo_output, text_out[k]);
			}
		}

		/* ---------- Decryption ---------- */
		else if ((command ^ 0x00000004) == 0x00000000) {
			for (k=0; k<2; k++) {
				fifo_read(fifo_input, &text_in[k]);
			}
			SimonDecrypt();
			for (k=0; k<2; k++) {
				fifo_write(fifo_output, text_out[k]);
			}
		}

	} /* ---------- Infinite loop ---------- */

}

