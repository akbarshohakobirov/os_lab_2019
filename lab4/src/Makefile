CC=gcc
CFLAGS=-I.
all : parallel_min_max zombie process_memory psum

psum : Summa.o utils.o Summa.h utils.h
	$(CC) -o psum utils.o Summa.o parallel_sum.c -lpthread $(CFLAGS)

process_memory : process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

zombie : zombie.c
	$(CC) -o zombie zombie.c $(CFLAGS)

parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)
	
Summa.o : Summa.h
	$(CC) -o Summa.o -c Summa.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

clean :
	rm utils.o find_min_max.o Summa.o parallel_min_max zombie process_memory psum