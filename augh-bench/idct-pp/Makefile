
CFILE = idct.c

AUGH_TECHNO = -p xilinx -chip xc7v585t -speed 3 -package ffg1157 -freq 100M

# AUGH Parameters
#AUGH_PARAMS = $(AUGH_TECHNO)
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.elaborate.txt -chip-ratio 1%
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.elaborate.txt -chip-ratio 80%
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.schedule.txt
AUGH_PARAMS = -p xilinx -board xupv5 -script cmds.transcript.txt

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


.PHONY : default augh gcc simulate simulate1

default:
	echo "Please specify a target."

augh:
	rm -rf vhdl/*.vhd
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) $(CFILE)

gcc:
	echo "Test vectors..."
	gcc --std=gnu99 -o test_vectors test_vectors.c
	./test_vectors > input_vectors.bin
	echo "Execution for output vectors..."
	gcc --std=gnu99 -I$(AUGH_HOME)/include -DGCC_COMPIL -o idct idct.c
	cat input_vectors.bin | ./idct > output_vectors.bin

simulate:
	$(MAKE) -C vhdl -f ../Makefile simulate1

simulate1:
	mkdir -p work
	# Only compile the entire design ?
	ghdl -a $(GHDL_PARAMS) *.vhd
	# Generate the entire simulator ?
	#ghdl -e $OPTIONS top

