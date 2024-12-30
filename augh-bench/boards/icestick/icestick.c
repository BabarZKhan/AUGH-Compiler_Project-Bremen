
#include <augh.h>
#include <icestick.h>


void local_sleep() {
	usleep(500000);
}

void loop_led_chaser() {
	unsigned leds_cur = ICESTICK_LED1 | ICESTICK_LED2;

	do {
		icestick_leds5 = leds_cur;
		local_sleep();
		// Make the center LED blink
		icestick_leds5 = leds_cur | ICESTICK_LED5;
		local_sleep();
		icestick_leds5 = leds_cur;
		local_sleep();
		// Compute next LED status
		leds_cur = ( (leds_cur & 0x0F) >> 1) | ( (leds_cur & 0x01) << 3);
	} while(1);

	icestick_leds5 = 0;
}

// Top-level function for AUGH
void augh_main () {

	icestick_leds5 = 0;

	loop_led_chaser();

}

