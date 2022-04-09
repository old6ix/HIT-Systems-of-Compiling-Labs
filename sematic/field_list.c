#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sema_util.h"
#include "symbol_schema.h"
#include "field_list.h"

pFieldList newFieldList(char *newName, pSchema newType)
{
    pFieldList p = (pFieldList)malloc(sizeof(FieldList));
    assert(p != NULL);
    p->name = newString(newName);
    p->schema = newType;
    p->next = NULL;
    return p;
}

pFieldList copyFieldList(pFieldList src)
{
    assert(src != NULL);
    pFieldList head = NULL, cur = NULL;
    pFieldList temp = src;

    while (temp)
    {
        if (!head)
        {
            head = newFieldList(temp->name, copyType(temp->schema));
            cur = head;
            temp = temp->next;
        }
        else
        {
            cur->next = newFieldList(temp->name, copyType(temp->schema));
            cur = cur->next;
            temp = temp->next;
        }
    }
    return head;
}

void deleteFieldList(pFieldList fieldList)
{
    assert(fieldList != NULL);
    if (fieldList->name)
    {
        free(fieldList->name);
        fieldList->name = NULL;
    }
    if (fieldList->schema)
        deleteType(fieldList->schema);
    fieldList->schema = NULL;
    free(fieldList);
}

void setFieldListName(pFieldList p, char *newName)
{
    assert(p != NULL && newName != NULL);
    if (p->name != NULL)
    {
        free(p->name);
    }
    p->name = newString(newName);
}

void printFieldList(pFieldList fieldList)
{
    if (fieldList == NULL)
        printf("fieldList is NULL\n");
    else
    {
        printf("fieldList name is: %s\n", fieldList->name);
        printf("FieldList Type:\n");
        printType(fieldList->schema);
        printFieldList(fieldList->next);
    }
}
