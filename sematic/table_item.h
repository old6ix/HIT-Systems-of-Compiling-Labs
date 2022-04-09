#ifndef _TABLE_ITEM_H_
#define _TABLE_ITEM_H_

#include <stdbool.h>

#include "field_list.h"

typedef struct table_item *pItem;

typedef struct table_item
{
    int depth;
    pFieldList field;
    pItem nextSymbol; // same depth next symbol, linked from stack
    pItem nextHash;   // same hash code next symbol, linked from hash table
} TableItem;

/**
 * 检查表项是否有冲突
 *
 * @param depth 嵌套深度
 * @param field 该项对应的域
 * @return 生成的十字链表项
 */
pItem newItem(int depth, pFieldList field);

void deleteItem(pItem item);
bool isStructDef(pItem src);

#endif
