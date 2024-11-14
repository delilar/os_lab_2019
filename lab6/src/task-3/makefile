CC=gcc
CFLAGS=-I. -pthread -Wall -Wextra
DEPS = common.h
OBJ_COMMON = common.o
OBJ_CLIENT = client.o
OBJ_SERVER = server.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: client server

client: $(OBJ_CLIENT) $(OBJ_COMMON)
	$(CC) -o $@ $^ $(CFLAGS)

server: $(OBJ_SERVER) $(OBJ_COMMON)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o client server