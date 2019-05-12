// Programmer: Andrew McCormick
// Function:  sharedTime.h
// Class:     CS-4760 PA4
// Date:      4/11/2019

#ifndef SHAREDTIME_H
#define SHAREDTIME_H

typedef struct
{
    int cpuUsage;
    int totalTime;
    int lastBurst;
    int proPrioty;
    int simPid;
}
ControlBlock;

typedef struct
{
	int seconds;
	double nanoSecs;

	ControlBlock block[18];
}
SharedTime;


#endif
