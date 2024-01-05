CC = gcc
CFLAGS = -Wall

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client client.c -lssl -lcrypto

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lssl -lcrypto -pthread

clean:
	rm -f client server
