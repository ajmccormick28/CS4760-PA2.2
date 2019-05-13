// Programmer: Andrew McCormick
// Function:   childPro.c
// Class:      CS-4760 PA4
// Date:       4/11/2019

#include <stdlib.h>
#include <sys/ipc.h>
#include <stdio.h> 
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include "getNamed.h"

#include "sharedMemory.h"
#include "optArg.h"

#define PERM (S_IRUSR | S_IWUSR)

// Semaphore constant name
//#define SEMNAME "/Semfile"

#define BLOCKS 18

static SharedMemory *sharedMem;

/**************************************************
 *                                                *
 *            Main Function                       *
 *                                                *
 *************************************************/

int main(int argc, char *argv[])
{
    int size = sizeof(SharedMemory) + sizeof(ControlBlock) * BLOCKS;
	int shmID;
	int i = 0;
	//int semFlag = 0;

	//FILE *palptr;

	/*
	// Keeping track of time
	time_t t;
	struct tm *timeInfo;
	time(&t);
	timeInfo = localtime(&t);
	time(&t);
	srand((int)time(&t) % getpid());

	OptArg args;

	// Setting output file names
	args.palin = "palin.out";
	args.noPalin = "nopalin.out";
*/
	// Getting index from parent process input
	int simPid = atoi(argv[0]);
	//int indexCh = atoi(argv[1]);

	//sem_t *semlockp;

	// Getting Key
	key_t key = ftok("main.c", 50);
	
	if(key == -1)
	{
		perror("Failed to derive key:");
		return EXIT_FAILURE;
	}
	
	// Get attached memory
	shmID = shmget(key, size, 0666 | IPC_CREAT);

	
	// Already created, access and attach it
        if((shmID == -1) && (errno != EEXIST))
        {
                return EXIT_FAILURE;
        }
	
	// Successfully Created, must attach and initialize variables
        else
        {
                sharedMem = (SharedMemory *)shmat(shmID, NULL, 0);

                if(sharedMem == (void *)-1)
                {
                        return -1;
                }

        }


    printf("SimPid: %d\n", sharedMem -> controlTable[simPid].simPid);
    printf("Time: secs: %d, NS: %d\n", sharedMem -> seconds, sharedMem -> nanoSecs);





 	// Detach from shared memory
	shmdt(sharedMem);

	return EXIT_SUCCESS;
}
