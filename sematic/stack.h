#ifndef _STACK_H_
#define _STACK_H_

#include "table_item.h"

typedef struct stack *pStack;

typedef struct stack
{
    pItem *stackArray;
    int curStackDepth;
} Stack;

pStack newStack();
void deleteStack(pStack stack);
void addStackDepth(pStack stack);
void popStack(pStack stack);
pItem getCurDepthStackHead(pStack stack);
void setCurDepthStackHead(pStack stack, pItem newVal);

#endif
