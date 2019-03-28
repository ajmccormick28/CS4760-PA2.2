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
#include "stack.h"
#define PERM (S_IRUSR | S_IWUSR)

static InputHold *inputArr;

int main(int argc, char *argv[])
{
	int shmID;
	int i = 0;
	int stringLen = 0;
	int nonSemFlag = 0;

	// Getting duration time from parent process input
	int index = atoi(argv[0]);

	// Creating an object of struct StackItem in stack.c
	StackItem lineInput;	

	// Creating the stack in stack.c
	Stack *stack = stackCreate();

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

	// If length is even
	if((stringLen % 2) == 0)
	{
		for(i = 0; i < (stringLen / 2); i++)
		{
			printf("lower: %c First: %c\n", inputArr -> input[index][i], inputArr -> input[index][stringLen - 1 - i]);

			if(inputArr -> input[index][i] != inputArr -> input[index][stringLen - 1 - i])
			{
				nonSemFlag = 1;
			}
		}
		
                if(nonSemFlag == 0)
                {
                        printf("Semphore: %s\n", inputArr -> input[index]);
                }

                else if(nonSemFlag == 1)
                {
                         printf("Non Semphore: %s\n", inputArr -> input[index]);
                }

	}

	// If length is odd
	else
	{
                for(i = 0; i < ((stringLen - 1) / 2); i++)
                {
                        if(inputArr -> input[index][i] != inputArr -> input[index][stringLen - 1 - i])
                        {
                                nonSemFlag = 1;
                        }
                }

		if(nonSemFlag == 0)
		{
			printf("Semphore: %s\n", inputArr -> input[index]);
		}
		
		else if(nonSemFlag == 1)
		{
			 printf("Non Semphore: %s\n", inputArr -> input[index]);
		}
	}	              





 	// Detach from shared memory
	shmdt(inputArr);

	return EXIT_SUCCESS;
}
