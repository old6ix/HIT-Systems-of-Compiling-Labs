#include <stdlib.h>
#include <assert.h>

#include "table_item.h"
#include "hash_table.h"

pHash newHash()
{
    pHash p = (pHash)malloc(sizeof(HashTable));
    assert(p != NULL);
    p->hashArray = (pItem *)malloc(sizeof(pItem) * HASH_TABLE_SIZE);
    assert(p->hashArray != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        p->hashArray[i] = NULL;
    }
    return p;
}

void deleteHash(pHash hash)
{
    assert(hash != NULL);
    for (int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        pItem temp = hash->hashArray[i];
        while (temp)
        {
            pItem tdelete = temp;
            temp = temp->nextHash;
            deleteItem(tdelete);
        }
        hash->hashArray[i] = NULL;
    }
    free(hash->hashArray);
    hash->hashArray = NULL;
    free(hash);
}

pItem getHashHead(pHash hash, int index)
{
    assert(hash != NULL);
    return hash->hashArray[index];
}

void setHashHead(pHash hash, int index, pItem newVal)
{
    assert(hash != NULL);
    hash->hashArray[index] = newVal;
}


inline unsigned int getHashCode(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if ((i = val & ~HASH_TABLE_SIZE) != 0)
            val = (val ^ (i >> 12)) & HASH_TABLE_SIZE;
    }
    return val;
}
