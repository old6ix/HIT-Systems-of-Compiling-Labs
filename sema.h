#ifndef _SEMA_H_
#define _SEMA_H_

#include <stdbool.h>
#include "syntax_tree.h"
#include "sematic/symbol_schema.h"
#include "sematic/field_list.h"
#include "sematic/table_item.h"
#include "sematic/hash_table.h"
#include "sematic/stack.h"

// 指针简写
typedef struct table *pTable;

typedef struct table
{
    pHash hash;
    pStack stack;
    int unNamedStructNum;
} Table;

extern pTable table;

// Table functions
pTable initTable();
void deleteTable(pTable table);
pItem searchTableItem(pTable table, char *name);
bool checkTableItemConflict(pTable table, pItem item);
void addTableItem(pTable table, pItem item);
void deleteTableItem(pTable table, pItem item);
void clearCurDepthStackList(pTable table);
// void addStructLayer(pTable table);
// void minusStructLayer(pTable table);
// bool isInStructLayer(pTable table);
void printTable(pTable table);

void traverseTree(SyntaxNode *node);

// Generate symbol table functions
void ExtDef(SyntaxNode *node);
void ExtDecList(SyntaxNode *node, pSchema specifier);
pSchema Specifier(SyntaxNode *node);
pSchema StructSpecifier(SyntaxNode *node);
pItem VarDec(SyntaxNode *node, pSchema specifier);
void FunDec(SyntaxNode *node, pSchema returnType);
void VarList(SyntaxNode *node, pItem func);
pFieldList ParamDec(SyntaxNode *node);
void CompSt(SyntaxNode *node, pSchema returnType);
void StmtList(SyntaxNode *node, pSchema returnType);
void Stmt(SyntaxNode *node, pSchema returnType);
void DefList(SyntaxNode *node, pItem structInfo);
void Def(SyntaxNode *node, pItem structInfo);
void DecList(SyntaxNode *node, pSchema specifier, pItem structInfo);
void Dec(SyntaxNode *node, pSchema specifier, pItem structInfo);
pSchema Exp(SyntaxNode *node);
void Args(SyntaxNode *node, pItem funcInfo);

#endif
