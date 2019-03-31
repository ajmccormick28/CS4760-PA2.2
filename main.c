// Programmer: Andrew McCormick 
// Function:   main.c
// Class:      CS-4760 PA 3
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

static InputHold *inputArr;
static const char *optString = "hi:n:";
static volatile sig_atomic_t doneflag = 0;

/**************************************************
 *                                                *
 * 		Siginal Code                      *
 *                                                *
 *************************************************/ 		

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

// Set ITIMER_PROF for 2-minute interval

static int setupitimer(void)
{
	struct itimerval value;
	value.it_interval.tv_sec = 120;
	value.it_interval.tv_usec = 0;
	value.it_value = value.it_interval;
	return (setitimer(ITIMER_PROF, &value, NULL));
}

/**************************************************
 *                                                *
 *                 Main Code                      *
 *                                                *
 *************************************************/

int main(int argc, char * argv[])
{
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

	FILE *readptr;
	FILE *palptr;

	int opt = 0;
	int shmID = 0;
	int status;
	int terminate = 0;
	int numChildern = 0;
	int childLaunch = 0;
	int indexCh = 5;
	int inputArrCount = 1;
	int index = 0;
	int i = 0;	
	
	char fileInput[100];
	char inputArrCountSt[10];
	char indexChSt[10];

	char launchChild = 'y';
	
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
				printf("Program Defaults:\n-i: input.txt,\n-n: 2 total child processes,\n");
				return EXIT_SUCCESS;
		}
		
		opt = getopt(argc, argv, optString);
	}
	
	// Setting up signal handler of CTRL-C
	act.sa_handler = setdoneflag;
	act.sa_flags = 0;
	if((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGINT, &act, NULL) == -1))
	{
		perror("Failed to set SIGINT handler");
		return EXIT_FAILURE;
	}
	
	// Intalizing array to hold child pids incasee of termination
	int pids[args.numChild];
	pids[0] = -1;

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

/*************************************************
 *                                               *
 *                Creating Key                   *
 *                                               *
 ************************************************/ 	              

	key_t key = ftok("main.c", 50);
	if(key == -1)
	{
		perror("Failed to derive key:");
		return EXIT_FAILURE;
	}

/*************************************************
 *                                               *
 *            Creating Shared Memory             *
 *                                               *
 ************************************************/

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
	}

/*************************************************
 *                                               *
 *               Creating Semaphore              *
 *                                               *
 ************************************************/

	if(getNamed(SEMNAME, &semlockp, 1) == -1)
	{
		perror("Failed to create named semaphore");
		return EXIT_FAILURE;	
	}

/*************************************************
 *                                               *
 *        Pulling from Input File                *
 *                                               *
 ************************************************/

        // Pulling first line from input file and checking if data exists
	if(fgets(fileInput, 100, readptr) == NULL)
	{
		perror("Master: Error: File contains no contents");
		return EXIT_FAILURE;
	}
	
	// Copying first line to shared memory
	strcpy(inputArr -> input[0], fileInput);

	// Pulling rest of lines from input file and copying to shared memory
	while(fgets(fileInput, 100, readptr) != NULL)
	{
		strcpy(inputArr -> input[inputArrCount], fileInput);
		
		inputArrCount++;
	}

/*************************************************
 *                                               *
 *    Main Loop to Fork and Exec Childern        *
 *                                               *
 ************************************************/

	do
	{
		// Checking for kill signal caused by Ctrl-C
		if(doneflag == 1)
                {
			// If no child processes has been created
                	if(pids[0] == -1)
                        {
                                fprintf(stderr, "Process terminated by Control C\n");

                        	break;
                        }
	
			// If child processes have been created
                        else
                        {
                                for(i = 0; i < numChildern; i++)
                                {
                                	kill(pids[i], SIGKILL);
                                }

                                fprintf(stderr, "Process terminated by Control C\n");

	                        break;
	                }
                }
		
		// Checking for kill signal caused by going over 2 mins
                if(overTime == 1)
                {
			// If no child processes have been created
                        if(pids[0] == -1)
                        {
                                fprintf(stderr, "Process terminated because it took longer than 2 mins\n");

                                break;
                        }

			// If child processes have been created
                        else
                        {
                                for(i = 0; i < numChildern; i++)
                                {
                                        kill(pids[i], SIGKILL);
                                }

                                fprintf(stderr, "Process terminated because it took longer than 2 mins\n");

                                break;
                        }
                }

		// Checking if a child process needs to be launched
		if(numChildern <= 20 && childLaunch < args.numChild && launchChild != 'n')
		{
			// Checking if all indexes have been divided up
			if(inputArrCount <= 0)
                	{
				launchChild = 'n';
                	}

			// Checking if less then 5 indexes need to be sent to child
                	else if(inputArrCount < 5)
                	{
                        	indexCh = inputArrCount;
                      		inputArrCount -= 5;
			}

			// Next child will get 5 indexes
                	else
                	{
                	       	inputArrCount -= 5;
                	       	indexCh = 5;	
        	        }
	
			// If indexes are still avialable, child process is forked
			if(launchChild != 'n')
			{
				// Casting int values to string values to send through exce
				sprintf(inputArrCountSt, "%d", index);
				sprintf(indexChSt, "%d", indexCh);
		
				// Forking child
        	        	if((childpid = fork()) == 0)
	                	{	
					// Execing child
        	        		execl("./palin", inputArrCountSt, indexChSt, NULL);
	                    		perror("exec Failed:");
                        		return EXIT_FAILURE;
                		}
				
				// Saving childpid
                        	pids[childLaunch] = childpid;

                        	childLaunch++;
                        	numChildern++;
				index += 5;
			}	
		}

		// Checking for terminated child processes
		testpid = waitpid(-1, &status, WNOHANG);
		
                if(testpid > 0)
                {       
                        terminate++;
                        numChildern--;
                }
	}
	while(terminate < childLaunch);

/*************************************************
 *                                               *
 *      Detaching and Destorying Semaphore       *
 *                                               *
 ************************************************/

  
	if(destroyNamed(SEMNAME, semlockp) == -1)
	{
		perror("Failed to destory named semaphore");
		return EXIT_FAILURE;
	}

/*************************************************
 *                                               *
 *           Detaching Shared Memory             *
 *                                               *
 *************************************************/

	if(detachAndRemove(shmID, inputArr) == -1)
	{
		perror("Failed to destory shared memory segment");
		return EXIT_FAILURE;
	}
	
	// Closing file pointers	
	fclose(readptr);

	return EXIT_SUCCESS;
}
