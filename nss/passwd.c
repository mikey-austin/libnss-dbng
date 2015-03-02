/**
 * @file passwd.c
 * @brief Implements the functions to retrieve passwd entries.
 * @author Mikey Austin
 * @date 2015
 */

#include "nss-dbng.h"

#include <pwd.h>
#include <errno.h>
#include <string.h>

#include <lib/dbng/service.h>
#include <lib/service-passwd.h>

static enum nss_status fill_passwd(struct passwd *, char *, size_t, SERVICE *, PASSWD_REC *, int *);

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
    SERVICE *passwd;
    PASSWD_KEY key;
    PASSWD_REC rec;
    int res;
    enum nss_status status;

    if((passwd = service_create(PASSWD, DBNG_RO, DEFAULT_BASE)) == NULL) {
        NSS_DEBUG("could not create passwd service object");
        return NSS_STATUS_UNAVAIL;
    }

    key.base.type = PRI;
    key.data.pri = (char *) name;
    res = passwd->get(passwd, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        NSS_DEBUG("found user by name %s", name);
        status = fill_passwd(pwbuf, buf, buflen, passwd, &rec, errnop);
        break;

    case DB_NOTFOUND:
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;

    default:
        NSS_DEBUG("unknown status from get: %d", res);
        goto cleanup;
    }

cleanup:
    service_free(&passwd);

    return status;
}

/**
 *
 */
enum nss_status
_nss_dbng_getpwuid_r(uid_t uid, struct passwd *pwbuf,
                     char *buf, size_t buflen, int *errnop)
{
    SERVICE *passwd;
    PASSWD_KEY key;
    PASSWD_REC rec;
    int res;
    enum nss_status status;

    if((passwd = service_create(PASSWD, DBNG_RO, DEFAULT_BASE)) == NULL) {
        NSS_DEBUG("could not create passwd service object");
        return NSS_STATUS_UNAVAIL;
    }

    /* Query on the secondary index. */
    key.base.type = SEC;
    key.data.sec = uid;
    res = passwd->get(passwd, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        NSS_DEBUG("found user by uid %d", uid);
        status = fill_passwd(pwbuf, buf, buflen, passwd, &rec, errnop);
        break;

    case DB_NOTFOUND:
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;

    default:
        NSS_DEBUG("unknown status from get: %d", res);
        goto cleanup;
    }

cleanup:
    service_free(&passwd);

    return status;
}

static enum nss_status
fill_passwd(struct passwd *pwbuf, char *buf, size_t buflen,
            SERVICE *service, PASSWD_REC *rec, int *errnop)
{
    if(buflen < service->rec_size(service, (REC *) rec)) {
        *errnop = ERANGE;
        return NSS_STATUS_TRYAGAIN;
    }

    pwbuf->pw_uid = rec->uid;
    pwbuf->pw_gid = rec->gid;

    strcpy(buf, rec->name);
    pwbuf->pw_name = buf;
    buf += strlen(rec->name) + 1;

    strcpy(buf, rec->passwd);
    pwbuf->pw_passwd = buf;
    buf += strlen(rec->passwd) + 1;

    strcpy(buf, rec->gecos);
    pwbuf->pw_gecos = buf;
    buf += strlen(rec->gecos) + 1;

    strcpy(buf, rec->shell);
    pwbuf->pw_shell = buf;
    buf += strlen(rec->shell) + 1;

    strcpy(buf, rec->homedir);
    pwbuf->pw_dir = buf;

    return NSS_STATUS_SUCCESS;
}
