CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm

all: client1 client2 server

debug: CFLAGS += -g
debug: client1 client2 server

client1: client1.o 
	${CC} client1.o -o client1

client2: client2.o 
	${CC} client2.o -o client2

server: server.o 
	${CC} server.o  -o server  -lssl -lcrypto

client1.o: client1.c
	${CC} ${CFLAGS} client1.c 

client2.o: client2.c
	${CC} ${CFLAGS} client2.c 

server.o: server.c
	${CC} ${CFLAGS} server.c

clean:
	rm -f *.o *~