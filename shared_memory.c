/* shared_memory.c */
#include "shared_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int create_shared_memory(size_t size) {
    int shmid = shmget(SHM_KEY, size, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

void* attach_shared_memory(int shmid) {
    void* shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void*)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    return shmaddr;
}

void detach_shared_memory(void* shmaddr) {
    if (shmdt(shmaddr) == -1) {
        perror("shmdt failed");
    }
}

void destroy_shared_memory(int shmid) {
    struct shmid_ds shminfo;

    // Get shared memory info to check permissions
    if (shmctl(shmid, IPC_STAT, &shminfo) == -1) {
        perror("shmctl IPC_STAT failed (checking ownership)");
        return;  // Do not attempt to remove if we can't access metadata
    }

    // Check if the current process is the owner of the shared memory
    if (shminfo.shm_perm.cuid != getuid()) {
        printf("Skipping shared memory removal: Not the owner.\n");
        return;
    }

    // Now attempt to remove shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed: Operation not permitted");
    } else {
        printf("Shared memory successfully removed.\n");
    }
}
