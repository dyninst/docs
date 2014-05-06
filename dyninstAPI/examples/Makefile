ifeq ($(DYNINST_ROOT),)
$(error DYNINST_ROOT is not set)
endif

ifeq ($(PLATFORM),)
$(error PLATFORM is not set)
endif

# These should point to where libelf and libdwarf are installed
LIBELF = /p/paradyn/packages/libelf/lib
LIBDWARF = /p/paradyn/packages/libdwarf/lib

DYNINST_INCLUDE = $(DYNINST_ROOT)/include
DYNINST_LIB = $(DYNINST_ROOT)/lib

CC = gcc
CCFLAGS = -g -Wall

CXX = g++
CXXFLAGS = -g -Wall -std=c++0x

TARGETS = instrumentFunc binaryAnalysis memoryAccesses InterestingProgram retee
all:$(TARGETS)

DYNINST_LIBS = -L$(DYNINST_LIB) \
			   -L$(LIBELF) -L$(LIBDWARF) \
			   -lcommon \
			   -ldwarf -lelf \
			   -ldyninstAPI \
			   -ldl

InterestingProgram: InterestingProgram.c
	$(CC) $(CCFLAGS) -o $@ $^

instrumentFunc: instrumentFunc.o
	$(CXX) $(CXXFLAGS) -I$(DYNINST_INCLUDE) $(DYNINST_LIBS) -o $@ $^ 

binaryAnalysis: binaryAnalysis.o
	$(CXX) $(CXXFLAGS) -I$(DYNINST_INCLUDE) $(DYNINST_LIBS) -o $@ $^ 

memoryAccesses: memoryAccesses.o
	$(CXX) $(CXXFLAGS) -I$(DYNINST_INCLUDE) $(DYNINST_LIBS) -o $@ $^ 

retee: retee.o
	$(CXX) $(CXXFLAGS) -I$(DYNINST_INCLUDE) $(DYNINST_LIBS) -o $@ $^ 

%.o: %.C
	$(CXX) $(CXXFLAGS) -I$(DYNINST_INCLUDE) -I$(LIBELF) -I$(LIBDWARF) -c $*.C

clean: 
	rm -f *.o $(TARGETS) core.* tmp

