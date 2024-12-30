
#include <augh.h>
#include <vc709.h>

uint8_t stdin;
uint8_t stdout;


// Slowly change fan speed
// PWM frequency is 40KHz
// There are 257 PWM values -> one 1/257 PWM step lasts 97 ns
//   WARNING: due to bug in AUGH, the function nsleep() does not work, so cysleep() is used with hardcoded input frequency...
//   At 80 MHz = 12.5 ns period, 97 ns is 7.76 cycles => using 8 cycles
// We want ~5 seconds for PWM to change min->max
// For full range 0-256 this means increment/decrement of 257/5/40k = 1.285 ms per PWM period
// => Using fixed-point *2^16 => increment of 84 per loop iteration
void loop_fan_pwm() {
	const unsigned sh = 16;
	const unsigned step_cy = 8;
	const unsigned inc_abs = 84;
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
			vc709_fan_pwm = 1;
			cysleep(pwm8<<3);
		}
		if(pwm8 < pwm8_min || pwm32 < inc_abs) inc = inc_abs;

		// Generate the pulse at 0
		if(pwm8 < 256) {
			vc709_fan_pwm = 0;
			cysleep((256-pwm8)<<3);
		}
		if(pwm32 > (256 << sh) - inc_abs) inc = -inc_abs;

		// Increment the PWM counter
		pwm32 += inc;
	}while(1);

	// Set the fan ON
	vc709_fan_pwm = 1;
}

void loop_led_is_dipsw() {
	do {
		vc709_leds8 = vc709_dipsw;
	}while(1);
	vc709_leds8 = 0;
}

void loop_led_shift_time() {
	vc709_leds8 = 0;
	do {
		sleep(1);
		vc709_leds8 = vc709_leds8==0 ? 1 : (vc709_leds8 << 1);
	}while(1);
	vc709_leds8 = 0;
}

void loop_led_superchaser() {
	unsigned value_r = 0x001F;
	unsigned value_l = 0x000F;
	unsigned shr, shl;

	vc709_leds8 = 0;

	do {
		vc709_leds8 = (value_r | value_l) >> 4;
		usleep(50000);
		shr = value_r >> 1;
		shl = value_l << 1;
		value_r = shr | (shl & 0x8000);
		value_l = shl | (shr & 0x0001);
	} while(1);

	vc709_leds8 = 0;
}

void loop_led_shift_btn() {
	vc709_leds8 = 0;
	do {
		while(vc709_swpos==0);
		vc709_leds8 = vc709_leds8==0 ? 1 : (vc709_leds8 << 1);
		while(vc709_swpos!=0);
	} while(1);
	vc709_leds8 = 0;
}

// Send "Hello world!\n" on UART
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
}

void loop_hello_btnleds() {
	vc709_leds8 = 0;
	do {
		while(vc709_swpos==0);
		vc709_leds8 = vc709_leds8==0 ? 1 : (vc709_leds8 << 1);
		uart_hello();
		while(vc709_leds8!=0);
	} while(1);
	vc709_leds8 = 0;
}

// Top-level function for AUGH
void augh_main () {

	// Important: Set the fan to full speed for security
	vc709_fan_pwm = 1;

	vc709_leds8 = 0;

	// Say Hello World
	uart_hello();

	//loop_fan_pwm();

	//loop_led_is_dipsw();
	//loop_led_shift_time();
	loop_led_superchaser();
	//loop_led_shift_btn();

	//loop_hello_btnleds();

}

