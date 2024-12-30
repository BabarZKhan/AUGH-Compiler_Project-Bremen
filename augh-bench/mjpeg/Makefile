
CFILE = decoder.c

# Note: to reduce image size, use this: convert ice_age_256x144.mjpeg -size 64x64 ice_age_64x64.mjpeg
#MPGFILE = ../../ice_age_256x144.mjpeg
MPGFILE = ../../ice_age_64x64.mjpeg

AUGH_TECHNO = -p xilinx -chip xc7v585t -speed 3 -package ffg1157 -freq 100M

# AUGH Parameters
#AUGH_PARAMS = $(AUGH_TECHNO)
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.elaborate.txt -chip-ratio 1%
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.elaborate.txt -chip-ratio 80%
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.schedule.txt
AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.transcript.txt

# Debug tools tu use with AUGH
AUGH_DBG =
ifdef GDB
	AUGH_DBG = gdb --args
endif
ifdef VALGRIND
	AUGH_DBG = valgrind --leak-check=full
endif

# Options for GHDL
GHDL_PARAMS = --workdir=work

# Options for hexdump
HEX_FLAGS = -v -e '"%08.8_ax  " 32/1 "%02x " "\n"'


.PHONY : default augh bitstream \
	xupv5 xupv5-augh xupv5-bitstream xupv5-program xupv5-pipe xupv5-check \
	vectors gcov simulate simulate1

default:
	echo "Please specify a target."

augh:
	rm -rf vhdl/*.vhd
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) $(CFILE)

bitstream:
	rm -rf vhdl/*
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) -board xupv5 $(CFILE)
	$(MAKE) -C xstprj

xupv5:
	$(MAKE) xupv5-augh
	$(MAKE) xupv5-bitstream
	$(MAKE) xupv5-program
	$(MAKE) xupv5-pipe
	$(MAKE) xupv5-check

# WARNING Take care to the buffer size: must be big enough to avoid overrun (PC sends faster than design can process)
# So: insert a big circular buffer, or implement a handshaking protocol (and use compatible software on the PC)
# Note: A clock divider is necessary (10 doesn't work, 40 works)
# Note: With elabo, the design should not be synthesizable because of excessive LUTRAM usage
xupv5-augh:
	rm -rf vhdl xstprj
	#augh -v -p xilinx -board xupv5 -DSTDIO8 -script ../cmds.xupv5.transcript $(CFILE)
	#augh -v -p xilinx -board xupv5 -DSTDIO8 -DASKSEND -script ../cmds.xupv5.transcript $(CFILE)
	augh -v -p xilinx -board xupv5 -DSTDIO8 -DASKSEND -ex "build clkdiv 40" -script ../cmds.xupv5.transcript $(CFILE)
	#augh -v -p xilinx -board xupv5 -DSTDIO8 -ex "build buffer -compmod uart -in circbuf 5000" -script ../cmds.xupv5.transcript $(CFILE)
	#augh -v -p xilinx -board xupv5 -DSTDIO8 -DASKSEND -script ../cmds.xupv5.elabo $(CFILE)
	#augh -v -p xilinx -board xupv5 -DSTDIO8 -DASKSEND -ex "build clkdiv 4" -script ../cmds.xupv5.elabo $(CFILE)

xupv5-bitstream:
	$(MAKE) -C xstprj

xupv5-program:
	$(MAKE) -C xstprj program

xupv5-pipe:
	#~/Sandbox/serial/serial -if input_vectors.bin -of output_xupv5.bin -out-max 100 -asksend -debug
	#~/Sandbox/serial/serial -if input_vectors.bin -of output_xupv5.bin -out-max `stat -c '%s' output_vectors.bin` -in-us 50000
	~/Sandbox/serial/serial -if input_vectors.bin -of output_xupv5.bin -out-max `stat -c '%s' output_vectors.bin` -asksend -debug
	#~/Sandbox/serial/serial -if input_vectors.bin -of output_xupv5.bin -out-max `stat -c '%s' output_vectors.bin` -asksend -debug -b 9600
	#~/Sandbox/serial/serial -if input_vectors.bin -of output_xupv5.bin

xupv5-check:
	cat output_vectors.bin | hexdump $(HEX_FLAGS) > output_vectors.hex
	cat output_xupv5.bin | hexdump $(HEX_FLAGS) > output_xupv5.hex
	diff -q output_xupv5.hex output_vectors.hex
	echo "Test vectors: SUCCESS"

vectors:
	echo "Test vectors..."
	gcc --std=gnu99 -o test_vectors test_vectors.c
	cat $(MPGFILE) | ./test_vectors > input_vectors.bin
	echo "Execution for output vectors..."
	gcc --std=gnu99 -Wall -I$(AUGH_HOME)/include -DGCC_COMPIL -o decoder $(CFILE)
	cat input_vectors.bin | ./decoder > output_vectors.bin
	echo "Conversion to hex..."
	cat input_vectors.bin | hexdump $(HEX_FLAGS) > input_vectors.hex
	cat output_vectors.bin | hexdump $(HEX_FLAGS) > output_vectors.hex

gcov:
	echo "Launching gcov"
	gcc --std=gnu99 -Wall -I$(AUGH_HOME)/include -DGCC_COMPIL -fprofile-arcs -ftest-coverage -o decoder $(CFILE)
	cat $(MPGFILE) | ./decoder > /dev/null
	gcov $(CFILE)

simulate:
	$(MAKE) -C vhdl -f ../Makefile simulate1

simulate1:
	mkdir -p work
	# Only compile the entire design ?
	ghdl -a $(GHDL_PARAMS) *.vhd
	# Generate the entire simulator ?
	#ghdl -e $OPTIONS top

