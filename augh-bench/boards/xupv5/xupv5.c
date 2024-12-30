
#include <augh.h>
#include <xupv5.h>

// These will be replaced by uart1 interface
uint8_t stdin;
uint8_t stdout;

// Produce a beep on the piezzo
// The frequency is set with the DIP switches
void beep() {
	unsigned duration;
	xupv5_piezzo = 0;
	do {
		duration = 100 + xupv5_dipsw * 10;
		usleep(duration);
		xupv5_piezzo = ~xupv5_piezzo;
	}while((xupv5_swpos & XUPV5_POS_CENTER)==0);
}

// The frequency can be changed with the rotary encoder
// Init = 1 kHz, resolution = 100 Hz
void beep_rot() {
	unsigned duration = 1000;
	uint8_t rot_old = xupv5_rotary;
	uint8_t rot_new;
	unsigned i;
	unsigned upd = 0;
	xupv5_piezzo = 0;
	do {
		for(i=0; i<duration; i++) {
			usleep(1);
			// Check rotary encoder.
			// This ensures the check is not too often AND often enough.
			upd++;
			if(upd>5000) {
				rot_new = xupv5_rotary;
				if(XUPV5_ROTARY_CHECKMOVE_CLOCKWISE(rot_old, rot_new)) duration -= duration/16;
				if(XUPV5_ROTARY_CHECKMOVE_CLOCKWISE(rot_new, rot_old)) duration += duration/16;
				rot_old = rot_new;
				upd = 0;
			}
		}
		xupv5_piezzo = ~xupv5_piezzo;
	}while((xupv5_swpos & XUPV5_POS_CENTER)==0);
	xupv5_piezzo = 0;
}

// Send "Hello world!\n" on UART1
void uart_hello() {
	char c;

	#if 1
	c = 'H';
	augh_write(stdout, &c);
	c = 'e';
	augh_write(stdout, &c);
	c = 'l';
	augh_write(stdout, &c);
	c = 'l';
	augh_write(stdout, &c);
	c = 'o';
	augh_write(stdout, &c);
	c = ' ';
	augh_write(stdout, &c);
	c = 'w';
	augh_write(stdout, &c);
	c = 'o';
	augh_write(stdout, &c);
	c = 'r';
	augh_write(stdout, &c);
	c = 'l';
	augh_write(stdout, &c);
	c = 'd';
	augh_write(stdout, &c);
	c = '!';
	augh_write(stdout, &c);
	c = '\n';
	augh_write(stdout, &c);
	#endif

	#if 0  // For branch hvex only
	static char msg[] = "Hello World!\n";
	unsigned i = 0;
	// Send characters
	do {
		c = msg[i++];
		if(c==0) break;
		augh_write(stdout, &c);
	} while(1);
	#endif

	// This loop avoids sending the message hundreds of times due to button glitches
	while((xupv5_swpos & XUPV5_POS_CENTER)==0);
}

// Send numbers from 0x00 to 0xFF on the UART1
void uart_count255() {
	uint8_t c = 0;
	bool finished = false;
	do {
		augh_write(stdout, &c);
		if(c==0xFF) finished = true;
		c++;
	}while(finished==false);
	// This loop avoids sending the message hundreds of times due to button glitches
	while((xupv5_swpos & XUPV5_POS_CENTER)==0);
}

// Listen on UART1 and repeat everything
// Display the characters on the 8 GPIO leds
void uart_echo() {
	char c;
	bool b;
	xupv5_leds8 = 0;
	do {
		b = augh_tryread(stdin, &c);
		if(b==true) {
			xupv5_leds8 = c;
			augh_write(stdout, &c);
		}
	}while((xupv5_swpos & XUPV5_POS_CENTER)==0);
	xupv5_leds8 = 0;
}

// Display the DIP switches state on the 8 GPIO leds
void dipsw() {
	do {
		xupv5_leds8 = xupv5_dipsw;
	}while((xupv5_swpos & XUPV5_POS_CENTER)==0);
	xupv5_leds8 = 0;
}

// Make the 8 GPIO leds blink slowly
// The frequency can be changed with the rotary encoder
// Init = 1 Hz, resolution = 0.1 Hz
void led8_blink() {
	unsigned duration = 1000;
	uint8_t rot_old = xupv5_rotary;
	uint8_t rot_new;
	unsigned i;
	unsigned upd = 0;
	xupv5_leds8 = 0;
	do {
		for(i=0; i<duration; i++) {
			usleep(1000);
			// Check rotary encoder.
			// This ensures the check is not too often AND often enough.
			upd++;
			if(upd>5) {
				rot_new = xupv5_rotary;
				if(XUPV5_ROTARY_CHECKMOVE_CLOCKWISE(rot_old, rot_new)) duration -= duration/8;
				if(XUPV5_ROTARY_CHECKMOVE_CLOCKWISE(rot_new, rot_old)) duration += duration/8;
				rot_old = rot_new;
				upd = 0;
			}
			if((xupv5_swpos & XUPV5_POS_CENTER)!=0) break;
		}
		xupv5_leds8 = ~xupv5_leds8;
	}while((xupv5_swpos & XUPV5_POS_CENTER)==0);
	xupv5_leds8 = 0;
}


// Top-level function for AUGH
void augh_main () {

	xupv5_leds8 = 0;
	xupv5_ledspos = 0;
	xupv5_lederr1 = 0;
	xupv5_lederr2 = 0;
	xupv5_piezzo = 0;

	do {

		if((xupv5_swpos & XUPV5_POS_NORTH)!=0) {
			//beep();
			beep_rot();
		}
		if((xupv5_swpos & XUPV5_POS_WEST)!=0) {
			uart_hello();
			//uart_count255();
		}
		if((xupv5_swpos & XUPV5_POS_EAST)!=0) {
			uart_echo();
		}
		if((xupv5_swpos & XUPV5_POS_SOUTH)!=0) {
			//dipsw();
			led8_blink();
		}

	}while(1);

}

