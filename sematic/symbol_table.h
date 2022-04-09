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

/**
 * 在符号表上查找符号
 *
 * @param table 符号表
 * @param name 符号名
 * @return 成功则返回对应表项的指针，否则返回NULL
 */
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

#endif
