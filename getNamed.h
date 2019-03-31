// Programmer: Andrew McCormick
// Function:   getNamed.h
// Class:      CS-4760 PA 3
// Date:       3/21/2019

#ifndef GETNAMED_H
#define GETNAMED_H

int getNamed(char *name, sem_t **sem, int val);
int destroyNamed(char *name, sem_t *sem);

#endif
