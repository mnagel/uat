GCC = g++
CFLAGS = -Wall -fPIC
LIBS = -pthread -lrt

LIBSRC = tuner.cpp SHA1.cpp
LIBOBJ = $(LIBSRC:.cpp=.o)  

WRAPPERSRC = tunerWrapper.cpp
WRAPPEROBJ = $(WRAPPERSRC:.cpp=.o)  

PARENTSRC =$(wildcard ../*.cpp)
PARENTOBJ =$(PARENTSRC:.cpp=.o) 

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

.PHONY: all
all: tunerlib ctunerlib

tunerlib: $(LIBOBJ) $(PARENTOBJ)
	ar rcs libtuner.a $(LIBOBJ) $(PARENTOBJ)

ctunerlib: $(WRAPPEROBJ) $(PARENTOBJ) $(LIBOBJ)
	$(GCC) -shared -o libctuner.so $(WRAPPEROBJ) $(LIBOBJ) $(PARENTOBJ)

.PHONY: clean
clean:
	rm -rf *.o *.a
