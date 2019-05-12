// Programmer: Andrew McCormick
// Function:   main.c
// Class:      CS-4760 PA 4
// Date:       4/11/2019

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
#include "controlBlock.h"
#include "sharedTime.h"

#define PERM (S_IRUSR | S_IWUSR)
#define BLOCKS 18

static SharedTime *sharedSum;
static const char *optString = "ho:i:n:s:";
static volatile sig_atomic_t doneflag = 0;

/*
  **************************************************
  *                                                *
  *                 Main Code                      *
  *                                                *
  **************************************************
*/

int main(int argc, char * argv[]) {
    /*   if(setupinterrupt() == -1)
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
//    FILE *readptr;
//    FILE *palptr;

    int opt = 0;
    int shmTimeID = 0;
    int shmBlockID = 0;
    int status;
    int terminate = 0;
    int numChildern = 0;
    int childLaunch = 0;
    int indexCh = 5;
    int inputArrCount = 1;
    int index = 0;
    int i = 0;
    int size = sizeof(SharedTime) + sizeof(ControlBlock) * BLOCKS;

    char fileInput[100];
    char inputArrCountSt[10];
    char indexChSt[10];

    char launchChild = 'y';

//    pid_t childpid;
//    pid_t testpid;

   // struct sigaction act;

    //sem_t *semlockp;

    sharedSum->nanoSecs = 0;
    sharedSum->seconds = 0;

    OptArg args = {"input.txt", "palin.out", "nopalin.out", 2};

    opt = getopt(argc, argv, optString);

    while (opt != -1) {
        switch (opt) {
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

/*
  **************************************************
  *                                                *
  *                Creating Key                    *
  *                                                *
  **************************************************
*/

    key_t keyTime = ftok("main.c", 50);
    if (keyTime == -1) {
        perror("Failed to derive key:");
        return EXIT_FAILURE;
    }

/*************************************************
 *                                               *
 *            Creating Shared Memory             *
 *                                               *
 ************************************************/

    // Get attached memory, creating it if necessary
    shmTimeID = shmget(keyTime, size, 0666 | IPC_CREAT);

    if ((shmTimeID == -1) && (errno != EEXIST))
    {
        return EXIT_FAILURE;
    }

    // Already created, access and attach it
    if (shmTimeID == -1)
    {
        if (((shmTimeID = shmget(keyTime, size, PERM)) == -1) || ((sharedSum = (SharedTime *) shmat(shmTimeID, NULL, 0)) == (void *) -1))
        {
            return EXIT_FAILURE;
        }
    }

    // Successfully Created, must attach and initialize variables
    else
    {
        sharedSum = (SharedTime *) shmat(shmTimeID, NULL, 0);

        if (sharedSum == (void *) -1)
        {
            return EXIT_FAILURE;
        }
    }

    for(i = 0; i < BLOCKS; i++)
    {
        sharedSum->block[i].a = i;
    }

    if((childpid = fork()) == 0)
    {
        // Execing child
        execl("./child", "9", NULL);
        perror("exec Failed:");
        return EXIT_FAILURE;
    }


    childpid = wait(&status);


/*************************************************
 *                                               *
 *           Detaching Shared Memory             *
 *                                               *
 *************************************************/

    if(detachAndRemove(shmTimeID, sharedSum) == -1)
    {
        perror("Failed to destroy shared memory segment");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

 }