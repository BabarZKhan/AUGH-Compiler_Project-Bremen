
#include <augh.h>
#include <a5soc.h>


void loop_led_is_dipsw() {
	do {
		a5soc_leds = a5soc_dipsw;
	}while(1);
	a5soc_leds = 0;
}

void loop_led_shift_time() {
	a5soc_leds = 0;
	do {
		sleep(1);
		a5soc_leds = a5soc_leds==0 ? 1 : (a5soc_leds << 1);
	}while(1);
	a5soc_leds = 0;
}

void loop_led_superchaser() {
	unsigned value_r = 0x07;
	unsigned value_l = 0x03;
	unsigned shr, shl;

	a5soc_leds = 0;

	do {
		a5soc_leds = (value_r | value_l) >> 2;
		usleep(50000);
		shr = value_r >> 1;
		shl = value_l << 1;
		value_r = shr | (shl & 0x80);
		value_l = shl | (shr & 0x01);
	} while(1);

	a5soc_leds = 0;
}

void loop_led_shift_btn() {
	a5soc_leds = 0;
	do {
		while(a5soc_buttons==0);
		a5soc_leds = a5soc_leds==0 ? 1 : (a5soc_leds << 1);
		while(a5soc_buttons!=0);
	} while(1);
	a5soc_leds = 0;
}

// Top-level function for AUGH
void augh_main () {

	a5soc_leds = 0;

	//loop_led_is_dipsw();
	//loop_led_shift_time();
	loop_led_superchaser();
	//loop_led_shift_btn();

}

