
#include <augh_annot.h>

#if defined(AUGH_SYNTHESIS) && !defined(STDIO8)

	#include <augh.h>

	uint32_t stdin;
	uint32_t stdout;

	static inline uint32_t read32_raw() {
		uint32_t u32;
		augh_read(stdin, &u32);
		return u32;
	}
	static inline uint32_t read32_swap() {
		uint32_t u32;
		augh_read(stdin, &u32);
		u32 = ((u32 << 24) & 0xFF000000) | ((u32 << 8) & 0x00FF0000) | ((u32 >> 8) & 0x0000FF00) | ((u32 >> 24) & 0x000000FF);
		return u32;
	}

	#define fifo_read_size  read32_raw
	#define fifo_read_block read32_swap

	static inline void write32(uint32_t u32) {
		augh_write(stdout, &u32);
	}

	#define DEBUGVALUE(u32) write32(u32)

#endif

#if defined(AUGH_SYNTHESIS) && defined(STDIO8)

	#include "../stdio8_defs.h"

	#define fifo_read_size  read32_be
	#define fifo_read_block read32_le

	#define write32 write32_be

	#define DEBUGVALUE(u32) write32(u32)

#endif

#ifdef GCC_COMPIL

	#include <stdio.h>
	#include <stdlib.h>
	#include <stdint.h>

	FILE* Fi = NULL;
	void fifo_read_open() {
		if(Fi==NULL) Fi = fopen("input_vectors.bin", "rb");
		if(Fi==NULL) abort();
	}

	// Read bytes left to right
	uint32_t fifo_read32_lr() {
		if(Fi==NULL) fifo_read_open();
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = fgetc(Fi); u32 = (u32 << 8) | u8; }
		return u32;
	}

	// Read bytes right to left
	uint32_t fifo_read32_rl() {
		if(Fi==NULL) fifo_read_open();
		uint32_t u32 = 0;
		for(unsigned i=0; i<4; i++) { uint8_t u8 = fgetc(Fi); u32 = ((uint32_t)u8 << 24) | (u32 >> 8); }
		return u32;
	}

	#define fifo_read_size  fifo_read32_lr
	#define fifo_read_block fifo_read32_rl

	void write32(uint32_t u32) {
		for(unsigned i=0; i<4; i++) {
			uint8_t u8 = (u32 >> 24) & 0xFF;
			u32 = u32 << 8;
			//printf("%02x", u8);
			putchar(u8);
		}
	}

	#if 1  // Read simulation outputs and compare
	void DEBUGVALUE(uint32_t u32) {
		static FILE* F = NULL;
		static unsigned output_idx = 0;
		static char buf[600];

		if(F==NULL) F = fopen("output_vectors_simu.txt", "rb");
		if(F==NULL) abort();

		buf[0] = 0;
		fscanf(F, "%s", buf);
		if(buf[0]==0) {
			printf("Error: missing simu output %u\n", output_idx);
			abort();
		}
		// Convert to test to binary
		uint32_t simu_u32 = 0;
		for(char* ptr=buf; (*ptr)!=0; ptr++) simu_u32 = (simu_u32 << 1) | ((*ptr) != '0' ? 1 : 0);
		//printf("Info: simu output %u = 0x%08x\n", output_idx, simu_u32);
		// Compare outputs
		if(simu_u32 != u32) {
			printf("Error: simu output %u differs: simu was 0x%08x instead of 0x%08x\n", output_idx, simu_u32, u32);
			abort();
		}
		// Increment output idx for next one
		output_idx++;
	}
	#endif

	#if 0  // Write all outputs directly in binary: they become golden simulation outputs
	void DEBUGVALUE(uint32_t u32) {
		static FILE* F = NULL;
		if(F==NULL) F = fopen("output_vectors.bin", "wb");
		if(F==NULL) abort();
		write32(u32);
	}
	#endif

	// Uncomment this so all design outputs are checked gainst simulation results
	//#define write32(u32) DEBUGVALUE(u32)

#endif



#define SHA_BLOCKSIZE		64


// Shared data

uint32_t sha_info_digest[5];	/* message digest */
uint32_t sha_info_count_lo, sha_info_count_hi;	/* 64-bit bit count */
uint32_t sha_info_data[16];



/* SHA f()-functions */

