PROG = lab3
CFLAGS = -g -fPIC -m64 -Wall
LFLAGS= -fPIC -m64 -Wall
CC = mpicc -std=gnu99

all: $(PROG)

lab3.o: lab3.c
	$(CC) $(CFLAGS) -c lab3.c

lab3 : lab3.o
	$(CC) $(LFLAGS) lab3.o -o lab3

run:
	mpirun --hostfile hostfile -np 24 lab3

ps:
	ps -fu $$USER

clean:
	/bin/rm -f *~
	/bin/rm -f *.o

test-mpi:
	mpirun --hostfile hostfile -np 4 lab3
