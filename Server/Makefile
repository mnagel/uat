GCC = g++
CFLAGS = -Wall
LIBS = -pthread -lrt

SRC =$(wildcard *.cpp)
OBJ =$(SRC:.cpp=.o)  
PARENTSRC =$(wildcard ../*.cpp)
PARENTOBJ =$(PARENTSRC:.cpp=.o) 

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

.PHONY: all
all: server

server: $(OBJ) $(PARENTOBJ)
	$(GCC) $(OBJ) $(PARENTOBJ) $(CFLAGS) $(LIBS) -o $@

.PHONY: clean
clean:
	rm -rf *.o server
