# Define required macros here
SHELL = /bin/sh

OBJ =  main.o server.o

CFLAG = -Wall -g
CC = gcc
INCLUDE =
LIBS = -lm

serv.exe:${OBJ}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ $^ ${LIBS}

main.o:server.h
server.o:server.h

clean:
	-rm -f *.o core *.core
