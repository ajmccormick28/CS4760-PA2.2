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

#include "sharedTime.h"
#include "optArg.h"

#define PERM (S_IRUSR | S_IWUSR)

// Semaphore constant name
//#define SEMNAME "/Semfile"

#define BLOCKS 18

static SharedTime *sharedSum;

/**************************************************
 *                                                *
 *            Main Function                       *
 *                                                *
 *************************************************/

int main(int argc, char *argv[])
{
    int size = sizeof(SharedTime) + sizeof(ControlBlock) * BLOCKS;
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
	//int index = atoi(argv[0]);
	//int indexCh = atoi(argv[1]);

	//sem_t *semlockp;

	// Getting Key
	key_t key = ftok("main.c", 50);
	
	if(key == -1)
	{
		perror("Failed to derive key:");
		return EXIT_FAILURE;
	}
	
	// Get attched memory
	shmID = shmget(key, size, 0666 | IPC_CREAT);

	
	// Already created, access and attach it
        if((shmID == -1) && (errno != EEXIST))
        {
                return EXIT_FAILURE;
        }
	
	// Successfully Created, must attach and initialize variables
        else
        {
                sharedSum = (SharedTime *)shmat(shmID, NULL, 0);

                if(sharedSum == (void *)-1)
                {
                        return -1;
                }

        }
/*
	// Attaching to semaphore
        if(getNamed(SEMNAME, &semlockp, 1) == -1)
        {
                perror("Failed to create named semaphore");
                return EXIT_FAILURE;
        }
*/
/**************************************************
 *                                                *
 *  Loop to write Index Strings to Output Files   *
 *                                                *
 *************************************************/
/*
	for(i = 0; i < indexCh; i++)
	{
		//generating random number
		int randNum = (rand() % 3) + 1;

		sleep(randNum);

		// Updating clock for each loop iteration
		t = time(0);
		time(&t);
		timeInfo = localtime(&t);
		time(&t);
	
		// Trying to enter critical section
		fprintf(stderr, "Child Process #%d trying to enter the critical section to work on index #%d: %s\n", getpid(), index, asctime(timeInfo));

		while(sem_wait(semlockp) == -1)
		{	
			if(errno != EINTR)
			{
				perror("Failed to lock semlock");
				return 1;
			}
		}
		
	
		// Entered the Critcal Section
		fprintf(stderr, "Child Process #%d entered the critical section to work on index #%d: %s\n", getpid(), index, asctime(timeInfo));

		// Calling palinCheck to see if the string at index is a semaphore and setting semFlag
	        semFlag = palinCheck(index);

		// If string is a semaphore
		if(semFlag == 2)
		{
			// Opening output file
			if((palptr = fopen(args.palin, "a")) == NULL)
			{
				printf("error opening output file\n");
				//break;
			}

			sleep(2);

			// Writing to output file
			fprintf(palptr, "PID: %d Index: %d String: %s", getpid(), index, inputArr -> input[index]);
		}

		// If string is not a semaphore
		else
                {
			// Opening output file 
                        if((palptr = fopen(args.noPalin, "a")) == NULL)
                        {
                                printf("error opening output file\n");
                                //break;
                        }

                        sleep(2);

			// Writing to output file
                        fprintf(palptr, "PID: %d Index: %d String: %s", getpid(), index, inputArr -> input[index]);
                }

		sleep(2);

		// Unclocking the semaphore
        	if(sem_post(semlockp) == -1)
        	{
                	perror("Failed to unlock semlock");
                	return 1;
        	}

		fprintf(stderr, "Child Process #%d exited the critical section to work on index #%d: %s\n", getpid(), index, asctime(timeInfo));
	
		index++;
	}
*/
	// Detach from semaphore
//	sem_close(semlockp);

	// Closing output file
//	fclose(palptr);



    for(i = 0; i < BLOCKS; i++)
    {
        printf("Child: %d\n", sharedSum->block[i].a);
    }

 	// Detach from shared memory
	shmdt(sharedSum);

	return EXIT_SUCCESS;
}
