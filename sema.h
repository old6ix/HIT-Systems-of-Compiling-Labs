#ifndef _SEMA_H_
#define _SEMA_H_

// 哈希表大小
#define HASH_TABLE_SIZE 0x3fff

#include <stdbool.h>
#include "syntax_tree.h"

// 符号类型
typedef enum _kind
{
    BASIC,
    ARRAY,
    STRUCTURE,
    FUNCTION
} Kind;

// 基本类型
typedef enum _basicType
{
    INT_TYPE,
    FLOAT_TYPE
} BasicType;

// 语义分析错误类型
typedef enum _errorType
{
    UNDEF_VAR = 1,        // Undefined Variable
    UNDEF_FUNC,           // Undefined Function
    REDEF_VAR,            // Redefined Variable
    REDEF_FUNC,           // Redefined Function
    TYPE_MISMATCH_ASSIGN, // Type mismatched for assignment.
    LEFT_VAR_ASSIGN,      // The left-hand side of an assignment must be a variable.
    TYPE_MISMATCH_OP,     // Type mismatched for operands.
    TYPE_MISMATCH_RETURN, // Type mismatched for return.
    FUNC_AGRC_MISMATCH,   // Function is not applicable for arguments
    NOT_A_ARRAY,          // Variable is not a Array
    NOT_A_FUNC,           // Variable is not a Function
    NOT_A_INT,            // Variable is not a Integer
    ILLEGAL_USE_DOT,      // Illegal use of "."
    NONEXISTFIELD,        // Non-existentfield
    REDEF_FEILD,          // Redefined field
    DUPLICATED_NAME,      // Duplicated name
    UNDEF_STRUCT          // Undefined structure
} ErrorType;

// 指针简写
typedef struct syntax_node *pNode;
typedef struct type *pType;
typedef struct fieldList *pFieldList;
typedef struct tableItem *pItem;
typedef struct hashTable *pHash;
typedef struct stack *pStack;
typedef struct table *pTable;

typedef struct type
{
    Kind kind;
    union
    {
        // 基本类型
        BasicType basic;
        // 数组类型信息包括元素类型与数组大小构成
        struct
        {
            pType elem;
            int size;
        } array;
        // 结构体类型信息是一个链表
        struct
        {
            char *structName;
            pFieldList field;
        } structure;

        struct
        {
            int argc;         // argument counter
            pFieldList argv;  // argument vector
            pType returnType; // returnType
        } function;
    } u;
} Type;

typedef struct fieldList
{
    char *name;      // 域的名字
    pType type;      // 域的类型
    pFieldList tail; // 下一个域
} FieldList;

typedef struct tableItem
{
    int symbolDepth;
    pFieldList field;
    pItem nextSymbol; // same depth next symbol, linked from stack
    pItem nextHash;   // same hash code next symbol, linked from hash table
} TableItem;

typedef struct hashTable
{
    pItem *hashArray;
} HashTable;

typedef struct stack
{
    pItem *stackArray;
    int curStackDepth;
} Stack;

typedef struct table
{
    pHash hash;
    pStack stack;
    int unNamedStructNum;
} Table;

extern pTable table;

// Type functions
pType newType(Kind kind, ...);
pType copyType(pType src);
void deleteType(pType type);
bool checkType(pType type1, pType type2);
void printType(pType type);

// FieldList functions

// inline pFieldList newFieldList() {
//     pFieldList p = (pFieldList)malloc(sizeof(FieldList));
//     p->name = NULL;
//     p->type = NULL;
//     p->tail = NULL;
//     return p;
// }

pFieldList newFieldList(char *newName, pType newType);
pFieldList copyFieldList(pFieldList src);
void deleteFieldList(pFieldList fieldList);
void setFieldListName(pFieldList p, char *newName);
void printFieldList(pFieldList fieldList);

// tableItem functions

// inline pItem newItem() {
//     pItem p = (pItem)malloc(sizeof(TableItem));
//     p->symbolDepth = 0;
//     p->field = NULL;
//     p->nextHash = NULL;
//     p->nextSymbol = NULL;
//     return p;
// }

pItem newItem(int symbolDepth, pFieldList pfield);
void deleteItem(pItem item);
bool isStructDef(pItem src);

// Hash functions
pHash newHash();
void deleteHash(pHash hash);
pItem getHashHead(pHash hash, int index);
void setHashHead(pHash hash, int index, pItem newVal);

// Stack functions
pStack newStack();
void deleteStack(pStack stack);
void addStackDepth(pStack stack);
void minusStackDepth(pStack stack);
pItem getCurDepthStackHead(pStack stack);
void setCurDepthStackHead(pStack stack, pItem newVal);

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

// Global functions
static inline unsigned int getHashCode(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~HASH_TABLE_SIZE)
            val = (val ^ (i >> 12)) & HASH_TABLE_SIZE;
    }
    return val;
}

static inline void pError(ErrorType type, int line, char *msg)
{
    printf("Error type %d at Line %d: %s\n", type, line, msg);
}

void traverseTree(pNode node);

// Generate symbol table functions
void ExtDef(pNode node);
void ExtDecList(pNode node, pType specifier);
pType Specifier(pNode node);
pType StructSpecifier(pNode node);
pItem VarDec(pNode node, pType specifier);
void FunDec(pNode node, pType returnType);
void VarList(pNode node, pItem func);
pFieldList ParamDec(pNode node);
void CompSt(pNode node, pType returnType);
void StmtList(pNode node, pType returnType);
void Stmt(pNode node, pType returnType);
void DefList(pNode node, pItem structInfo);
void Def(pNode node, pItem structInfo);
void DecList(pNode node, pType specifier, pItem structInfo);
void Dec(pNode node, pType specifier, pItem structInfo);
pType Exp(pNode node);
void Args(pNode node, pItem funcInfo);

#endif
