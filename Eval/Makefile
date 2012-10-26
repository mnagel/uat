GCC = g++
CFLAGS = -Wall
LIBS = -pthread -lrt

DIRS = EvalFunctional EvalImplicit EvalOverhead EvalMultipleShared EvalManyClients EvalNumIterations

#EVALIMPLICITSRC = evalImplicit.cpp tuner.cpp
#EVALIMPLICITOBJ =$(EVALIMPLICITSRC:.cpp=.o)  

PARENTSRC =$(wildcard ../*.cpp)
PARENTOBJ =$(PARENTSRC:.cpp=.o) 

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

.PHONY: all
all:
	-for d in $(DIRS); do ($(MAKE) -C $$d); done

.PHONY: clean
clean:
	-for d in $(DIRS); do ($(MAKE) -C $$d clean); done

#evalimplicit:  $(EVALIMPLICITOBJ) $(PARENTOBJ)
#	$(GCC) $(EVALIMPLICITOBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@