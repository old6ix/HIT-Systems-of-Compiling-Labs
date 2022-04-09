#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sema_util.h"

inline char *newString(char *src)
{
    if (src == NULL)
        return NULL;
    int length = strlen(src) + 1;
    char *p = (char *)malloc(sizeof(char) * length);
    assert(p != NULL);
    strncpy(p, src, length);
    return p;
}
