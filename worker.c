/*
* Worker - Child process program
 * Written by: Lucas Lovellette
 * Written on: 09/16/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    // Generate a unique key
    key_t key = ftok("shmfile", 65);

    // Get the shared memory segment
    int shmid = shmget(key, 2 * sizeof(int), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory segment
    int *shm_clock = (int *)shmat(shmid, (void *)0, 0);
    if (shm_clock == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Access and print the simulated system clock
    printf("Worker: Simulated Clock - Seconds: %d, Nanoseconds: %d\n", shm_clock[0], shm_clock[1]);

    // Detach shared memory
    if (shmdt(shm_clock) == -1) {
        perror("shmdt failed");
    }

    return 0;
}
