#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "sema_util.h"
#include "field_list.h"
#include "symbol_schema.h"

pSchema newType(SymbolKind kind, ...)
{
    pSchema p = (pSchema)malloc(sizeof(SymbolSchema));
    assert(p != NULL);
    p->kind = kind;
    va_list vaList;
    assert(kind == BASIC || kind == ARRAY || kind == STRUCTURE || kind == FUNCTION);

    va_start(vaList, kind);
    switch (kind)
    {
    case BASIC:
        p->data.basic = va_arg(vaList, BasicType);
        break;
    case ARRAY:
        p->data.array.elem = va_arg(vaList, pSchema);
        p->data.array.size = va_arg(vaList, int);
        break;
    case STRUCTURE:
        p->data.structure.structName = va_arg(vaList, char *);
        p->data.structure.field = va_arg(vaList, pFieldList);
        break;
    case FUNCTION:
        p->data.function.argc = va_arg(vaList, int);
        p->data.function.argv = va_arg(vaList, pFieldList);
        p->data.function.returnType = va_arg(vaList, pSchema);
        break;
    }
    va_end(vaList);
    return p;
}

pSchema copyType(pSchema src)
{
    if (src == NULL)
        return NULL;
    pSchema p = (pSchema)malloc(sizeof(SymbolSchema));
    assert(p != NULL);
    p->kind = src->kind;
    assert(p->kind == BASIC || p->kind == ARRAY || p->kind == STRUCTURE || p->kind == FUNCTION);
    switch (p->kind)
    {
    case BASIC:
        p->data.basic = src->data.basic;
        break;
    case ARRAY:
        p->data.array.elem = copyType(src->data.array.elem);
        p->data.array.size = src->data.array.size;
        break;
    case STRUCTURE:
        p->data.structure.structName = newString(src->data.structure.structName);
        p->data.structure.field = copyFieldList(src->data.structure.field);
        break;
    case FUNCTION:
        p->data.function.argc = src->data.function.argc;
        p->data.function.argv = copyFieldList(src->data.function.argv);
        p->data.function.returnType = copyType(src->data.function.returnType);
        break;
    }

    return p;
}

void deleteType(pSchema type)
{
    assert(type != NULL);
    assert(type->kind == BASIC || type->kind == ARRAY ||
           type->kind == STRUCTURE || type->kind == FUNCTION);
    pFieldList temp = NULL;
    // pFieldList tDelete = NULL;
    switch (type->kind)
    {
    case BASIC:
        break;
    case ARRAY:
        deleteType(type->data.array.elem);
        type->data.array.elem = NULL;
        break;
    case STRUCTURE:
        if (type->data.structure.structName)
            free(type->data.structure.structName);
        type->data.structure.structName = NULL;

        temp = type->data.structure.field;
        while (temp)
        {
            pFieldList tDelete = temp;
            temp = temp->next;
            deleteFieldList(tDelete);
        }
        type->data.structure.field = NULL;
        break;
    case FUNCTION:
        deleteType(type->data.function.returnType);
        type->data.function.returnType = NULL;
        temp = type->data.function.argv;
        while (temp)
        {
            pFieldList tDelete = temp;
            temp = temp->next;
            deleteFieldList(tDelete);
        }
        type->data.function.argv = NULL;
        break;
    }
    free(type);
}

bool checkType(pSchema type1, pSchema type2)
{
    if (type1 == NULL || type2 == NULL)
        return 1;
    if (type1->kind == FUNCTION || type2->kind == FUNCTION)
        return 0;
    if (type1->kind != type2->kind)
        return 0;
    else
    {
        assert(type1->kind == BASIC || type1->kind == ARRAY ||
               type1->kind == STRUCTURE);
        switch (type1->kind)
        {
        case BASIC:
            return type1->data.basic == type2->data.basic;
        case ARRAY:
            return checkType(type1->data.array.elem, type2->data.array.elem);
        case STRUCTURE:
            return !strcmp(type1->data.structure.structName,
                           type2->data.structure.structName);
        default:
            return 0;
        }
    }
}

void printType(pSchema type)
{
    if (type == NULL)
    {
        printf("type is NULL.\n");
    }
    else
    {
        printf("type kind: %d\n", type->kind);
        switch (type->kind)
        {
        case BASIC:
            printf("type basic: %d\n", type->data.basic);
            break;
        case ARRAY:
            printf("array size: %d\n", type->data.array.size);
            printType(type->data.array.elem);
            break;
        case STRUCTURE:
            if (!type->data.structure.structName)
                printf("struct name is NULL\n");
            else
            {
                printf("struct name is %s\n", type->data.structure.structName);
            }
            printFieldList(type->data.structure.field);
            break;
        case FUNCTION:
            printf("function argc is %d\n", type->data.function.argc);
            printf("function args:\n");
            printFieldList(type->data.function.argv);
            printf("function return type:\n");
            printType(type->data.function.returnType);
            break;
        }
    }
}
