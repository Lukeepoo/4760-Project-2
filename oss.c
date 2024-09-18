/*
* OSS - Main control program
 * Written by: Lucas Lovellette
 * Written on: 09/16/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    // Generate a unique key
    key_t key = ftok("shmfile", 65);

    // Create and attach shared memory segment
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

    // Initialize the simulated system clock
    shm_clock[0] = 0; // Seconds
    shm_clock[1] = 0; // Nanoseconds

    // Fork a child process to run the worker
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process: Run the worker
        execl("./worker", "worker", NULL);
        perror("execl failed");
        exit(1);
    } else {
        // Parent process: Output and wait for the child to complete
        printf("OSS: Forked worker process with PID %d\n", pid);
        wait(NULL); // Wait for the worker to complete
    }

    // Detach shared memory
    if (shmdt(shm_clock) == -1) {
        perror("shmdt failed");
    }

    // Destroy the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    }

    return 0;
}
