// Programmer: Andrew McCormick 
// Function:   optArg.h
// Class:      CS-4760 PA2
// Date:       3/5/2019

#ifndef OPTARG_H
#define OPTARG_H

typedef struct
{
	const char *inputFileName; // -i option
	const char *outputFileName; // -o option
	int numChild; // -n option
	int childAtTime; // -s option

} OptArg;

#endif
