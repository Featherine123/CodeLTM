CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all:  server

debug: CFLAGS += -g
debug:  server

#client: client.o 
#	${CC} client.o -o client

server: server.o 
	${CC} server.o  -o server

#client.o: client.c
#	${CC} ${CFLAGS} client.c

#server.o: server.c
#	${CC} ${CFLAGS} server.c

clean:
	rm -f *.o *~
