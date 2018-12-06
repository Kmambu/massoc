CXXFLAGS  = -Wno-deprecated -Wall -O3

MODULE = run
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
HEADER = $(shell find . -name *.h)

INCDIR = -I./include -I./include/common $(SYSTEMC_INCLUDE_DIRS)
LIBDIR = $(SYSTEMC_LIBRARY_DIRS)
LIBS   = -lsystemc-ams -lsystemc -lm

EXE    = $(MODULE).exe

.SUFFIXES: .cc .cpp .o .exe

.PHONY:	all clean

all:	$(EXE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCDIR) $(LIBDIR) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt

%.o:%.cpp $(HEADER)
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $<

clean:
	rm -f $(OBJS) *~ $(EXE) trace/*.dat
