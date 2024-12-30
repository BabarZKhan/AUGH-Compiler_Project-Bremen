
# Optional debug tools tu use with AUGH
AUGH_DBG =
ifdef GDB
	AUGH_DBG = gdb --args
endif
ifdef VALGRIND
	AUGH_DBG = valgrind --leak-check=full
endif

.PHONY : augh synth program

augh:
	rm -rf vhdl/*.vhd
	$(AUGH_DBG) augh -v -p xilinx -script cmds.augh

synth:
	make -C xstprj synth

program:
	make -C xstprj program
