#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

// 哈希表大小
#define HASH_TABLE_SIZE 0x3fff

#include "table_item.h"

typedef struct hashTable *pHash;

typedef struct hashTable
{
    pItem *hashArray;
} HashTable;

pHash newHash();
void deleteHash(pHash hash);
pItem getHashHead(pHash hash, int index);
void setHashHead(pHash hash, int index, pItem newVal);

unsigned int getHashCode(char *name);

#endif
