
CFLAGS= -g -Wall -pedantic 
CC=gcc

all: client_server_1

client_server_1: client_server_1.o  mplib3.o leQueue.o leQueue.h mplib3.h
	$(CC) $(CFLAGS) -pthread -o client_server_1 client_server_1.o mplib3.o leQueue.o

client_server_1.o: client_server_1.c mplib3.h
	$(CC) $(CFLAGS) -c client_server_1.c 


mplib_aulateorica.o: mplib3.c mplib3.h 
	$(CC) $(CFLAGS) -c mplib3.c

leQueue.o: leQueue.c leQueue.h
	$(CC) $(CFLAGS) -c leQueue.c

clean:
	rm -f *.o client_server_1
