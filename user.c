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
#include <fcntl.h>
#include <sys/stat.h>
#include "sharedTime.h"
#define PERM (S_IRUSR | S_IWUSR)

static SharedTime *sharedSum;

int main(int argc, char *argv[])
{
	int shmID;
	int durationSecs = 0;
	int terminate = 0;

	// Getting duration time from parent process input
	double durationNano = (double) strtod(argv[0], NULL);

	// Getting Key
	key_t key = ftok("main.c", 50);
	
	if(key == -1)
	{
		perror("Failed to derive key:");
		return EXIT_FAILURE;
	}
	
	// Get attched memory
	shmID = shmget(key, sizeof(SharedTime), 0666 | IPC_CREAT);

	
	// Already created, access and attach it
        if((shmID == -1) && (errno != EEXIST))
        {
                return EXIT_FAILURE;
        }
	
	// Successfully Created, mush attach and initialize variables
        else
        {
                sharedSum = (SharedTime *)shmat(shmID, NULL, 0);

                if(sharedSum == (void *)-1)
                {
                        return -1;
                }

        }
	
	// Adding duration to shared time to find the time to terminate child
	if((durationNano + sharedSum -> nanoSecs) >= 1000000000.0)
	{
		durationNano = (durationNano + sharedSum -> nanoSecs) - 1000000000.0;
		durationSecs = sharedSum -> seconds + 1;
	}
	else
	{
		durationNano = durationNano + sharedSum -> nanoSecs;
		durationSecs = sharedSum -> seconds;
	}
	
	// Loop to check when it is time to terminate child
	while(terminate != 1)
	{
		// Checking to see if the seconds are equal
		if(durationSecs == (sharedSum -> seconds))
		{
			if(durationNano <= sharedSum -> nanoSecs)
			{
				terminate = 1;
			}
		}

		// Checking to see if seconds are unequal
		else if(durationSecs < sharedSum -> seconds)
		{
			terminate = 1;
		}
	}

	// Telling user child is terminating
	printf("My Pid is: %d I'm Terminating at: %d Seconds, %f Nanoseconds\n", getpid(), sharedSum -> seconds, sharedSum -> nanoSecs);

 	// Detach from shared memory
	shmdt(sharedSum);

	return EXIT_SUCCESS;
}
