// Programmer: Andrew McCormick 
// Function:   optArg.h
// Class:      CS-4760 PA3
// Date:       3/21/2019

#ifndef OPTARG_H
#define OPTARG_H

typedef struct
{
	const char *inputFileName; // -i option
	const char *palin; // Palindrom file
	const char *noPalin; // non-Palindrom file
	int numChild; // -n option
} OptArg;

#endif
