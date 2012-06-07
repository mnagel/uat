SERVER_DIR = Server
CLIENT_DIR = Client

GCC = g++
CFLAGS = -Wall

SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o) 

.PHONY: server
.PHONY: client

all: general server client

.PHONY: general
general: $(OBJ)

%.o : %.cpp
	$(GCC) $(CFLAGS) -c $<

server:
	$(MAKE) -C $(SERVER_DIR)

client:
	$(MAKE) -C $(CLIENT_DIR)

.PHONY: clean
clean:
	$(MAKE) -C $(SERVER_DIR) clean
	$(MAKE) -C $(CLIENT_DIR) clean
	rm -rf *o
