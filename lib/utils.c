/**
 *
 *
 */

#include "utils.h"

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
*xcalloc(size_t nmemb, size_t size)
{
    void *res = NULL;

    res = calloc(nmemb, size);
    if(res == NULL)
        err(1, "calloc");

    return res;
}

extern void
xfree(void **p)
{
    if(*p != NULL)
        free(*p);
    *p = NULL;
}
