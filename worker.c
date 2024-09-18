/*
* Worker program
 * Simulates work by incrementing the clock in shared memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    // Get the shared memory segment
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 2 * sizeof(int), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    int *shm_clock = (int *)shmat(shmid, (void *)0, 0);
    if (shm_clock == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Log worker start
    printf("Worker PID %d started at clock: %d seconds, %d nanoseconds\n", getpid(), shm_clock[0], shm_clock[1]);

    // Simulate some work: increment the shared clock
    for (int i = 0; i < 1000; i++) {
        shm_clock[1] += 1000; // Increment nanoseconds
        if (shm_clock[1] >= 1000000000) {
            shm_clock[0]++;     // Increment seconds
            shm_clock[1] = 0;   // Reset nanoseconds
        }
        usleep(1000); // Simulate time delay (1 millisecond)
    }

    // Log worker end
    printf("Worker PID %d finished at clock: %d seconds, %d nanoseconds\n", getpid(), shm_clock[0], shm_clock[1]);

    // Detach from shared memory
    if (shmdt(shm_clock) == -1) {
        perror("shmdt failed");
    }

    return 0;
}
