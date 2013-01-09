#makefile for craft
CC=gcc
BINDIR = $(prefix)/usr/sbin
MANDIR = $(prefix)/usr/share/man

all: logcraft
logcraft: logcraft.o
	${CC} -o  logcraft logcraft.o ${LIBS}  -lsqlite3 -pthread -levent
logcraft.o: logcraft.c
	${CC} -c -Wall logcraft.c
clean:
	rm -f *.o
