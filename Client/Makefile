GCC = g++
CFLAGS = -Wall
LIBS = -pthread -lrt

#CLIENTSRC =$(wildcard *.cpp)
#DUMMYCLIENTSRC =$(wildcard *.cpp)

CLIENTSRC = testClient.cpp tuner.cpp
DUMMYCLIENTSRC = dummyClient.cpp tuner.cpp

CLIENTOBJ =$(CLIENTSRC:.cpp=.o)  
DUMMYCLIENTOBJ =$(DUMMYCLIENTSRC:.cpp=.o)  

PARENTSRC =$(wildcard ../*.cpp)
PARENTOBJ =$(PARENTSRC:.cpp=.o) 

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

.PHONY: all
all: client dummyclient

client: $(CLIENTOBJ) $(PARENTOBJ)
	$(GCC) $(CLIENTOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

dummyclient:  $(DUMMYCLIENTOBJ) $(PARENTOBJ)
	$(GCC) $(DUMMYCLIENTOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

.PHONY: clean
clean:
	rm -rf *o client dummyclient
