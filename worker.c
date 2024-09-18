/*
* Worker - Child process program
 * Written by: Lucas Lovellette
 * Written on: 09/16/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main() {
    // Generate a unique key
    key_t key = ftok("shmfile", 65);

    // Get the shared memory segment
    int shmid = shmget(key, 2 * sizeof(int), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach to the shared memory segment
    int *shm_clock = (int *)shmat(shmid, (void *)0, 0);
    if (shm_clock == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Simulate work
    for (int i = 0; i < 100; i++) {
        shm_clock[1] += 1000; // Increment nanoseconds
        if (shm_clock[1] >= 1000000000) {
            shm_clock[0] += 1; // Increment seconds
            shm_clock[1] -= 1000000000;
        }
        printf("Worker: Simulated Clock - Seconds: %d, Nanoseconds: %d\n", shm_clock[0], shm_clock[1]);
        usleep(10000); // Sleep to simulate work (10 milliseconds)
    }

    // Detach from shared memory
    if (shmdt(shm_clock) == -1) {
        perror("shmdt failed");
        exit(1);
    }

    return 0;
}
