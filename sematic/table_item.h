#ifndef _TABLE_ITEM_H_
#define _TABLE_ITEM_H_

#include <stdbool.h>

#include "field_list.h"

typedef struct table_item *pItem;

typedef struct table_item
{
    int symbolDepth;
    pFieldList field;
    pItem nextSymbol; // same depth next symbol, linked from stack
    pItem nextHash;   // same hash code next symbol, linked from hash table
} TableItem;

pItem newItem(int symbolDepth, pFieldList pfield);
void deleteItem(pItem item);
bool isStructDef(pItem src);

#endif
