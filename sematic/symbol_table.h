#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include "hash_table.h"
#include "stack.h"

typedef struct symbol_table *pTable;

typedef struct symbol_table
{
    pHash hash;
    pStack stack;
    int unNamedStructNum;
} SymbolTable;

pTable initTable();
void deleteTable(pTable table);
pItem searchTableItem(pTable table, char *name);

/**
 * 检查表项是否有冲突
 *
 * @param table 符号表
 * @param item 表项
 * @return 是否有冲突
 */
bool checkTableItemConflict(pTable table, pItem item);

void addTableItem(pTable table, pItem item);
void deleteTableItem(pTable table, pItem item);
void clearCurDepthStackList(pTable table);


/**
 * 在stdout输出符号表，Debug用
 *
 * @param table 符号表
 */
void printTable(pTable table);

#endif
