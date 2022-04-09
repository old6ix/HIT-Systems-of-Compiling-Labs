#ifndef _FIELD_LIST_H_
#define _FIELD_LIST_H_

typedef struct symbol_schema *pSchema;
typedef struct field_list *pFieldList;

typedef struct field_list
{
    char *name;      // 名字
    pSchema schema;  // 类型信息
    pFieldList next; // 下一个域
} FieldList;

pFieldList newFieldList(char *newName, pSchema newType);
pFieldList copyFieldList(pFieldList src);
void deleteFieldList(pFieldList fieldList);
void setFieldListName(pFieldList p, char *newName);
void printFieldList(pFieldList fieldList);

#endif
