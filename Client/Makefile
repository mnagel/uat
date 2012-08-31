GCC = g++
CFLAGS = -Wall
LIBS = -pthread -lrt

#CLIENTSRC =$(wildcard *.cpp)
#DUMMYCLIENTSRC =$(wildcard *.cpp)

CLIENTSRC = testClient.cpp tuner.cpp
DUMMYCLIENTSRC = dummyClient.cpp tuner.cpp
EVALOVERHEADSRC = evalOverhead.cpp tuner.cpp
EVALMULTIPLESHAREDSRC = evalMultipleSectionsSharedParams.cpp tuner.cpp
EVALIMPLICITSRC = evalImplicit.cpp tuner.cpp


CLIENTOBJ =$(CLIENTSRC:.cpp=.o)  
DUMMYCLIENTOBJ =$(DUMMYCLIENTSRC:.cpp=.o)  
EVALOVERHEADOBJ =$(EVALOVERHEADSRC:.cpp=.o)  
EVALMULTIPLESHAREDOBJ =$(EVALMULTIPLESHAREDSRC:.cpp=.o)  
EVALIMPLICITOBJ =$(EVALIMPLICITSRC:.cpp=.o)  

PARENTSRC =$(wildcard ../*.cpp)
PARENTOBJ =$(PARENTSRC:.cpp=.o) 

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

.PHONY: all
all: client dummyclient evaloverhead evalmultipleshared evalimplicit

client: $(CLIENTOBJ) $(PARENTOBJ)
	$(GCC) $(CLIENTOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

dummyclient:  $(DUMMYCLIENTOBJ) $(PARENTOBJ)
	$(GCC) $(DUMMYCLIENTOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

evaloverhead:  $(EVALOVERHEADOBJ) $(PARENTOBJ)
	$(GCC) $(EVALOVERHEADOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

evalmultipleshared:  $(EVALMULTIPLESHAREDOBJ) $(PARENTOBJ)
	$(GCC) $(EVALMULTIPLESHAREDOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

evalimplicit:  $(EVALIMPLICITOBJ) $(PARENTOBJ)
	$(GCC) $(EVALIMPLICITOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

.PHONY: clean
clean:
	rm -rf *o client dummyclient