#define f1(x,y,z)	((x & y) | (~x & z))
#define f2(x,y,z)	(x ^ y ^ z)
#define f3(x,y,z)	((x & y) | (x & z) | (y & z))
#define f4(x,y,z)	(x ^ y ^ z)

/* SHA constants */

#define CONST1		0x5a827999
#define CONST2		0x6ed9eba1
#define CONST3		0x8f1bbcdc
#define CONST4		0xca62c1d6

/* 32-bit rotate */

#define ROT32(x,n) ((x << n) | (x >> (32 - n)))



static inline void memset_loc(unsigned c, unsigned n, unsigned from) {
  unsigned i;
  for(i=0; i<n/4; i++) sha_info_data[from+i] = c;
}

static inline void read_one_block(unsigned n) {
	unsigned i;
	for(i=0; i<n/4; i++) {
		augh_iter_nb = 16;  // User annotation
		sha_info_data[i] = fifo_read_block();
	}
}

void sha_transform () {
  int i;
  uint32_t A, B, C, D, E, W[80];
	uint32_t func_temp;

  for (i = 0; i < 16; ++i)  W[i] = sha_info_data[i];
  for (i = 16; i < 80; ++i) W[i] = W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16];

  A = sha_info_digest[0];
  B = sha_info_digest[1];
  C = sha_info_digest[2];
  D = sha_info_digest[3];
  E = sha_info_digest[4];

	#define FUNC(n,i) do { \
		func_temp = ROT32(A,5) + f##n(B,C,D) + E + W[i] + CONST##n; \
		E = D; D = C; C = ROT32(B,30); B = A; A = func_temp; \
	}while(0)

  for (i = 0; i < 20; ++i)  FUNC (1, i);
  for (i = 20; i < 40; ++i) FUNC (2, i);
  for (i = 40; i < 60; ++i) FUNC (3, i);
  for (i = 60; i < 80; ++i) FUNC (4, i);

	#undef FUNC

  sha_info_digest[0] += A;
  sha_info_digest[1] += B;
  sha_info_digest[2] += C;
  sha_info_digest[3] += D;
  sha_info_digest[4] += E;
}

void sha_init () {
  sha_info_digest[0] = 0x67452301L;
  sha_info_digest[1] = 0xefcdab89L;
  sha_info_digest[2] = 0x98badcfeL;
  sha_info_digest[3] = 0x10325476L;
  sha_info_digest[4] = 0xc3d2e1f0L;
  sha_info_count_lo = 0L;
  sha_info_count_hi = 0L;
}

static inline void sha_update (unsigned count) {
	unsigned idxcol_indata = 0;
	if ((sha_info_count_lo + ((uint32_t) count << 3)) < sha_info_count_lo) sha_info_count_hi ++;
	sha_info_count_lo += (uint32_t) count << 3;
	sha_info_count_hi += (uint32_t) count >> 29;
	while (count >= SHA_BLOCKSIZE) {
		augh_iter_nb = 256;  // User annotation
		read_one_block(SHA_BLOCKSIZE);
		sha_transform ();
		count -= SHA_BLOCKSIZE;
	}
	read_one_block(count);
}

void sha_final () {
  unsigned count;
  unsigned s;
  uint32_t lo_bit_count;
  uint32_t hi_bit_count;

  lo_bit_count = sha_info_count_lo;
  hi_bit_count = sha_info_count_hi;
  count = (lo_bit_count >> 3) & 0x3f;
  sha_info_data[count++] = 0x80;

  if (count > 56) {
		memset_loc(0, 64-count, count);
		sha_transform();
		memset_loc(0, 56, 0);
	}
  else {
		memset_loc(0, 56-count, count);
	}

  sha_info_data[14] = hi_bit_count;
  sha_info_data[15] = lo_bit_count;
  sha_transform ();
}

// Compute the SHA digest for data from standard input FIFO
static inline void sha_stream (uint32_t size) {
	sha_init();
	sha_update(size);
	sha_final();
}


#ifdef AUGH_SYNTHESIS
void augh_main()
#endif
#ifdef GCC_COMPIL
int main()
#endif
{
	unsigned i;
	uint32_t read_size;

	#ifdef ASKSEND
	read8();
	#endif

	do {

		read_size = fifo_read_size();

		sha_stream(read_size);

		for(i=0; i<5; i++) write32(sha_info_digest[i]);

		#ifdef GCC_COMPIL
		break;
		#endif
	} while(1);

	#ifdef GCC_COMPIL
	return 0;
	#endif
}

