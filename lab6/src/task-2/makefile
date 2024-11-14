CC=gcc
CFLAGS=-I. -pthread -Wall -Wextra
DEPS = 
OBJ_CLIENT = client.o
OBJ_SERVER = server.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: client server

client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS)

server: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o client server