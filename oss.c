/* Updated oss.c */
#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>

#define DEFAULT_N 5
#define DEFAULT_S 2
#define DEFAULT_T 5
#define DEFAULT_I 100

int shmid;
SimClock* systemClock;
PCB processTable[MAX_PROCESSES];

void incrementClock(SimClock* clock) {
    clock->nanoseconds += 1000000; // Increment by 1ms
    if (clock->nanoseconds >= 1000000000) {
        clock->seconds++;
        clock->nanoseconds -= 1000000000;
    }
}

void checkChildren() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processTable[i].pid == pid) {
                processTable[i].occupied = false;
                printf("Process %d terminated\n", pid);
                break;
            }
        }
    }
}

int countActiveWorkers() {
    int active = 0;
    for (int j = 0; j < MAX_PROCESSES; j++) {
        if (processTable[j].occupied) {
            active++;
        }
    }
    return active;
}

int main(int argc, char* argv[]) {
    int n = DEFAULT_N, s = DEFAULT_S, t = DEFAULT_T, i = DEFAULT_I;
    int option;
    while ((option = getopt(argc, argv, "hn:s:t:i:")) != -1) {
        switch (option) {
            case 'h':
                printf("Usage: ./oss [-n num_proc] [-s simul_proc] [-t max_time] [-i interval]\n");
                return 0;
            case 'n':
                n = atoi(optarg);
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 't':
                t = atoi(optarg);
                break;
            case 'i':
                i = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Invalid option\n");
                return 1;
        }
    }

    shmid = create_shared_memory(sizeof(SimClock));
    systemClock = (SimClock*) attach_shared_memory(shmid);
    systemClock->seconds = 0;
    systemClock->nanoseconds = 0;

    for (int j = 0; j < MAX_PROCESSES; j++) {
        processTable[j].occupied = false;
    }

    int launched = 0;
    while (launched < n) {
        checkChildren();  // Check for terminated children

        int activeProcesses = countActiveWorkers();

        // Launch new process if within limit
        if (activeProcesses < s && launched < n) {
            pid_t pid = fork();
            if (pid == 0) {
                char sec[10], nano[10];
                sprintf(sec, "%d", rand() % t + 1);
                sprintf(nano, "%d", rand() % 1000000000);
                execl("./worker", "worker", sec, nano, NULL);
                perror("execl failed");
                exit(EXIT_FAILURE);
            } else if (pid > 0) {
                for (int j = 0; j < MAX_PROCESSES; j++) {
                    if (!processTable[j].occupied) {
                        processTable[j].occupied = true;
                        processTable[j].pid = pid;
                        processTable[j].startSeconds = systemClock->seconds;
                        processTable[j].startNano = systemClock->nanoseconds;
                        break;
                    }
                }
                launched++;
            }
        }

        int prevNano = systemClock->nanoseconds;
        int prevSeconds = systemClock->seconds;
        while ((systemClock->seconds == prevSeconds &&
               (systemClock->nanoseconds - prevNano) < (i * 1000000)) ||
              (systemClock->seconds == prevSeconds + 1 &&
               (systemClock->nanoseconds + (1000000000 - prevNano)) < (i * 1000000))) {
            incrementClock(systemClock);
            checkChildren();
        }
    }

    // **Wait for all remaining workers before removing shared memory**
    printf("OSS: Waiting for all workers to exit before cleaning up...\n");
    while (countActiveWorkers() > 0) {
        checkChildren();
        sleep(1);  // Give time for processes to finish
    }

    // **Now it's safe to remove shared memory**
    destroy_shared_memory(shmid);
    printf("OSS: Shared memory removed. Exiting cleanly.\n");

    return 0;
}
