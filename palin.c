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
#include "inputHold.h"
#include "palinCheck.h"
#include "optArg.h"

#define PERM (S_IRUSR | S_IWUSR)

// Semaphore constant name
#define SEMNAME "/Semfile"

static InputHold *inputArr;

/**************************************************
 *                                                *
 *            Main Function                       *
 *                                                *
 *************************************************/

int main(int argc, char *argv[])
{
	int shmID;
	int i = 0;
	int semFlag = 0;

	FILE *palptr;
	
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

	// Getting index from parent process input
	int index = atoi(argv[0]);
	int indexCh = atoi(argv[1]);

	sem_t *semlockp;

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
	
	// Successfully Created, must attach and initialize variables
        else
        {
                inputArr = (InputHold *)shmat(shmID, NULL, 0);

                if(inputArr == (void *)-1)
                {
                        return -1;
                }

        }

	// Attaching to semaphore
        if(getNamed(SEMNAME, &semlockp, 1) == -1)
        {
                perror("Failed to create named semaphore");
                return EXIT_FAILURE;
        }

/**************************************************
 *                                                *
 *  Loop to write Index Strings to Output Files   *
 *                                                *
 *************************************************/

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

	// Detach from semaphore
	sem_close(semlockp);

	// Closing output file
	fclose(palptr);

 	// Detach from shared memory
	shmdt(inputArr);

	return EXIT_SUCCESS;
}
