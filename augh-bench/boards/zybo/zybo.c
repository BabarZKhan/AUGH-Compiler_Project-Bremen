
#include <augh.h>
#include <zybo.h>

uint8_t stdin;
uint8_t stdout;


void loop_led4_xor_btnsw() {
	do {
		zybo_led4 = zybo_sw4 ^ zybo_btn4;
	}while(1);
	zybo_led4 = 0;
}

void loop_led4_shift_time() {
	zybo_led4 = 0;
	do {
		sleep(1);
		zybo_led4 = zybo_led4==0 ? 1 : (zybo_led4 << 1);
	}while(1);
	zybo_led4 = 0;
}

void loop_led4_shift_btn() {
	zybo_led4 = 0;
	do {
		while(zybo_btn4==0);
		zybo_led4 = zybo_led4==0 ? 1 : (zybo_led4 << 1);
		while(zybo_btn4!=0);
	} while(1);
	zybo_led4 = 0;
}

#if 1  // Comment if the board does not define stdout

// Send "Hello world!\n" on UART1
void uart_hello() {
	char c;
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
}

void loop_hello_btnleds() {
	zybo_led4 = 0;
	do {
		while(zybo_btn4==0);
		zybo_led4 = zybo_led4==0 ? 1 : (zybo_led4 << 1);
		uart_hello();
		while(zybo_btn4!=0);
	} while(1);
	zybo_led4 = 0;
}

#endif

// Top-level function for AUGH
void augh_main () {

	zybo_led4 = 0;

	//loop_led4_xor_btnsw();
	//loop_led4_shift_time();
	//loop_led4_shift_btn();
	loop_hello_btnleds();

}

