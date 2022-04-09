#include <stdlib.h>
#include <assert.h>

#include "hash_table.h"
#include "stack.h"

pStack newStack()
{
    pStack p = (pStack)malloc(sizeof(Stack));
    assert(p != NULL);
    p->stackArray = (pItem *)malloc(sizeof(pItem) * HASH_TABLE_SIZE);
    assert(p->stackArray != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        p->stackArray[i] = NULL;
    }
    p->curStackDepth = 0;
    return p;
}

void deleteStack(pStack stack)
{
    assert(stack != NULL);
    free(stack->stackArray);
    stack->stackArray = NULL;
    stack->curStackDepth = 0;
    free(stack);
}

void addStackDepth(pStack stack)
{
    assert(stack != NULL);
    stack->curStackDepth++;
}

void popStack(pStack stack)
{
    assert(stack != NULL);
    stack->curStackDepth--;
}

pItem getCurDepthStackHead(pStack stack)
{
    assert(stack != NULL);
    return stack->stackArray[stack->curStackDepth];
}

void setCurDepthStackHead(pStack stack, pItem newVal)
{
    assert(stack != NULL);
    stack->stackArray[stack->curStackDepth] = newVal;
}

