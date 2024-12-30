
CFILE = vc709.c

AUGH_TECHNO = -p xilinx -board vc709

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
	rm -rf vhdl vivadoprj
	$(AUGH_DBG) augh -v $(AUGH_PARAMS) $(CFILE)
	#$(AUGH_DBG) augh -v -ex "build clkdiv 2" $(AUGH_PARAMS) $(CFILE)

bitstream:
	$(MAKE) -C vivadoprj

program:
	$(MAKE) -C vivadoprj xc3sprog
