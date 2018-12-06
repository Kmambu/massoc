CXXFLAGS=-g -Wall $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS=$(SYSTEMC_LIBRARY_DIRS)
LDLIBS=-lsystemc-ams -lsystemc

PROGRAMS:=pwm_dynamic pwm_dynamic_lsf

.PHONY:		all clean

all:		$(PROGRAMS)

pwm_dynamic:	test_bench.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test_bench.o:	test_bench.cpp ref_src.h diff.h pwm.h drv_motor.h pi_ctrl.h

pwm_dynamic_lsf:	test_bench.cpp ref_src.h diff.h pwm.h drv_motor.h pi_ctrl.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS) -DUSE_LSF_FOR_PI_CTRL

clean:
	-rm $(PROGRAMS) *~ *.o *.dat *.vcd
