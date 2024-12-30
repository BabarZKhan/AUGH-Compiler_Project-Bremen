
// This file contains template inline functions to use when writing
// an application for AUGH

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

#ifdef STDIO8
	#include <augh.h>

	uint8_t stdin;
	uint8_t stdout;

	#ifdef ASKSEND
	static inline uint8_t read8() {
		uint8_t u8 = 0;
		augh_write(stdout, &u8);  // Ask for a byte of data
		augh_read(stdin, &u8);    // Receive the data
		return u8;
	}
	static inline void write8(uint8_t u8) {
		uint8_t u8ask = 1;
		augh_write(stdout, &u8ask);  // Indicate it's data being sent
		augh_write(stdout, &u8);     // Send the data
	}
	#else
	static inline uint8_t read8() { uint8_t u8; augh_read(stdin, &u8); return u8; }
	static inline void write8(uint8_t u8) { augh_write(stdout, &u8); }
	#endif

	static inline int16_t read16_be() {
		uint16_t u16 = 0;
		uint8_t u8;
		uint8_t i;
		for(i=0; i<2; i++) { u8 = read8(); u16 = (u16 << 8) | u8; }
		return u16;
	}
	static inline int16_t read16_le() {
		uint16_t u16 = 0;
		uint8_t u8;
		uint8_t i;
		for(i=0; i<2; i++) { u8 = read8(); u16 = ((uint16_t)u8 << 8) | (u16 >> 8); }
		return u16;
	}

	static inline uint32_t read32_le() {
		uint32_t u32 = 0;
		uint8_t u8;
		uint8_t i;
		for(i=0; i<4; i++) { u32 >>= 8; u8 = read8(); u32 = (((uint32_t)u8) << 24) | u32; }
		return u32;
	}
	static inline uint32_t read32_be() {
		uint32_t u32 = 0;
		uint8_t u8;
		uint8_t i;
		for(i=0; i<4; i++) { u32 <<= 8; u8 = read8(); u32 = u8 | u32; }
		return u32;
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

	static inline void write32_be(uint32_t u32) {
		uint8_t i;
		uint8_t u8;
		for(i=0; i<4; i++) { u8 = (u32 >> 24) & 0xFF; u32 = u32 << 8; write8(u8); }
	}
	static inline void write32_le(uint32_t u32) {
		uint8_t i;
		uint8_t u8;
		for(i=0; i<4; i++) { u8 = u32 & 0xFF; u32 = u32 >> 8; write8(u8); }
	}

	#define read16 read16_be
	#define read32 read32_be

	#define write16 write16_be
	#define write32 write32_be

#endif

#ifdef GCC_COMPIL

	#include <stdio.h>
	#include <stdint.h>
	#include <stdbool.h>

	static inline int16_t read16_lr() {
		uint16_t u16 = 0;
		for(unsigned i=0; i<2; i++) { uint8_t u8 = getchar(); u16 = (u16 << 8) | u8; }
		return u16;
	}

	static inline int16_t read16_rl() {
		uint16_t u16 = 0;
		for(unsigned i=0; i<2; i++) { uint8_t u8 = getchar(); u16 = ((uint16_t)u8 << 8) | (u16 >> 8); }
		return u16;
	}

	#define read16 read16_lr

	static inline void write16_lr(uint16_t u16) {
		for(unsigned i=0; i<2; i++) {
			uint8_t u8 = (u16 >> 8) & 0xFF;
			u16 = u16 << 8;
			putchar(u8);
		}
	}
	static inline void write16_rl(uint16_t u16) {
		for(unsigned i=0; i<2; i++) {
			uint8_t u8 = u16 & 0xFF;
			u16 = u16 >> 8;
			putchar(u8);
		}
	}

	#define write16 write16_lr

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

