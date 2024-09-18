/*
 * OSS - Main control program
 * Written by: Lucas Lovellette
 * Written on: 09/16/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>

// Struct for process table entries
typedef struct {
    pid_t pid;
    int start_seconds;
    int start_nanoseconds;
    int end_seconds;
    int end_nanoseconds;
    int status;
} ProcessTableEntry;

// Function prototypes
void print_usage(const char *program_name);

int main(int argc, char *argv[]) {
    // Default values for command-line arguments
    int num_processes = 5;      // -n option
    int num_simultaneous = 2;   // -s option
    int time_limit = 100;       // -t option in seconds
    int launch_interval = 1000; // -i option in milliseconds

    int opt;

    // Parsing command-line arguments using getopt
    while ((opt = getopt(argc, argv, "hn:s:t:i:")) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'n':
                num_processes = atoi(optarg);
                break;
            case 's':
                num_simultaneous = atoi(optarg);
                break;
            case 't':
                time_limit = atoi(optarg);
                break;
            case 'i':
                launch_interval = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Print the parsed values for debugging
    printf("Number of processes: %d\n", num_processes);
    printf("Number of simultaneous processes: %d\n", num_simultaneous);
    printf("Time limit for child processes: %d seconds\n", time_limit);
    printf("Interval to launch children: %d milliseconds\n", launch_interval);

    // Shared memory setup
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

    // Initialize the simulated system clock
    shm_clock[0] = 0; // Seconds
    shm_clock[1] = 0; // Nanoseconds

    // Process table setup
    ProcessTableEntry process_table[num_processes];

    // Process management variables
    int active_processes = 0; // Tracks the number of active child processes
    pid_t finished_pid;

    // Launching child processes
    for (int i = 0; i < num_processes; i++) {
        if (active_processes >= num_simultaneous) {
            // Wait for any child process to finish before launching another
            finished_pid = wait(NULL);
            active_processes--;

            // Capture end time in the process table
            for (int j = 0; j < num_processes; j++) {
                if (process_table[j].pid == finished_pid) {
                    process_table[j].end_seconds = shm_clock[0];  // Record seconds
                    process_table[j].end_nanoseconds = shm_clock[1];  // Record nanoseconds
                    process_table[j].status = 1;  // Mark as completed

                    printf("OSS: Process %d finished at clock: %d seconds, %d nanoseconds\n",
                           finished_pid, process_table[j].end_seconds, process_table[j].end_nanoseconds);
                    break;
                }
            }
        }

        // Fork a new process
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
            // Parent process: Record start time and increment active process count
            process_table[i].pid = pid;
            process_table[i].start_seconds = shm_clock[0];
            process_table[i].start_nanoseconds = shm_clock[1];
            process_table[i].status = 0; // Mark as in progress

            printf("OSS: Forked worker process with PID %d at clock: %d seconds, %d nanoseconds\n",
                   pid, process_table[i].start_seconds, process_table[i].start_nanoseconds);
            active_processes++;
        }

        // Simulate time passage (launch interval)
        usleep(launch_interval * 1000); // Convert milliseconds to microseconds
    }

    // Wait for all child processes to complete
    while (active_processes > 0) {
        finished_pid = wait(NULL);
        active_processes--;

        // Capture end time in the process table
        for (int i = 0; i < num_processes; i++) {
            if (process_table[i].pid == finished_pid) {
                process_table[i].end_seconds = shm_clock[0];
                process_table[i].end_nanoseconds = shm_clock[1];
                process_table[i].status = 1; // Mark as completed

                printf("OSS: Process %d finished at clock: %d seconds, %d nanoseconds\n",
                       finished_pid, process_table[i].end_seconds, process_table[i].end_nanoseconds);
                break;
            }
        }
    }

    // Print the process table for verification
    printf("\nProcess Table:\n");
    printf("%-10s %-20s %-20s %-10s\n", "PID", "Start Time (s:nanoseconds)", "End Time (s:nanoseconds)", "Status");

    for (int i = 0; i < num_processes; i++) {
        printf("%-10d %d:%-15d %d:%-15d %-10d\n",
               process_table[i].pid,
               process_table[i].start_seconds, process_table[i].start_nanoseconds,
               process_table[i].end_seconds, process_table[i].end_nanoseconds,
               process_table[i].status);
    }

    // Detach and destroy shared memory
    if (shmdt(shm_clock) == -1) {
        perror("shmdt failed");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    }

    return 0;
}

// Function to print usage instructions
void print_usage(const char *program_name) {
    printf("Usage: %s [-h] [-n num_processes] [-s num_simultaneous] [-t time_limit] [-i launch_interval]\n", program_name);
    printf("  -h                 Display this help message\n");
    printf("  -n num_processes   Set the number of processes (default: 5)\n");
    printf("  -s num_simultaneous Set the number of simultaneous processes (default: 2)\n");
    printf("  -t time_limit      Set the time limit for child processes in seconds (default: 100)\n");
    printf("  -i launch_interval Set the interval to launch children in milliseconds (default: 1000)\n");
}
