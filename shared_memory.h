/* shared_memory.h */
#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdbool.h>

#define SHM_KEY 1234 // Key for shared memory
#define MAX_PROCESSES 20 // Maximum number of processes

// Simulated system clock
typedef struct {
    int seconds;
    int nanoseconds;
} SimClock;

// Process Control Block
typedef struct {
    bool occupied;
    pid_t pid;
    int startSeconds;
    int startNano;
} PCB;

// Function prototypes for shared memory management
int create_shared_memory(size_t size);
void* attach_shared_memory(int shmid);
void detach_shared_memory(void* shmaddr);
void destroy_shared_memory(int shmid);

#endif