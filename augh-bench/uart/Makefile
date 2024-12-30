
# Optional debug tools tu use with AUGH
AUGH_DBG =
ifdef GDB
	AUGH_DBG = gdb --args
endif
ifdef VALGRIND
	AUGH_DBG = valgrind --leak-check=full
endif

.PHONY : augh

augh:
	rm -rf vhdl
	$(AUGH_DBG) augh -v -p xilinx -no-start -script cmds.augh
