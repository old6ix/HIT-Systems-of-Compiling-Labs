#ifndef _SYMBOL_SCHEMA_H_
#define _SYMBOL_SCHEMA_H_

#include <stdbool.h>

typedef struct symbol_schema *pSchema;
typedef struct field_list *pFieldList;

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
 * 符号数据
 */
typedef struct symbol_schema
{
    SymbolKind kind; // 符号类别
    union            // 符号数据
    {
        // 基本类型
        BasicType basic;

        // 数组
        struct
        {
            pSchema elem; // 元素类型
            int size;     // 数组大小
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
            int argc;           // 参数个数
            pFieldList argv;    // 各参数
            pSchema returnType; // 返回值类型
        } function;
    } data;
} SymbolSchema;

pSchema newType(SymbolKind kind, ...);
pSchema copyType(pSchema src);
void deleteType(pSchema type);
bool checkType(pSchema type1, pSchema type2);
void printType(pSchema type);

#endif
