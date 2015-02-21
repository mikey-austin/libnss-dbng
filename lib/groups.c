/**
 * @file groups.c
 * @brief Implements the functions to retrieve group entries.
 * @author Mikey Austin
 * @date 2015
 */

#include "nss-dbng.h"

#include <grp.h>

enum nss_status
_nss_dbng_setgrent(void)
{
    return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_dbng_endgrent(void)
{
    return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_dbng_getgrent_r(struct group *gbuf, char *buf,
                       size_t buflen, int *errnop)
{
    return NSS_STATUS_NOTFOUND;
}

enum nss_status
_nss_dbng_getgrnam_r(const char* name, struct group *gbuf,
                       char *buf, size_t buflen, int *errnop)
{
    return NSS_STATUS_NOTFOUND;
}


enum nss_status
_nss_dbng_getgrgid_r(gid_t gid, struct group *gbuf,
                       char *buf, size_t buflen, int *errnop)
{
        return NSS_STATUS_NOTFOUND;
}

enum nss_status
_nss_dbng_initgroups_dyn(const char *user, gid_t gid, long int *start,
                           long int *size, gid_t **groupsp, long int limit,
                           int *errnop)
{
    return NSS_STATUS_UNAVAIL;
}
