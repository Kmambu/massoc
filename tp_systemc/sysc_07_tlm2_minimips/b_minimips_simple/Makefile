# The use of tlm_utils::simple_target_socket requires
# SC_INCLUDE_DYNAMIC_PROCESSES to be defined
CPPFLAGS=$(SYSTEMC_INCLUDE_DIRS) -DSC_INCLUDE_DYNAMIC_PROCESSES
LDFLAGS=$(SYSTEMC_LIBRARY_DIRS) -lsystemc

# .PHONY targets don't generate files
.PHONY:	all clean

# Default target
all: run.exe

run.exe: main.cpp *.h
	g++ $(CPPFLAGS) -o $@ $(LDFLAGS) $<

clean:
	-rm -f run.exe
