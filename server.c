#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include "include/constants.h"

int main(int argc, char *argv[]) {
    // declaration of variables
    double *sharedmem;
    int shm_fd;
    sem_t *sem;
    FILE *file;
    char slog[100];
    double position[6];

    // Open the POSIX shared memory object
    shm_fd = shm_open(SHMPATH, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }


    // Map the shared memory object into the program's address space
    sharedmem = mmap(NULL, 6 * sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedmem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // open a semaphore for shared memory access
    sem = sem_open(SEMPATH, O_RDWR, 0666); 
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }  
    sem_post(sem);

    // open the semaphore for logging
    sem_t *LOGsem = sem_open(LOGSEMPATH, O_RDWR, 0666); // Initial value is 1
    if (LOGsem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_post(LOGsem);


    // Infinite loop for printing
    while (1) {
     
        sem_wait(LOGsem);

        file = fopen(LOGPATH, "a");
        // Check if the file was opened successfully
        if (file == NULL) {
            fprintf(stderr, "Error opening the file.\n");
            exit(EXIT_FAILURE);
        }

        sem_wait(sem); // Wait for the semaphore
        sprintf(slog, "[server] position: %lf, %lf", sharedmem[0], sharedmem[1]);
        sem_post(sem); // Release the semaphore

        // Write the string to the file
        fprintf(file, "%s\n", slog);
        // Close the file
        fclose(file);
        sem_post(LOGsem);

        usleep(100000); // Sleep for 100 milliseconds
    }

    // Cleanup
    munmap(sharedmem, 6 * sizeof(double));
    close(shm_fd);
    sem_close(sem);
    sem_unlink(SEMPATH);


    return 0;
}
