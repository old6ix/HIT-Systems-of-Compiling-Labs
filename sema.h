#ifndef _SEMA_H_
#define _SEMA_H_

// 哈希表大小
#define HASH_TABLE_SIZE 0x3fff

#include <stdbool.h>
#include "syntax_tree.h"

// 符号类型
typedef enum symbol_kind
{
    BASIC,
    ARRAY,
    STRUCTURE,
    FUNCTION
} SymbolKind;

/**
 * 基本类型
 */
typedef enum basic_type
{
    INT_TYPE,
    FLOAT_TYPE
} BasicType;

/**
 * 语义分析错误类型
 * 对应书P39的17个错误类型
 */
typedef enum error_type
{
    UNDEF_VAR = 1,        // 变量未定义即使用
    UNDEF_FUNC,           // 函数未定义即使用
    REDEF_VAR,            // 变量重复定义
    REDEF_FUNC,           // 函数重复定义
    TYPE_MISMATCH_ASSIGN, // 赋值号两边类型不匹配
    LEFT_VAR_ASSIGN,      // 赋值号左边无法赋值
    TYPE_MISMATCH_OP,     // 操作数不匹配
    TYPE_MISMATCH_RETURN, // 返回值类型不对
    FUNC_AGRC_MISMATCH,   // 函数参数有误
    NOT_A_ARRAY,          // 变量非数组
    NOT_A_FUNC,           // 变量非函数
    NOT_A_INT,            // 变量非int
    ILLEGAL_USE_DOT,      // 变量非数组（非法使用取域运算符）
    NONEXISTFIELD,        // struct域不存在
    REDEF_FEILD,          // 域重复定义
    DUPLICATED_NAME,      // 结构体名重复定义（先前值也可能为变量or函数）
    UNDEF_STRUCT          // 结构体未定义即使用
} ErrorType;

// 指针简写
typedef struct syntax_node *pNode;
typedef struct symbol_type *pType;
typedef struct fieldList *pFieldList;
typedef struct tableItem *pItem;
typedef struct hashTable *pHash;
typedef struct stack *pStack;
typedef struct table *pTable;

typedef struct symbol_type
{
    SymbolKind kind; // 符号类型
    union            // 符号数据
    {
        // 基本类型
        BasicType basic;

        // 数组
        struct
        {
            pType elem; // 元素类型
            int size;   // 数组大小
        } array;

        // 结构体
        struct
        {
            char *structName;
            pFieldList field;
        } structure;

        // 函数
        struct
        {
            int argc;         // 参数个数
            pFieldList argv;  // 各参数
            pType returnType; // 返回值类型
        } function;
    } data;
} SymbolType;

typedef struct fieldList
{
    char *name;      // 名字
    pType type;      // 类型
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
pType newType(SymbolKind kind, ...);
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
