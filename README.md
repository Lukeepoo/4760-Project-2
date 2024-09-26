# 4760-Project-2

## OSS and Worker Process Simulation

This project is a simulation of an operating system scheduler (oss) that manages worker processes.
The simulation uses shared memory for a simulated system clock, and each worker process runs its task while updating the shared clock.
The oss process manages the forking of worker processes and tracks their start and end times.


# Project Structure
oss.c: The main scheduling program, which forks worker processes, manages a simulated system clock in shared memory, and maintains a process table to track each worker's start and end times.

worker.c: The code for the worker process that performs its task while updating the system clock shared with oss.

Makefile: Build instructions to compile oss and worker.

README.md: This document, providing an overview and instructions for using the project.
Compilation

To compile the project, use the provided Makefile. From the project directory, simply run: make

This will compile oss.c and worker.c, creating the executables oss and worker.


# Running the Project
The oss executable controls the simulation, while worker processes are managed automatically by oss.
You can customize the behavior of oss using command-line arguments. The options available are:

Usage: ./oss [-h] [-n num_processes] [-s num_simultaneous] [-t time_limit] [-i launch_interval]

Options:

-h                     Display this help message

-n num_processes       Set the number of worker processes to fork (default: 5)

-s num_simultaneous    Set the number of simultaneous processes allowed (default: 2)

-t time_limit          Set the maximum time limit for child processes in seconds (default: 100)

-i launch_interval     Set the interval (in milliseconds) between launching child processes (default: 1000)


# Example

To run oss with 5 processes, 2 simultaneous processes, and a time limit of 60 seconds with an interval of 500 milliseconds between launching processes:

./oss -n 5 -s 2 -t 60 -i 500


# Process Table Output

During execution, oss will display a process table showing the following information for each worker process:

PID: The Process ID of the worker.
Start Time (s): The start time in seconds and nanoseconds relative to the simulated clock.

End Time (s): The end time when the worker completes its task.

Status: The status of the process (e.g., success or failure).


# Cleaning up

To clean up the project directory by removing object files and executables, run:

make clean