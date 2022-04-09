#include <stdio.h>
#include "sema_error.h"

inline void pError(ErrorType type, int line, char *msg)
{
    printf("Error type %d at Line %d: %s\n", type, line, msg);
}
