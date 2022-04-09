#include <stdlib.h>
#include <assert.h>

#include "symbol_schema.h"
#include "table_item.h"

pItem newItem(int symbolDepth, pFieldList pfield)
{
    pItem p = (pItem)malloc(sizeof(TableItem));
    assert(p != NULL);
    p->symbolDepth = symbolDepth;
    p->field = pfield;
    p->nextHash = NULL;
    p->nextSymbol = NULL;
    return p;
}

void deleteItem(pItem item)
{
    assert(item != NULL);
    if (item->field != NULL)
        deleteFieldList(item->field);
    free(item);
}

bool isStructDef(pItem src)
{
    if (src == NULL)
        return 0;
    if (src->field->schema->kind != STRUCTURE)
        return 0;
    if (src->field->schema->data.structure.structName)
        return 0;
    return 1;
}
