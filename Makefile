PORT=51376
CC = gcc
CFLAGS= -DPORT=\$(PORT) -g -Wall -Werror


#all: 
#	$(CC) $(CFLAGS) -o buxserver buxserver.c buxfer.c wrapsock.c lists.c

buxserver: buxserver.o buxfer.o wrapsock.o lists.o buxfer.h wrapsock.h
	$(CC) $(CFLAGS) -o buxserver buxserver.o buxfer.o wrapsock.o lists.o

buxserver.o: buxserver.c buxfer.h wrapsock.h
	$(CC) $(CFLAGS) -c buxserver.c

buxfer.o: buxfer.c buxfer.h lists.h
	$(CC) $(CFLAGS) -c buxfer.c

wrapsock.o: wrapsock.c wrapsock.h
	$(CC) $(CFLAGS) -c wrapsock.c

lists.o: lists.c lists.h
	$(CC) $(CFLAGS) -c lists.c

clean: 
	rm buxserver *.o
