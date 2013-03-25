SERVER_DIR = Server
CLIENT_DIR = Client
EVAL_DIR = Eval

GCC = g++
CFLAGS = -Wall -fPIC

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o) 

.PHONY: server
.PHONY: client

all: general server client eval

.PHONY: general
general: $(OBJ)

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

server:
	$(MAKE) -C $(SERVER_DIR)

client:
	$(MAKE) -C $(CLIENT_DIR)

eval:
	$(MAKE) -C $(EVAL_DIR)

.PHONY: clean
clean:
	$(MAKE) -C $(SERVER_DIR) clean
	$(MAKE) -C $(CLIENT_DIR) clean
	$(MAKE) -C $(EVAL_DIR) clean
	rm -rf *.o
