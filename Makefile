# Written by: Lucas Lovellette
# Written on: 09/16/2024

# Compiler
CC = gcc

# Flags
CFLAGS = -Wall -g

# Targets
TARGETS = oss worker

# Build all targets
all: $(TARGETS)

# Build oss
oss: oss.o
	$(CC) $(CFLAGS) -o oss oss.o

# Build worker
worker: worker.o
	$(CC) $(CFLAGS) -o worker worker.o

# Compile oss.o
oss.o: oss.c
	$(CC) $(CFLAGS) -c oss.c

# Compile worker.o
worker.o: worker.c
	$(CC) $(CFLAGS) -c worker.c

# Clean up object files and executables
clean:
	rm -f *.o $(TARGETS)

# Optional: Run the program
run: oss
	./oss

# Trying to make this a habit
.PHONY: all clean run
