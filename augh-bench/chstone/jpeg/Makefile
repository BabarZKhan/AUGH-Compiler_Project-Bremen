
CFILE = main.c

AUGH_TECHNO = -p xilinx -chip xc7v585t -speed 3 -package ffg1157 -freq 100M

# AUGH Parameters
#AUGH_PARAMS = $(AUGH_TECHNO)
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.elaborate.txt -chip-ratio 1%
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.elaborate.txt -chip-ratio 80%
AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.transcript.txt

# Debug tools tu use with AUGH
AUGH_DBG =
ifdef GDB
	AUGH_DBG = gdb --args
endif
ifdef VALGRIND
	AUGH_DBG = valgrind --leak-check=full
endif


.PHONY : default augh vectors exec

default:
	echo "Please specify a target."

augh:
	rm -rf vhdl/*.vhd
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) $(CFILE)

vectors:
	echo "Test vectors..."
	gcc --std=gnu99 -o test_vectors test_vectors.c
	./test_vectors

exec:
	gcc --std=gnu99 -I$(AUGH_HOME)/include -DGCC_EXEC -g -o jpeg main.c
	./jpeg


