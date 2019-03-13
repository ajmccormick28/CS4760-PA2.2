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
#include "optArg.h"
#include "sharedTime.h"
#include "detachAndRemove.h"
#define PERM (S_IRUSR | S_IWUSR) 

static SharedTime *sharedSum;
static const char *optString = "ho:i:n:s:";

int main(int argc, char * argv[])
{
	FILE *readptr;
	FILE *writeptr;

	int opt = 0;
	int shmID = 0;
	int parentCounter = 0;
	int status;
	
	double timeInc = 0;

	char fileInput[100];

	pid_t childpid;

	//SharedTime *sharedSum;

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

	//printf("\n%s\n%s\n%d\n%d\n", args.outputFileName, args.inputFileName, args.numChild, args.childAtTime);

	// Opening input file and error checking
	if((readptr = fopen(args.inputFileName, "r")) == NULL)
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

	shmID = shmget(key, sizeof(SharedTime), 0666 | IPC_CREAT);
	
	if((shmID == -1) && (errno != EEXIST))
	{
		return EXIT_FAILURE;
	}

	if(shmID == -1)
	{
		if(((shmID = shmget(key, sizeof(SharedTime), PERM)) == -1) || ((sharedSum = (SharedTime *)shmat(shmID, NULL, 0)) == (void *)-1))
		{
			return EXIT_FAILURE;
		}
	}
		
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

	printf("\n%d\n%d\n", sharedSum -> seconds, sharedSum -> nanoSecs);

/*****************************************************************
 *
 * 		Creating Fork and Exec
 *
 *****************************************************************/
/*
	if((sharedSum = (SharedTime *) shmat(shmID, NULL, 0)) == (void *)-1)
	{
		perror("Failed to attach shared memeory segment");
		
		if(shmctl(shmID, IPC_RMID, NULL) == -1)
		{
			perror("Failed to remove memory segment");
		}
	
		return EXIT_FAILURE;
	}
*/
	 // Pulling first line from input file and checking if not data exists
	if(fgets(fileInput, 100, readptr) == NULL)
	{
	        perror("logParse: Error: Line 1 in the input file is empty");
		return EXIT_FAILURE;
	}

	timeInc = (int) strtol(fileInput, NULL, 10); // Converting char* into integer

	
	for(parentCounter = 0; parentCounter < args.numChild; parentCounter++)
	{
		if((timeInc + sharedSum -> nanoSecs) == 1000000000)
		{
			sharedSum -> seconds += 1;
			sharedSum -> nanoSecs = 0;
		}
		else
		{
			sharedSum -> nanoSecs += timeInc;
		}

		printf("\nSecs%d Nano%d\n", sharedSum -> seconds, sharedSum -> nanoSecs);
		if((childpid = fork()) == 0)
		{	
			printf("%d is running\n", getpid());
			execl("./user", "hello", NULL);
			//printf("\nI'm here\n");
			perror("exec Failed:");
			return EXIT_FAILURE;
		}

		else
		{
			childpid = wait(&status);
		}
		//printf("\n%d\n%d\n", sharedSum -> seconds, sharedSum -> nanoSecs);
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
	
	return EXIT_SUCCESS;
}
