/**
 * @file passwd.c
 * @brief Implements the functions to retrieve passwd entries.
 * @author Mikey Austin
 * @date 2015
 */

#include "nss-dbng.h"

#include <pwd.h>

/**
 *
 */
enum nss_status
_nss_dbng_setpwent(void)
{
    NSS_DEBUG("Initializing pw functions");
    return NSS_STATUS_SUCCESS;
}

/**
 *
 */
enum nss_status
_nss_dbng_endpwent(void)
{
    NSS_DEBUG("Finishing pw functions");
    return NSS_STATUS_SUCCESS;
}

/**
 *
 */
enum nss_status
_nss_dbng_getpwent_r(struct passwd *pwbuf, char *buf,
                     size_t buflen, int *errnop)
{
    NSS_DEBUG("getpwent_r");
    return NSS_STATUS_UNAVAIL;
}

/**
 *
 */
enum nss_status
_nss_dbng_getpwnam_r(const char* name, struct passwd *pwbuf,
                     char *buf, size_t buflen, int *errnop)
{
    NSS_DEBUG("getpwnam_r");
    return NSS_STATUS_NOTFOUND;
}

/**
 *
 */
enum nss_status
_nss_dbng_getpwuid_r(uid_t uid, struct passwd *pwbuf,
                     char *buf, size_t buflen, int *errnop)
{
    NSS_DEBUG("getpwnam_r");
    return NSS_STATUS_NOTFOUND;
}
