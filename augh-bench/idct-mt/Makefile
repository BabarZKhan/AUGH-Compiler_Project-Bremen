
CFILE = idct-mt.c

AUGH_TECHNO = -p xilinx -chip xc7v585t -speed 3 -package ffg1157 -freq 100M

# AUGH Parameters
#AUGH_PARAMS = $(AUGH_TECHNO)
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


.PHONY : default augh

default:
	echo "Please specify a target."

augh:
	rm -rf vhdl/*.vhd xstprj/*
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) $(CFILE)

