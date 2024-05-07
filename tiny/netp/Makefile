CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.
LIB = -lpthread

all: echoserveri.o echoclient.o test.o

csapp.o: ../csapp.c ../csapp.h
	$(CC) $(CFLAGS) -c ../csapp.c

echoserveri.o: echoserveri.c csapp.o
	$(CC) $(CFLAGS) -g -o echoserveri echoserveri.c csapp.o $(LIB)

echoclient.o: echoclient.c csapp.o
	$(CC) $(CFLAGS) -g -o echoclient echoclient.c csapp.o $(LIB)

test.o: test.c
	$(CC) -g test.c -o test 

clean:
	rm -f *.o ../csapp.o ./echoclient ./echoserveri ./test

