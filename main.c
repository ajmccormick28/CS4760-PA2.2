// Programmer: Andrew McCormick 
// Function:   main.c
// Clas s:      CS-4760 PA 2
// Date:       3/21/2019

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/file.h>
#include "optArg.h"
#include "inputHold.h"
#include "detachAndRemove.h"
#include "getNamed.h"

#define PERM (S_IRUSR | S_IWUSR)
#define SEMNAME "/Semfile" 
//#define PERMS (mode_t) (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
//#define FLAGS (O_CREAT | O_EXCL)
static InputHold *inputArr;
static const char *optString = "hi:n:";
static volatile sig_atomic_t doneflag = 0;

/**************************************************
 *
 * 		Siginal Code                    
 *
 *************************************************/ 		
/*
int overTime = 0;

// ARGSUSED

static void setdoneflag(int signo)
{
	doneflag = 1;
}

static void myhandler(int s) 
{
	char aster = '*';
	struct sigaction act;
	int errsave;
	errsave = errno;
	overTime = 1;
	errno = errsave;
}

// Set up myhandler for SIGPROF

static int setupinterrupt(void)
{
	struct sigaction act;
	act.sa_handler = myhandler;
	act.sa_flags = 0;
	return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}

// Set ITIMER_PROF for 2-second intervals

static int setupitimer(void)
{
	struct itimerval value;
	value.it_interval.tv_sec = 2;
	value.it_interval.tv_usec = 0;
	value.it_value = value.it_interval;
	return (setitimer(ITIMER_PROF, &value, NULL));
}

*/
int main(int argc, char * argv[])
{
/*
	if(setupinterrupt() == -1)
	{
		perror("Failed to set up handler for SIGPROF");
		return EXIT_FAILURE;
	}

	if(setupitimer() == -1)
	{
		perror("Failed to set up the ITIMER_PROF interval timer");
		return EXIT_FAILURE;
	}
*/
	FILE *readptr;
	FILE *palptr;

	int opt = 0;
	int shmID = 0;
	int parentCounter = 0;
	int status;
	int actChild = 0;
	int getNewLaunch = 0;
	int secLaunch = 0;
	int terminate = 0;
	int numChild = 0;
	//int i = 0;
	int childLaunch = 0;
	int noLines = 0;
	int indexCh = 5;

	int inputArrCount = 1;
	int index = 0;
	int i = 0;	
	
	double timeInc = 0.0;
	double nanoLaunch = 0.0;
	
	char *duration;
	char fileInput[100];
	char *end;

	pid_t childpid;
	pid_t testpid;

	struct sigaction act;

	sem_t *semlockp;

	OptArg args = {"input.txt", "palin.out", "nopalin.out", 2};

	opt = getopt(argc, argv, optString);

	while(opt != -1)
	{
		switch(opt)
		{
			// for option -i inputFileName
			case 'i':
				args.inputFileName = optarg;
				break;
					
			// for option -s number of childern running at a time
			case 'n':
				args.numChild = atoi(optarg);
				break;

			case 'h':
				printf("Program Defaults:\n-i: input.txt,\n-n: 20 total child processes,\n");
				return EXIT_SUCCESS;
		}
		
		opt = getopt(argc, argv, optString);
	}

	// Checking if number of child processes is too large
	if(args.numChild > 20)
	{
		printf("The number of child processes cannot exceed 20");
		return EXIT_FAILURE;
	}
/*	
	// Setting up signal handler of CTRL-C
	act.sa_handler = setdoneflag;
	act.sa_flags = 0;
	if((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1))
	{
		perror("Failed to set SIGINT handler");
		return EXIT_FAILURE;
	}
*/	
	// Intalizing array to hold child pids incasee of termination
	//int pids[args.numChild];
	//pids[0] = -1;

	// Opening input file and error checking
	
	if((readptr = fopen(args.inputFileName, "r")) == NULL)
	{
		perror("Master: Error");
		return EXIT_FAILURE;
	}

	// Opening both output files to clear or create them
	
	if((palptr = fopen(args.palin, "w")) == NULL)
        {
                perror("Master: Error");
                return EXIT_FAILURE;
        }

	fclose(palptr);

	if((palptr = fopen(args.noPalin, "w")) == NULL)
        {
                perror("Master: Error");
                return EXIT_FAILURE;
        }

	fclose(palptr);

/*************************************************************
 *
 * 	              Creating Key 
 *
 *************************************************************/ 	              

	key_t key = ftok("main.c", 50);
	if(key == -1)
	{
		perror("Failed to derive key:");
		return EXIT_FAILURE;
	}

/************************************************************
 *
 * 		Creating shared Memory
 *
 ************************************************************/ 

	// Get attached memory, creating it if necessary
	shmID = shmget(key, sizeof(InputHold), 0666 | IPC_CREAT);
	
	if((shmID == -1) && (errno != EEXIST))
	{
		return EXIT_FAILURE;
	}

	// Already created, access and attach it
	if(shmID == -1)
	{
		if(((shmID = shmget(key, sizeof(InputHold), PERM)) == -1) || ((inputArr = (InputHold *)shmat(shmID, NULL, 0)) == (void *)-1))
		{
			return EXIT_FAILURE;
		}
	}

	// Successfully Created, must attach and initialize variables		
	else
	{
		inputArr = (InputHold *)shmat(shmID, NULL, 0);
		
		if(inputArr == (void *)-1)
		{
			return EXIT_FAILURE;
		}
		
		//inputArr -> seconds = 0;
		
	}


	if(getNamed(SEMNAME, &semlockp, 1) == -1)
	{
		perror("Failed to create named semaphore");
		return EXIT_FAILURE;	
	}
/*
	unsigned int value = 1;
	sem_t *semlock = sem_open(SEMNAME, FLAGS, PERMS, 1);
	
	// Checking for successful creation 
	if(semlock == SEM_FAILED)
	{
		printf("errno: %s\n", strerror(errno));
		perror("Semaphore creation failed, exiting");
		return EXIT_FAILURE;	
	}
*/
/*****************************************************************
 *
 * 		Creating Fork and Exec
 *
 *****************************************************************/
		
	 // Pulling first line from input file and checking if data exists
	if(fgets(fileInput, 100, readptr) == NULL)
	{
		perror("Master: Error: File contains no contents");
		return EXIT_FAILURE;
	}
	
	strcpy(inputArr -> input[0], fileInput);

	while(fgets(fileInput, 100, readptr) != NULL)
	{
		strcpy(inputArr -> input[inputArrCount], fileInput);
		
		inputArrCount++;
	}
/*
	printf("%d\n", inputArrCount);

	for(i = 0; i < inputArrCount; i++)
	{
		printf("%d\n", i);
		printf("%s\n", inputArr -> input[i]);
	}

*/
	for(i = 0; i < args.numChild; i++)
	{
                if(inputArrCount <= 0)
                {
                        break;
                }

                else if(inputArrCount < 5)
                {
                        indexCh = inputArrCount;
                        inputArrCount -= 5;
                }

                else
                {
                        inputArrCount -= 5;
                        indexCh = 5;
                }



		char inputArrCountSt[10];
		char indexChSt[10];
		sprintf(inputArrCountSt, "%d", index);
		sprintf(indexChSt, "%d", indexCh);
		//printf("Made a child!\n");
		//printf("%s\n", inputArrCountSt);
                if((childpid = fork()) == 0)
                {
                	execl("./palin", inputArrCountSt, indexChSt, NULL);
                        perror("exec Failed:");
                        return EXIT_FAILURE;
                }

		index += 5;
	}


/*******************************************************************
 *
 * 		Main Loop of Program 
 *
 *******************************************************************/
/*
	while(terminate < args.numChild)
	{
		// Incremening Shared Time 
		// Checking if nanoseconds are greater than 1 second
		if((timeInc + sharedSum -> nanoSecs) >= 1000000000.0)
		{
			sharedSum -> seconds += 1;
			sharedSum -> nanoSecs = 0.0;
		}
			
		// If nanoseconds are not greater than one second
		else
		{
			sharedSum -> nanoSecs += timeInc;
		}

		// Checking for kill signals 
		if(overTime == 1 || doneflag == 1)
		{
			// If no child processes have been created
			if(pids[0] == -1)
			{
				fprintf(writeptr, "Process terminated at %d Seconds, %f nanoseconds", sharedSum -> seconds, sharedSum -> nanoSecs);
				break;
			}
			
			// If child processes have been created
			else
			{
				for(i = 0; i < args.numChild; i++)
				{
					kill(pids[i], SIGKILL);
				}

				fprintf(writeptr, "Process terminated at %d Seconds, %f nanoseconds", sharedSum -> seconds, sharedSum -> nanoSecs);
				break;

			}
		}
	
		// Checking to see if anymore childern need to be created
		if(numChild > 0)
		{
			// Checking to see if the number of childern excuting is over the max
			if(actChild < args.childAtTime && actChild <= 20 && noLines == 0)
			{
				// Checking to see if a child needs to be launched
				if(secLaunch == (sharedSum -> seconds))
				{	
					// Checking to see if the shared memory nanoseconds is greater than the child process launch time
					if(nanoLaunch <=  (sharedSum -> nanoSecs))
					{ 
						if((childpid = fork()) == 0)
						{		
							execl("./user", duration, NULL);
							perror("exec Failed:");
							return EXIT_FAILURE;
						}	
						
						// Outputing the child process creation time
						fprintf(writeptr, "Child Process: %d    Started at: %d Seconds, %f NanoSeconds\n", childpid, sharedSum -> seconds, sharedSum -> nanoSecs);

						pids[childLaunch] = childpid;
						childLaunch++; // Number of childern launched
						getNewLaunch = 1; // Setting flag to get new child information
						numChild--;  // Decrementing the number of childern that need to be launched
						actChild++; // Keeping track of active child processes
					}
				}
				
				// Checking to see if the seconds are not equal
				else if(secLaunch < sharedSum -> seconds)
				{
					if((childpid = fork()) == 0)
					{
						execl("./user", duration, NULL);
						perror("exec Failed:");
						return EXIT_FAILURE;
					}
					
					// Outputting the child process creation time
 					fprintf(writeptr, "Child Process: %d    Started at: %d Seconds, %f NanoSeconds\n", childpid, sharedSum -> seconds, sharedSum -> nanoSecs);
				
                                        // See commments above
                                        pids[childLaunch] = childpid;
                                        childLaunch++;
                                        getNewLaunch = 1;
                                        numChild--;
                                        actChild++;

				}
				
				// Getting new information from the input file
				if(getNewLaunch == 1 && numChild > 0)
				{
        				if(fgets(fileInput, 100, readptr) == NULL)
        				{			
                				noLines = 1;
						numChild = 0;
						args.numChild = childLaunch;
        				}

              				secLaunch = (int) strtol(fileInput, &end, 10);
					nanoLaunch = (double) strtod(end, &end);
					duration = strtok(end, " \n\t");		
	
					getNewLaunch = 0; // Resetting flag
				}
			}
		}
		
		testpid = waitpid(-1, &status, WNOHANG); // Checking for terminated child process
		
		// Outputting terminated child process information
		if(testpid > 0)
		{
			fprintf(writeptr, "Child Process: %d Terminated at: %d Seconds, %f Nanoseconds\n", testpid, sharedSum -> seconds, sharedSum -> nanoSecs);
			terminate++;
			actChild--;
		}
	}
*/


	// Detaching from the Semaphore
	//sem_close(semlock);

	// Deleting the Semaphore
	//sem_unlink(SEMNAME);
	

	if(destroyNamed(SEMNAME, semlockp) == -1)
	{
		perror("Failed to destory named semaphore");
		return EXIT_FAILURE;
	}

/*****************************************************************
 *
 * 		Detaching Shared Memory
 *
 *****************************************************************/ 		

	while((testpid = wait(&status)) > 0);

	if(detachAndRemove(shmID, inputArr) == -1)
	{
		perror("Failed to destory shared memory segment");
		return EXIT_FAILURE;
	}
	
	// Closing file pointers	
	fclose(readptr);


	printf("I made it to the end");
	return EXIT_SUCCESS;
}
