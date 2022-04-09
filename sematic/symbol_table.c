#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "symbol_schema.h"
#include "hash_table.h"
#include "symbol_table.h"

pTable initTable()
{
    pTable table = (pTable)malloc(sizeof(SymbolTable));
    assert(table != NULL);
    table->hash = newHash();
    table->stack = newStack();
    table->unnamedStructCnt = 0;
    return table;
};

void deleteTable(pTable table)
{
    deleteHash(table->hash);
    table->hash = NULL;
    deleteStack(table->stack);
    table->stack = NULL;
    free(table);
};

pItem searchTableItem(pTable table, char *name)
{
    unsigned hashCode = getHashCode(name);
    pItem temp = getHashHead(table->hash, hashCode);
    if (temp == NULL)
        return NULL;
    while (temp)
    {
        if (!strcmp(temp->field->name, name))
            return temp;
        temp = temp->nextHash;
    }
    return NULL;
}

bool checkTableItemConflict(pTable table, pItem item)
{
    pItem temp = searchTableItem(table, item->field->name);
    if (temp == NULL)
        return 0;
    while (temp)
    {
        if (!strcmp(temp->field->name, item->field->name))
        {
            // 二者都是结构体，没有作用域，直接报冲突
            if (temp->field->schema->kind == STRUCTURE ||
                item->field->schema->kind == STRUCTURE)
                return 1;
            if (temp->depth == table->stack->curStackDepth)
                return 1;
        }
        temp = temp->nextHash;
    }
    return 0;
}

void addTableItem(pTable table, pItem item)
{
    assert(table != NULL && item != NULL);
    unsigned hashCode = getHashCode(item->field->name);
    pHash hash = table->hash;
    pStack stack = table->stack;
    
    item->nextSymbol = getCurDepthStackHead(stack);
    setCurDepthStackHead(stack, item);

    item->nextHash = getHashHead(hash, hashCode);
    setHashHead(hash, hashCode, item);
}

void deleteTableItem(pTable table, pItem item)
{
    assert(table != NULL && item != NULL);
    unsigned hashCode = getHashCode(item->field->name);
    if (item == getHashHead(table->hash, hashCode))
        setHashHead(table->hash, hashCode, item->nextHash);
    else
    {
        pItem cur = getHashHead(table->hash, hashCode);
        pItem last = cur;
        while (cur != item)
        {
            last = cur;
            cur = cur->nextHash;
        }
        last->nextHash = cur->nextHash;
    }
    deleteItem(item);
}

void clearCurDepthStackList(pTable table)
{
    assert(table != NULL);
    pStack stack = table->stack;
    pItem temp = getCurDepthStackHead(stack);
    while (temp)
    {
        pItem tDelete = temp;
        temp = temp->nextSymbol;
        deleteTableItem(table, tDelete);
    }
    setCurDepthStackHead(stack, NULL);
    popStack(stack);
}
