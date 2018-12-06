#CXXFLAGS := -g -Wall $(SYSTEMC_INCLUDE_DIRS) -I./include -I./include/common
CXXFLAGS := -O3 -Wall $(SYSTEMC_INCLUDE_DIRS) -I./include -I./include/common
LDFLAGS  := $(SYSTEMC_LIBRARY_DIRS)
LDLIBS   := -lsystemc-ams -lsystemc

PROGS    := run.exe

.PHONY:	all clean

all:	$(PROGS)

run.exe:	main.cpp coords.h sink.h wavegen.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS) 2>&1 | c++filt

clean:
	rm -f $(PROGS) *~ *.o tr.vcd iter*
