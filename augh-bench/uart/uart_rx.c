
// Warning: Always ensure there is no wait-on-start loop
// FIXME With initial values of global variables, the entire design can be reduced to one state -> no FSM component
// FIXME Finish AUGH functionality for cramming, and replace conditional TryWrite by unconditional stuff


#include <augh.h>

#define BITSNB  8
#define CLKDIV  800

int1_t stdin;
int8_t stdout;

// For the clock divider
unsigned clkcnt;
bool     clkdiv_en;  // value is true when clkcnt overflows

// For the state register
#define STATE_START  0
#define STATE_BITS   1
#define STATE_STOP   2
unsigned state;

// For the data buffer
uint8_t buf;
bool    buf_avail;

// When reading bits: number of bits read
unsigned bits_nb;

// A flag to use with trywrite
bool b;

void augh_main() {

	// Indicate to AUGH what are the top-level interfaces
	augh_access_uart_rx(stdin);
	augh_access_fifo_out(stdout);

	clkcnt = 0;
	state = 0;
	buf_avail = false;

	// This loop does one iteration per clock cycle
	do {

		BEG_CLK_CYCLE:

		// Increment the clock cycle counter
		clkdiv_en = false;
		if(clkcnt==CLKDIV-1) {
			clkdiv_en = true;
			clkcnt = 0;
		}
		else {
			clkcnt = clkcnt + 1;
		}

		// FIFO output
		// Note: The top-level ports must be written to unconditionally for cramming to work
		fifoout_setdata(stdout, &buf);
		fifoout_setreadyval(stdout, buf_avail);
		b = fifoout_spyready(stdout);
		if(b==true) buf_avail = false;

		// State-specific actions
		switch(state) {

			case STATE_START: {
				bits_nb = 0;
				if(stdin!=0 && clkcnt < CLKDIV/2) {
					// Assume we are still in a stop bit
					clkcnt = 0;
				}
				if(clkdiv_en==true) {
					state = STATE_BITS;
				}
				break;
			}

			case STATE_BITS: {
				// Read data bits at the middle of the duration of symbols
				if(clkcnt==CLKDIV/2) {
					buf_avail = false;  // Drop any previously unread data
					buf = (buf >> 1) | ( (unsigned)stdin << (BITSNB-1) );
					bits_nb ++;
				}
				if(clkdiv_en==true) {
					if(bits_nb==BITSNB) {
						buf_avail = true;
						state = STATE_STOP;
					}
				}
				break;
			}

			case STATE_STOP: {
				// Wait for only half a symbol to let some slack
				if(clkcnt==CLKDIV/2) {
					clkcnt = 0;
					state = STATE_START;
				}
				break;
			}

			default: break;
		}

		END_CLK_CYCLE:

	}while(1);

}


