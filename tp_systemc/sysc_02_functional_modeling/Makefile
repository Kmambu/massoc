# Compiler and linker flags
CXXFLAGS = -g -Wall -I$(SYSTEMC_INCLUDE_DIRS)
LDFLAGS = -L$(SYSTEMC_LIBRARY_DIRS)

# List of all ecutables to be compiled
EXECUTABLES = run.exe

# .PHONY targets don't generate files
.PHONY:	all clean

# Default targets
all:	$(EXECUTABLES)

# Compile rules
run.exe:	main.cpp DF_Adder.h DF_Const.h DF_Fork.h DF_Printer.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< -lsystemc

# Clean rule to delete temporary and generated files
clean:
	rm -f $(EXECUTABLES) *.o *~
