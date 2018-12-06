# Compiler and linker flags
CXXFLAGS = -g -Wall -I. $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS = $(SYSTEMC_LIBRARY_DIRS)

# List of all ecutables to be compiled
EXECUTABLES = 01_lp_filter_ltf_tb 02_lp_filter_lsf_tb 03_lp_filter_eln_tb
# .PHONY targets don't generate files
.PHONY:	all clean

# Default targets
all:	$(EXECUTABLES)


# Rules defining the targets and their dependencies and how to generate
# the first from the latter.

01_lp_filter_ltf_tb:	01_lp_filter_ltf_tb.cpp \
			sin_source.h lp_filter_ltf.h sink.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -lsystemc-ams -lsystemc | c++filt

02_lp_filter_lsf_tb:	02_lp_filter_lsf_tb.cpp \
			sin_source.h lp_filter_ltf.h sink.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -lsystemc-ams -lsystemc | c++filt

03_lp_filter_eln_tb:	03_lp_filter_eln_tb.cpp \
			sin_source.h lp_filter_ltf.h sink.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -lsystemc-ams -lsystemc | c++filt

# Clean rule to delete temporary and generated files
clean:
	rm -rf *~ *.o *.dat *.vcd *.dSYM $(EXECUTABLES)
