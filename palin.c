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
#define SEMNAME "/Semfile"

static InputHold *inputArr;

int main(int argc, char *argv[])
{
	int shmID;
	int i = 0;
	//int stringLen = 0;
	int semFlag = 0;

	FILE *palptr;

	time_t t;
	struct tm *timeInfo;
	time(&t);
	timeInfo = localtime(&t);
	time(&t);
	srand((int)time(&t) % getpid());

	OptArg args;

	//printf("I;m a child!\n");

	// Getting index from parent process input
	int index = atoi(argv[0]);

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

        if(getNamed(SEMNAME, &semlockp, 1) == -1)
        {
                perror("Failed to create named semaphore");
                return EXIT_FAILURE;
        }

	//stringLen = strlen(inputArr -> input[index]) - 1;

//	printf("stringlen: %d\n", stringLen);
	// For loop to run 5 indicies 

	//printf("I'm here\n");

	semFlag = palinCheck(index);

	
	printf("semFlag: %d\n", semFlag);
	//printf("I'm here\n");
//	for(i = 0; i < 5; i++)
//	{
		//generating random number
		int randNum = (rand() % 3) + 1;

		sleep(randNum);

		t = time(0);
		time(&t);
		timeInfo = localtime(&t);
		time(&t);
	
		printf("Child Process #%d trying to enter the critical section: %s\n", getpid(), asctime(timeInfo));

		while(sem_wait(semlockp) == -1)
		{	
			if(errno != EINTR)
			{
				perror("Failed to lock semlock");
				return 1;
			}
		}

		printf("Child Process #%d entered the critical section: %s\n", getpid(), asctime(timeInfo));

		if(semFlag == 2)
		{
			if((palptr = fopen(args.palin, "w")) == NULL)
			{
				printf("error opening output file\n");
				//break;
			}

			sleep(2);

			printf("PID: %d Index: %d String: %s", getpid(), index, inputArr -> input[index]);
		}

		else
                {
                        if((palptr = fopen(args.noPalin, "w")) == NULL)
                        {
                                printf("error opening output file\n");
                                //break;
                        }

                        sleep(2);

                        printf("PID: %d Index: %d String: %s", getpid(), index, inputArr -> input[index]);
                }

		sleep(2);

/*
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
                        printf("Palin: %s\n", inputArr -> input[index]);
                }

                else if(nonSemFlag == 1)
                {
                         printf("Non Palin: %s\n", inputArr -> input[index]);
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
			printf("Palin: %s\n", inputArr -> input[index]);
		}
		
		else if(nonSemFlag == 1)
		{
			 printf("Non Palin: %s\n", inputArr -> input[index]);
		}
	}	              
*/
        	if(sem_post(semlockp) == -1)
        	{
                	perror("Failed to unlock semlock");
                	return 1;
        	}

		printf("Child Process #%d exited the critical section: %s\n", getpid(), asctime(timeInfo));
	
//	}

	sem_close(semlockp);
	
	fclose(palptr);
 	// Detach from shared memory
	shmdt(inputArr);

	printf("I made it to the end of child\n");

	return EXIT_SUCCESS;
}
