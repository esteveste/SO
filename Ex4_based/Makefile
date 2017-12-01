# Makefile

CFLAGS= -g -Wall -pedantic
CC=gcc

heatSim: main.o matrix2d.o
	$(CC) $(CFLAGS) -pthread -o heatSim main.o matrix2d.o

main.o: main.c matrix2d.h
	$(CC) $(CFLAGS) -c main.c

matrix2d.o: matrix2d.c matrix2d.h
	$(CC) $(CFLAGS) -c matrix2d.c

clean:
	rm -f *.o heatSim

zip:
	zip Ex4.zip main.c matrix2d.c matrix2d.h Makefile

run:
	./heatSim 10 10 10 0 0 10 5 0 ola 1

