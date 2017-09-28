# Makefile
# Sistemas Operativos, DEI/IST/ULisboa 2017-18

heatSim: main.o matrix2d.o
	gcc -o heatSim main.o matrix2d.o

main.o: main.c matrix2d.h
	gcc -g -c main.c

matrix2d.o: matrix2d.c matrix2d.h
	gcc -g -c matrix2d.c

