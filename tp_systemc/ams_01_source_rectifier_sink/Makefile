# Compiler and linker flags
CXXFLAGS = -g -Wall -I. $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS = $(SYSTEMC_LIBRARY_DIRS)

# List of all ecutables to be compiled
EXECUTABLES = 01_source_sink_tb 02_source_rectifier_sink_tb

# .PHONY targets don't generate files
.PHONY:	all clean

# Default targets
all:	$(EXECUTABLES)


# Rules defining the targets and their dependencies and how to generate
# the first from the latter.

# 1st example can be directly compiled, as it only depends on one
# CPP file and the headers of th source and the sink, which also
# include their implementation.
01_source_sink_tb: 01_source_sink_tb.cpp sin_source.h sink.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -lsystemc-ams -lsystemc | c++filt

# 2nd example is constituted of two CPP implementation files, which have
# to be first compiled to object files before being linked. For
# big models, this is preferable, as the object files can be selectively
# updated after individual modifications.
02_source_rectifier_sink_tb: 02_source_rectifier_sink_tb.o rectifier.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ -lsystemc-ams -lsystemc | c++filt

# GNU make knows how to compile a CPP file using compiler $(CXX) and
# the flags $(CXXFLAGS).
02_source_rectifier_sink_tb.o: 02_source_rectifier_sink_tb.cpp sin_source.h rectifier.h sink.h

rectifier.o: rectifier.cpp rectifier.h


# Clean rule to delete temporary and generated files
clean:
	rm -rf *~ *.o *.dat *.vcd *.dSYM $(EXECUTABLES)
