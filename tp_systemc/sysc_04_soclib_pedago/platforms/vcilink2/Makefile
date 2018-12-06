SOCLIB			= $(PWD)/../../soclib
SOCLIB_LIB		= $(SOCLIB)/lib-$(SYSTEMC_ARCH)/
SOCLIB_INCLUDE	= $(SOCLIB)/include
CXXFLAGS		= -Wall -Wno-deprecated -I. -I$(SOCLIB_INCLUDE) $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS			= -L. -L$(SOCLIB_LIB) $(SYSTEMC_LIBRARY_DIRS)

.PHONY:	all clean

all:	run.exe

run.exe:	system.cpp soclib_vci_local_crossbar_simple.h soclib_vci_simpleinitiator.h soclib_vci_simpletarget.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -lsystemc -lm -lsoclib 2>&1 | c++filt

clean:
	-rm -f *.o 
	-rm -f run.exe
	-rm -f core*
	-rm -f trace_file.vcd
	-rm -f TTY_* 
	-rm -f code-*.cc code-*.so

