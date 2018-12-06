CXXFLAGS = -Wall $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS = $(SYSTEMC_LIBRARY_DIRS)
LDLIBS = -lsystemc

PROGRAMS =	tlm2_getting_started_1 tlm2_getting_started_2 \
			tlm2_getting_started_3 tlm2_getting_started_31 \
			tlm2_getting_started_4 tlm2_getting_started_5 \
			tlm2_getting_started_6

.PHONY:	all clean

%:	%.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

all:	$(PROGRAMS)

tlm2_getting_started_1:	tlm2_getting_started_1.cpp

tlm2_getting_started_2:	tlm2_getting_started_2.cpp

tlm2_getting_started_3:	tlm2_getting_started_3.cpp

tlm2_getting_started_31:	tlm2_getting_started_31.cpp

tlm2_getting_started_4:	tlm2_getting_started_4.cpp

tlm2_getting_started_5:	tlm2_getting_started_5.cpp

tlm2_getting_started_6:	tlm2_getting_started_6.cpp

clean:
	rm -f *~ *.o $(PROGRAMS) foo.txt