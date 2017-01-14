PROG = lab1
CFLAGS = -g -fPIC -m64 -Wall
LFLAGS= -fPIC -m64 -Wall
CC = gcc -std=gnu99

all: $(PROG)

lab1.o: lab1.c
	$(CC) $(CFLAGS) -c lab1.c

lab1 : lab1.o
	$(CC) $(LFLAGS) lab1.o -o lab1

run:
	./lab1

ps:
	ps -fu $$USER

clean:
	/bin/rm -f *~
	/bin/rm -f *.o
