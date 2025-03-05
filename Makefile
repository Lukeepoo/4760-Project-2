# Makefile

CC = gcc
CFLAGS = -Wall -g

all: oss worker

oss: oss.c shared_memory.c
	$(CC) $(CFLAGS) oss.c shared_memory.c -o oss

worker: worker.c shared_memory.c
	$(CC) $(CFLAGS) worker.c shared_memory.c -o worker

clean:
	rm -f oss worker