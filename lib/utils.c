/**
 *
 *
 */

#include "dbng/utils.h"

extern void
*xmalloc(size_t size)
{
    void *res = NULL;

    res = malloc(size);
    if(res == NULL)
        err(1, "malloc");

    return res;
}

extern void
xfree(void **p)
{
    if(*p != NULL)
        free(*p);
    *p = NULL;
}
