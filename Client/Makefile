GCC = g++
CFLAGS = -Wall
LIBS = -pthread -lrt

LIBSRC = tuner.cpp

LIBOBJ = $(LIBSRC:.cpp=.o)  

PARENTSRC =$(wildcard ../*.cpp)
PARENTOBJ =$(PARENTSRC:.cpp=.o) 

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

.PHONY: all
all: tunerlib

tunerlib: $(LIBOBJ) $(PARENTOBJ)
	ar rcs libtuner.a $(LIBOBJ) $(PARENTOBJ)

.PHONY: clean
clean:
	rm -rf *o *a