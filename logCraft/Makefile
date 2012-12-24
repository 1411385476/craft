#makefile for craft
CC=gcc
BINDIR = $(prefix)/usr/sbin
MANDIR = $(prefix)/usr/share/man

all: logCraft
logCraft: logCraft.o
	${CC} -o logCraft logCraft.o ${LIBS}  -lsqlite3 -pthread -levent
logCraft.o: logCraft.c
	${CC} -c logCraft.c
clean:
	rm -f *.o
