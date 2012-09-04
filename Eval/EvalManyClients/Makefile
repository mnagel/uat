GCC = g++
CFLAGS =-Wall
LIBS =-pthread -lrt
INCLUDE_DIR =../../Client
LIB_DIR =../../Client

SRC =$(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)  

%.o : %.cpp
	$(GCC) $(CFLAGS) -I$(INCLUDE_DIR) -c $<

.PHONY: all
all: evalmanyclients

evalmanyclients: $(OBJ)
	$(GCC) $(OBJ) $(CFLAGS) -static -L$(LIB_DIR) -ltuner $(LIBS) -o $@


.PHONY: clean
clean:
	rm -rf *o evalmanyclients
