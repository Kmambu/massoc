#CXXFLAGS := -g -Wall $(SYSTEMC_INCLUDE_DIRS) -I./include -I./include/common
CXXFLAGS := -O3 -Wall $(SYSTEMC_INCLUDE_DIRS) -I./include -I./include/common
LDFLAGS  := $(SYSTEMC_LIBRARY_DIRS)
LDLIBS   := -lsystemc-ams -lsystemc

PROGS    := qpsk_trx

.PHONY:	all clean

all:	$(PROGS)

main.o:	main.cpp \
	./include/channel_AWGN.h \
	./include/common/noise.h \
	./include/common/rf.h \
	./include/pulse_src.h \
	./include/receiver/antenna.h \
	./include/receiver/decision.h \
	./include/receiver/integrator.h \
	./include/receiver/lna.h \
	./include/receiver/lo.h \
	./include/receiver/mixer.h \
	./include/receiver/mux.h \
	./include/receiver/sampler.h \
	./include/receiver.h \
	./include/transmitter/adder.h \
	./include/transmitter/antenna.h \
	./include/transmitter/demux.h \
	./include/transmitter/encoder.h \
	./include/transmitter/interpolator.h \
	./include/transmitter/lo.h \
	./include/transmitter/mixer.h \
	./include/transmitter/pa.h \
	./include/transmitter.h

qpsk_trx:	main.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 2>&1 | c++filt

clean:
	rm -f $(PROGS) *.o *~ trace/*.dat
