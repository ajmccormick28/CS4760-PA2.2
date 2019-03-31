// Programmer: Andrew McCormick
// Function:   getNamed.c
// Class:      CS-4760 PA 3
// Date:       3/21/2019

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include "getNamed.h"
#define PERMS (mode_t) (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)	
#define FLAGS (O_CREAT | O_EXCL)

/**************************************************
 *                                                *
 *    Creating Name Semaphore or Getting Name     *
 *                                                *
 *************************************************/

int getNamed(char *name, sem_t **sem, int val)
{
	while(((*sem = sem_open(name, FLAGS, PERMS, val)) == SEM_FAILED) && (errno == EINTR));
	
	if(*sem != SEM_FAILED)
	{
		return 0;
	}

	if(errno != EEXIST)
	{
		return -1; 
	}

	while(((*sem = sem_open(name, 0)) == SEM_FAILED) && (errno == EINTR));

	if(*sem != SEM_FAILED)
	{
		return 0;
	}

	return -1;
}

/**************************************************
 *                                                *
 *   Detaching and Destorying Named Semaphore     *
 *                                                *
 *************************************************/

int destroyNamed(char *name, sem_t *sem)
{
	int error = 0;
	
	if(sem_close(sem) == -1)
	{
		error = errno;
	}

	if((sem_unlink(name) != -1) && !error)
	{
		return 0;
	}

	if(error)
	{
		errno = error;
	}

	return -1;
} 
