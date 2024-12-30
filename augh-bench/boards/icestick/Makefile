
CFILE = icestick.c

AUGH_TECHNO = -p lattice -board icestick

# AUGH Parameters
#AUGH_PARAMS = $(AUGH_TECHNO) -no-start -script cmds.elaborate.txt
AUGH_PARAMS = $(AUGH_TECHNO) -no-start -script cmds.transcript.txt

# Debug tools tu use with AUGH
AUGH_DBG =
ifdef GDB
	AUGH_DBG = gdb --args
endif
ifdef VALGRIND
	AUGH_DBG = valgrind --leak-check=full
endif


.PHONY : augh bitstream program

augh:
	rm -rf vhdl yosysprj
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) $(CFILE)

bitstream:
	$(MAKE) -C yosysprj

