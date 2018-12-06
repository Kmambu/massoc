CXXFLAGS := -g -Wall $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS  := $(SYSTEMC_LIBRARY_DIRS)
LDLIBS   := -lsystemc-ams -lsystemc

PROGS    := tdf_vibration_sensor_tb de_tdf_vibration_sensor_tb

.PHONY:	clean

all:	$(PROGS)

tdf_vibration_sensor_tb.o:	tdf_vibration_sensor_tb.cpp \
				harmonic_sine_wavelets_source.h \
				tdf_vibration_sensor.h \
				tdf_sensor_frontend.h \
				tdf_programmable_gain_amplifier.h \
				ad_converter.h \
				tdf_abs_amplitude_averager.h \
				tdf_gain_controller.h

tdf_vibration_sensor_tb:	tdf_vibration_sensor_tb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 2>&1 | c++filt

de_tdf_vibration_sensor_tb.o:	de_tdf_vibration_sensor_tb.cpp \
				harmonic_sine_wavelets_source.h \
				tdf_vibration_sensor.h \
				de_tdf_sensor_frontend.h \
				de_programmable_gain_amplifier.h \
				ad_converter.h \
				de_abs_amplitude_averager.h \
				de_gain_controller.h

de_tdf_vibration_sensor_tb:	de_tdf_vibration_sensor_tb.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 2>&1 | c++filt

trace:
	gnuplot tdf_vibration_sensor_tb.plt de_tdf_vibration_sensor_tb.plt
clean:
	rm -f $(PROGS) *.o *~ *.dat *.eps
