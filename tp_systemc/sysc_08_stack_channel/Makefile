# Compiler and linker flags
CXXFLAGS = -g -Wall -I. $(SYSTEMC_INCLUDE_DIRS)
LDFLAGS = $(SYSTEMC_LIBRARY_DIRS)

# List of all ecutables to be compiled
EXECUTABLES = main

# .PHONY targets don't generate files
.PHONY:	all clean

# Default targets
all:	$(EXECUTABLES)

# Compile rules
main:	main.cpp consumer.h producer.h stack.h stack_if.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -lsystemc

# Clean rule to delete temporary and generated files
clean:
	rm -f $(EXECUTABLES) *.o *~
