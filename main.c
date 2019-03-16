// Programmer: Andrew McCormick 
// Function:   main.c
// Class:      CS-4760 PA 1
// Date:       2/7/2019

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
#include "optArg.h"
#include "sharedTime.h"
#include "detachAndRemove.h"
#define PERM (S_IRUSR | S_IWUSR) 

static SharedTime *sharedSum;
static const char *optString = "ho:i:n:s:";
static volatile sig_atomic_t doneflag = 0;

/**************************************************
 *
 * 		Siginal Code                    
 *
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

// Set ITIMER_PROF for 2-second intervals

static int setupitimer(void)
{
	struct itimerval value;
	value.it_interval.tv_sec = 2;
	value.it_interval.tv_usec = 0;
	value.it_value = value.it_interval;
	return (setitimer(ITIMER_PROF, &value, NULL));
}


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
	FILE *writeptr;

	int opt = 0;
	int shmID = 0;
	int parentCounter = 0;
	int status;
	int actChild = 0;
	int getNewLaunch = 0;
	int secLaunch = 0;
	int terminate = 0;
	int numChild = 0;
	int i = 0;
	int childLaunch = 0;
	int noLines = 0;
		
	double timeInc = 0.0;
	double nanoLaunch = 0.0;
	
	char *duration;
	char fileInput[100];
	char *end;

	pid_t childpid;
	pid_t testpid;

	struct sigaction act;

	OptArg args = {"input.txt", "output.txt", 4, 2};

	opt = getopt(argc, argv, optString);

	while(opt != -1)
	{
		switch(opt)
		{
			// for option -o outputfile
			case 'o':
				args.outputFileName = optarg;
				break;

			// for option -i inputfile
			case 'i':
				args.inputFileName = optarg;
				break;
					
			// for option -n number of child processes
			case 'n':
				args.numChild = atoi(optarg);
				break;
			
			// for option -s number of childern running at a time
			case 's':
				args.childAtTime = atoi(optarg);
				break;

			case 'h':
				printf("Program Defaults:\n-i: input.txt,\n-o output.txt,\n-n: 4 total child processes,\n-s: Max of 2 childs running at a time\n");
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
		perror("oss: Error");
		return EXIT_FAILURE;
	}

	// Opening output file and error checking
	if((writeptr = fopen(args.outputFileName, "w")) == NULL)
	{
		perror("oss: Error");
		return EXIT_FAILURE;
	}

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
	shmID = shmget(key, sizeof(SharedTime), 0666 | IPC_CREAT);
	
	if((shmID == -1) && (errno != EEXIST))
	{
		return EXIT_FAILURE;
	}

	// Already created, access and attach it
	if(shmID == -1)
	{
		if(((shmID = shmget(key, sizeof(SharedTime), PERM)) == -1) || ((sharedSum = (SharedTime *)shmat(shmID, NULL, 0)) == (void *)-1))
		{
			return EXIT_FAILURE;
		}
	}

	// Successfully Created, must attach and initialize variables		
	else
	{
		sharedSum = (SharedTime *)shmat(shmID, NULL, 0);
		
		if(sharedSum == (void *)-1)
		{
			return EXIT_FAILURE;
		}
		
		sharedSum -> seconds = 0;
		sharedSum -> nanoSecs = 0;
	}

/*****************************************************************
 *
 * 		Creating Fork and Exec
 *
 *****************************************************************/
		
	 // Pulling first line from input file and checking if data exists
	if(fgets(fileInput, 100, readptr) == NULL)
	{
	        perror("logParse: Error: Line 1 in the input file is empty");
		return EXIT_FAILURE;
	}

	timeInc = (double) strtod(fileInput, NULL); // Converting char* into integer

	// Pulling second line from input file and checking if data exists
	if(fgets(fileInput, 100, readptr) == NULL)
	{
		perror("oss: Error: Line 2 in the input file is empty");
		return EXIT_FAILURE;
	}

	// Setting the seconds, nanoseconds, and duration variables from input file data
	secLaunch = (int) strtol(fileInput, &end, 10);
	nanoLaunch = (double) strtod(end, &end);
	duration = strtok(end, " \n\t");
	numChild = args.numChild;

/*******************************************************************
 *
 * 		Main Loop of Program 
 *
 *******************************************************************/

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

/*****************************************************************
 *
 * 		Detaching Shared Memory
 *
 *****************************************************************/ 		

	if(detachAndRemove(shmID, sharedSum) == -1)
	{
		perror("Failed to destory shared memory segment");
		return EXIT_FAILURE;
	}
	
	// Closing file pointers	
	fclose(readptr);
	fclose(writeptr);

	return EXIT_SUCCESS;
}
