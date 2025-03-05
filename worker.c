/* Updated worker.c */
#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <seconds> <nanoseconds>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int maxSeconds = atoi(argv[1]);
    int maxNano = atoi(argv[2]);

    int shmid = shmget(SHM_KEY, sizeof(SimClock), 0666);
    if (shmid == -1) {
        perror("worker: shmget failed (shared memory may have been removed)");
        exit(EXIT_FAILURE);
    }

    SimClock* systemClock = (SimClock*) shmat(shmid, NULL, 0);
    if (systemClock == (void*)-1) {
        perror("worker: shmat failed");
        exit(EXIT_FAILURE);
    }

    int termSeconds = systemClock->seconds + maxSeconds;
    int termNano = systemClock->nanoseconds + maxNano;
    if (termNano >= 1000000000) {
        termSeconds++;
        termNano -= 1000000000;
    }

    printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d -- Just Starting\n",
            getpid(), getppid(), systemClock->seconds, systemClock->nanoseconds, termSeconds, termNano);

    int lastNano = systemClock->nanoseconds;
    int lastSeconds = systemClock->seconds;

    // **Ensure termination**
    int maxWaitLoops = 1000000; // Prevent infinite looping
    int loopCount = 0;

    while (systemClock->seconds < termSeconds ||
          (systemClock->seconds == termSeconds && systemClock->nanoseconds < termNano)) {

        if (lastSeconds != systemClock->seconds) {
            printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d -- %d seconds have passed since starting\n",
                getpid(), getppid(), systemClock->seconds, systemClock->nanoseconds, termSeconds, termNano,
                systemClock->seconds - (termSeconds - maxSeconds));
        }

        while (lastSeconds == systemClock->seconds && lastNano == systemClock->nanoseconds) {
            loopCount++;
            if (loopCount > maxWaitLoops) {
                printf("WORKER PID:%d stuck in loop, force exiting!\n", getpid());
                break;
            }
        }

        lastSeconds = systemClock->seconds;
        lastNano = systemClock->nanoseconds;

        // **Break out of infinite loop if needed**
        if (loopCount > maxWaitLoops) {
            printf("WORKER PID:%d detected infinite loop, force terminating.\n", getpid());
            break;
        }
    }

    printf("WORKER PID:%d PPID:%d SysClockS: %d SysClockNano: %d TermTimeS: %d TermTimeNano: %d -- Terminating\n",
            getpid(), getppid(), systemClock->seconds, systemClock->nanoseconds, termSeconds, termNano);

    shmdt(systemClock);
    return 0;
}
