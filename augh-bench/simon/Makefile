
CFILE = simon.c

# Debug tools to use with AUGH
AUGH_DBG =
ifdef GDB
	AUGH_DBG = gdb --args
endif
ifdef VALGRIND
	AUGH_DBG = valgrind --leak-check=full
endif

AUGH_FLAGS = -p xilinx -chip xc7k160t -no-synth-prj

.PHONY : default augh cpsimu vectors

default:
	echo "Please specify a target."

augh:
	rm -rf vhdl/*.vhd
	$(AUGH_DBG) augh -v $(AUGH_FLAGS) $(CFILE)

cpsimu:
	cp vhdl/*.vhd bench/one/vhdl/

vectors:
	echo "Test vectors..."
	gcc --std=gnu99 -o test_vectors test_vectors.c
	./test_vectors

