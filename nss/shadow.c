/**
 * @file shadow.c
 * @brief Implements the functions to retrieve shadow entries.
 * @author Mikey Austin
 * @date 2015
 */

#include "nss-dbng.h"

#include <shadow.h>

enum nss_status
_nss_dbng_getspnam_r(const char* name, struct spwd *spbuf,
                       char *buf, size_t buflen, int *errnop)
{
    return NSS_STATUS_NOTFOUND;
}
