// Programmer: Andrew McCormick
// Function:   palinCheck.c
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
#include <semaphore.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/ipc.h>

#include "palinCheck.h"
#include "inputHold.h"

static InputHold *inputArr;

int palinCheck(int index)
{
	int stringLen = 0;
	int i = 0;
	int shmID;
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


	
        stringLen = strlen(inputArr -> input[index]) - 1;

	//printf("stringlen: %d\n", stringLen);
        



	//printf("I'm here\n");
	if((stringLen % 2) == 0)
        {
                
		for(i = 0; i < (stringLen / 2); i++)
                {
			
			if(inputArr -> input[index][i] != inputArr -> input[index][stringLen - 1 - i])
                        {
				shmdt(inputArr);
                                return 1;
				
                        }
			printf("I'm through\n");
                }

        }
	
        else
        {
		printf("I'm negative\n");
                for(i = 0; i < ((stringLen - 1) / 2); i++)
                {
                        if(inputArr -> input[index][i] != inputArr -> input[index][stringLen - 1 - i])
                        {
                                shmdt(inputArr);
				return 1;	
                        }
                }
        }
	 
        // Detach from shared memory
	shmdt(inputArr);
	                 
	return 2;
 

}