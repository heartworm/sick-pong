CC =gcc
CFLAGS =-LZDK -IZDK -lzdk -lncurses -std=c99

all: main.o interface.o
	$(CC) interface.o main.o $(CFLAGS) -o main

main.o: 
	$(CC) $(CFLAGS) -c main.c

interface.o: 
	$(CC) $(CFLAGS) -c interface.c
	
clean:
	rm *.o main