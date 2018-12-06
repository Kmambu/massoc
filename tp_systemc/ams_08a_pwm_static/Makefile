CXXFLAGS=-g -Wall $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS=$(SYSTEMC_LIBRARY_DIRS)
LDLIBS=-lsystemc-ams -lsystemc

PROGRAMS:=pwm_static

.PHONY:		all clean

all:		$(PROGRAMS)

pwm_static:	test_bench.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test_bench.o:	test_bench.cpp ref_src.h diff.h pwm.h drv_motor.h pi_ctrl.h

clean:
	-rm $(PROGRAMS) *~ *.o *.dat *.vcd
