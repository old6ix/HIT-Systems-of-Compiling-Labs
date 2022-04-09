#ifndef _SEMA_H_
#define _SEMA_H_

#include <stdbool.h>
#include "syntax_tree.h"
#include "sematic/symbol_schema.h"
#include "sematic/field_list.h"
#include "sematic/table_item.h"
#include "sematic/symbol_table.h"

extern pTable table;

/**
 * 遍历语法分析树，生成并检查符号表
 *
 * @param node 语法树根节点
 */
void traverseTree(SyntaxNode *node);

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
