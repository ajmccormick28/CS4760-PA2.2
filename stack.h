// Programmer: Andrew McCormick
// Function:   stack.h
// Class:      CS-4760 PA3
// Date:       3/21/2019

// Source code for this file came from RenatoUtsch / stack.c on GitHub

#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#if !defined(_bool_true_false_are_defined) && !defined(_cpluscplus)

typedef int bool;
#define true 1
#define false 0
#define __bool__true_false_are_defined
#endif

#define StackItem char*

typedef struct Stack Stack;

Stack *stackCreate();

void stackDestroy(Stack *stack);
void stackClean(Stack *stack);
bool stackIsEmpty(Stack *stack);
size_t stackSize(Stack *stack);
StackItem stackTop(Stack *stack);
bool stackPush(Stack *stack, StackItem item);
StackItem stackPop(Stack *stack);

#endif
