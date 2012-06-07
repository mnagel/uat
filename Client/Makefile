GCC = g++
CFLAGS = -Wall
LIBS = -pthread

SRC =$(wildcard *.cpp)
OBJ =$(SRC:.cpp=.o)  
PARENTSRC =$(wildcard ../*.cpp)
PARENTOBJ =$(PARENTSRC:.cpp=.o) 

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

.PHONY: all
all: client

client: $(OBJ) $(PARENTOBJ)
	$(GCC) $(OBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

.PHONY: clean
clean:
	rm -rf *o client
