// Programmer: Andrew McCormick
// Function:   palinCheck.c
// Class:      CS-4760 PA3
// Date:       3/21/2019

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "palinCheck.h"
#include "inputHold.h"

static InputHold *inputArr;

int palinCheck(int index)
{
	int stringLen = 0;
	int i = 0;

        stringLen = strlen(inputArr -> input[index]) - 1;


        if((stringLen % 2) == 0)
        {
                for(i = 0; i < (stringLen / 2); i++)
                {
                        if(inputArr -> input[index][i] != inputArr -> input[index][stringLen - 1 - i])
                        {
                                return 1;
                        }
                }

        }
	
        else
        {
                for(i = 0; i < ((stringLen - 1) / 2); i++)
                {
                        if(inputArr -> input[index][i] != inputArr -> input[index][stringLen - 1 - i])
                        {
                                return 1;
                        }
                }
        }
	
	return 0;
}
