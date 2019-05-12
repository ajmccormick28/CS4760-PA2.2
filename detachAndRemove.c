// Programmer: Andrew McCormick
// Function:   detachAndRemove.c
// Class:      CS-4760 PA4
// Date:       4/11/2019

#include <stdio.h>
#include <errno.h>
#include <sys/shm.h>
#include "detachAndRemove.h"

int detachAndRemove(int shmid, void *shmaddr) 
{
	int error = 0;

	if(shmdt(shmaddr) == -1)
    {
		error = errno;
	}

	if((shmctl(shmid, IPC_RMID, NULL) == -1) && !error)
	{
		error = errno;
	}

	if(!error)
	{
		return 0;
	}

	errno = error;
	
	return -1;
}
