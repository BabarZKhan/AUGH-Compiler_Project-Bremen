
CFILE = xupv5.c

AUGH_TECHNO = -p xilinx -board xupv5

# AUGH Parameters
#AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.elaborate.txt
AUGH_PARAMS = $(AUGH_TECHNO) -script cmds.transcript.txt

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
	rm -rf vhdl
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) $(CFILE)

bitstream:
	$(MAKE) -C xstprj

program:
	$(MAKE) -C xstprj program
