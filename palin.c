// Programmer: Andrew McCormick
// Function:   palin.c
// Class:      CS-4760 PA3
// Date:       3/21/2019

#include <stdlib.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "inputHold.h"
#define PERM (S_IRUSR | S_IWUSR)

static InputHold *inputArr;

int main(int argc, char *argv[])
{
	int shmID;
	int i = 0;
	
	// Getting duration time from parent process input
	int inputArrCount = atoi(argv[0]);
	

	printf("I'm here\n");
	printf("%d\n", inputArrCount);

	// Getting Key
	key_t key = ftok("main.c", 50);
	
	if(key == -1)
	{
		perror("Failed to derive key:");
		return EXIT_FAILURE;
	}
	
	// Get attched memory
	shmID = shmget(key, sizeof(InputHold), 0666 | IPC_CREAT);

	
	// Already created, access and attach it
        if((shmID == -1) && (errno != EEXIST))
        {
                return EXIT_FAILURE;
        }
	
	// Successfully Created, mush attach and initialize variables
        else
        {
                inputArr = (InputHold *)shmat(shmID, NULL, 0);

                if(inputArr == (void *)-1)
                {
                        return -1;
                }

        }
	
        for(i = 0; i < inputArrCount; i++)
        {
                printf("%d\n", i);
                printf("%s\n", inputArr -> input[i]);
        }

	

 	// Detach from shared memory
	shmdt(inputArr);

	return EXIT_SUCCESS;
}
