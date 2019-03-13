// Programmer: Andrew McCormick
// Function:   user.c
// Class:      CS-4760 PA2
// Date:       3/5/2019

#include <stdlib.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include "sharedTime.h"

static SharedTime *sharedSum;

int main(int argc, char *argv[])
{
	int shmID;

	key_t key = ftok("main.c", 50);
	
	if(key == -1)
	{
		perror("Failed to derive key:");
		return EXIT_FAILURE;
	}

	shmID = shmget(key, sizeof(SharedTime), PERM | IPC_CREAT | IPC_EXCL);

	
        if((shmID == -1) && (errno != EEXIST))
        {
                return EXIT_FAILURE;
        }
        else
        {
                sharedSum = (SharedTime *)shmat(shmID, NULL, 0);

                if(sharedSum == (void *)-1)
                {
                        return EXIT_FAILURE;
                }

        }

	printf("\nchildSec:%d\nchildnano:%d\n", sharedSum -> seconds, sharedSum -> nanoSecs);

	// Detach from shared memory
	shmdt(shareSum);

	return EXIT_SUCCESS;
}
