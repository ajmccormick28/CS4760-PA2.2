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
#include <sys/msg.h>
#include "optArg.h"
#include "inputHold.h"
#include "detachAndRemove.h"
#include "getNamed.h"
#include "controlBlock.h"
#include "sharedMemory.h"

#define PERM (S_IRUSR | S_IWUSR)
#define BLOCKS 18

static SharedMemory *sharedMem;
static const char *optString = "ho:i:n:s:";
static volatile sig_atomic_t doneflag = 0;

const int maxTimeBetweenNewProcsNS = 500000;
const int maxTimeBetweenNewProcsSecs = 1;

void setupMsgQueue();

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

    int launchTime  = 1;

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
    int size = sizeof(SharedMemory) + sizeof(ControlBlock) * BLOCKS;
    int timeInc = 20000;
    int nextLaunchSecs = 0;
    int nextLaunchNS = 0;
    char fileInput[100];
    char inputArrCountSt[10];
    char indexChSt[10];

    char simPidSt[10];

    char launchChild = 'y';

    pid_t childpid;
    pid_t testpid;

   // struct sigaction act;

    //sem_t *semlockp;



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

    key_t key = ftok("main.c", 50);
    if (key == -1) {
        perror("Failed to derive key:");
        return EXIT_FAILURE;
    }

/*************************************************
 *                                               *
 *            Creating Shared Memory             *
 *                                               *
 ************************************************/

    // Get attached memory, creating it if necessary
    shmID = shmget(key, size, 0666 | IPC_CREAT);

    if ((shmID == -1) && (errno != EEXIST))
    {
        return EXIT_FAILURE;
    }

    // Already created, access and attach it
    if (shmID == -1)
    {
        if (((shmID = shmget(key, size, PERM)) == -1) || ((sharedMem = (SharedMemory *) shmat(shmID, NULL, 0)) == (void *) -1))
        {
            return EXIT_FAILURE;
        }
    }

    // Successfully Created, must attach and initialize variables
    else
    {
        sharedMem = (SharedMemory *) shmat(shmID, NULL, 0);

        if (sharedMem == (void *) -1)
        {
            return EXIT_FAILURE;
        }

        sharedMem->nanoSecs = 0;
        sharedMem->seconds = 0;
    }


    i = 0;

    do
    {
        // Incremening Shared Time
        // Checking if nanoseconds are greater than 1 second
        if((timeInc + sharedMem -> nanoSecs) >= 1000000000)
        {
            sharedMem -> seconds += 1;
            sharedMem -> nanoSecs = 0;
        }

            // If nanoseconds are not greater than one second
        else
        {
            sharedMem -> nanoSecs += timeInc;
        }

        nextLaunchNS -= timeInc;

        if(nextLaunchNS <= 0)
        {
            nextLaunchSecs -= 1;
            nextLaunchNS += 1000000000;
        }

        //printf("NextLaunchNS: %d NextLaunchSecs: %d\n", nextLaunchSecs, nextLaunchNS);

        if(nextLaunchSecs <= -1 && launchTime  == 0)
        {
            //printf("NS: %d Secs: %d\n", nextLaunchNS, nextLaunchSecs);
            sharedMem -> controlTable[i].simPid = i;

            sprintf(simPidSt, "%d", i);
            if((childpid = fork()) == 0)
            {
                // Execing child
                execl("./child", simPidSt, NULL);
                perror("exec Failed:");
                return EXIT_FAILURE;
            }

            launchTime  = 1;
        }

        if(launchTime == 1)
        {
            nextLaunchNS = rand() % maxTimeBetweenNewProcsNS + 1;
            // Need to fix so it varies more
            nextLaunchSecs = rand() % maxTimeBetweenNewProcsSecs + 1;
            launchTime  = 0;

            //printf("NextLaunchNS: %d NextLaunchSecs: %d\n", nextLaunchSecs, nextLaunchNS);
        }

        //childpid = wait(&status);

        // Checking for terminated child processes
        testpid = waitpid(-1, &status, WNOHANG);

        if(testpid > 0)
        {
            i++;
           // terminate++;
           // numChildern--;
        }


        //i++;
    }
    while(i < 2);




/*************************************************
 *                                               *
 *           Detaching Shared Memory             *
 *                                               *
 *************************************************/

    if(detachAndRemove(shmID, sharedMem) == -1)
    {
        perror("Failed to destroy shared memory segment");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

 }