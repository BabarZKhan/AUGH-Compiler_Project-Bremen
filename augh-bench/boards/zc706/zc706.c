
#include <augh.h>
#include <zc706.h>


// Slowly change fan speed
// PWM frequency is 40KHz
// There are 257 PWM values -> one 1/257 PWM step lasts 97 ns
//   WARNING: due to bug in AUGH, the function nsleep() does not work, so cysleep() is used with hardcoded input frequency...
//   At 200 MHz = 5 ns period, 97 ns is 19.4 cycles => using 20 cycles
// We want ~4 seconds for PWM to change min->max
// For full range 0-256 this means increment/decrement of 257/4/40k = 1.61 ms per PWM period
// => Using fixed-point *2^16 => increment of 105 per loop iteration
void loop_fan_pwm() {
	const unsigned sh = 16;
	const unsigned step_cy = 20;
	const unsigned inc_abs = 105;
	const unsigned pwm8_min = 64;

	unsigned pwm32;
	unsigned pwm8;
	int inc;

	// At start, the fan is at full speed, so its speed must decrease
	pwm32 = 256 << sh;
	inc = -inc_abs;

	// Infinite loop, iterates onces per PWM period
	do {
		// Get the actual PWM, range 0-256
		pwm8 = (pwm32 >> sh) & 0x1FF;

		// Generate the pulse at 1
		if(pwm8 > 0) {
			zc706_fan_pwm = 1;
			cysleep(pwm8<<3);
		}
		if(pwm8 < pwm8_min || pwm32 < inc_abs) inc = inc_abs;

		// Generate the pulse at 0
		if(pwm8 < 256) {
			zc706_fan_pwm = 0;
			cysleep((256-pwm8)<<3);
		}
		if(pwm32 > (256 << sh) - inc_abs) inc = -inc_abs;

		// Increment the PWM counter
		pwm32 += inc;
	}while(1);

	// Set the fan ON
	zc706_fan_pwm = 1;
}

void loop_led_is_dipsw() {
	do {
		zc706_leds = zc706_dipswitch;
	}while(1);
	zc706_leds = 0;
}

void loop_led_shift_time() {
	zc706_leds = 0;
	do {
		sleep(1);
		zc706_leds = zc706_leds==0 ? 1 : (zc706_leds << 1);
	}while(1);
	zc706_leds = 0;
}

void loop_led_superchaser() {
	unsigned value_r = 0x07;
	unsigned value_l = 0x03;
	unsigned shr, shl;

	zc706_led_0 = 0;
	zc706_leds = 0;

	do {
		zc706_led_0 = (value_r | value_l) >> 2;
		zc706_leds = (value_r | value_l) >> 3;
		usleep(50000);
		shr = value_r >> 1;
		shl = value_l << 1;
		value_r = shr | (shl & 0x80);
		value_l = shl | (shr & 0x01);
	} while(1);

	zc706_led_0 = 0;
	zc706_leds = 0;
}

void loop_led_shift_btn() {
	zc706_leds = 0;
	do {
		while(zc706_pushbuttons==0);
		zc706_leds = zc706_leds==0 ? 1 : (zc706_leds << 1);
		while(zc706_pushbuttons!=0);
	} while(1);
	zc706_leds = 0;
}

// Top-level function for AUGH
void augh_main () {

	// Important: Set the fan to full speed for security
	zc706_fan_pwm = 1;

	zc706_led_0 = 0;
	zc706_leds = 0;

	//loop_fan_pwm();

	//loop_led_is_dipsw();
	//loop_led_shift_time();
	loop_led_superchaser();
	//loop_led_shift_btn();

}

