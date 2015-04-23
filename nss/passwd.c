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
#include <pthread.h>

#include "../lib/dbng/service.h"
#include "../lib/service-passwd.h"

#define NSS_DBNG_LOCK()                \
    do {                               \
        pthread_mutex_lock(&pmutex);   \
    } while (0)
#define NSS_DBNG_UNLOCK()              \
    do {                               \
        pthread_mutex_unlock(&pmutex); \
    } while (0)

static pthread_mutex_t pmutex = PTHREAD_MUTEX_INITIALIZER;
static SERVICE Pwd_service;
static volatile int init;

static enum nss_status fill_passwd(struct passwd *, char *, size_t,
                                   SERVICE *, PASSWD_REC *, int *);

/**
 *
 */
enum nss_status
_nss_dbng_setpwent(void)
{
    enum nss_status status = NSS_STATUS_SUCCESS;

    NSS_DBNG_LOCK();
    service_init(&Pwd_service, TYPE_PASSWD, DBNG_RO, DEFAULT_BASE);
    if(Pwd_service.db.pri == NULL) {
        status = NSS_STATUS_UNAVAIL;
        goto cleanup;
    }
    init = 1;
    NSS_DBNG_UNLOCK();

cleanup:
    return status;
}

/**
 *
 */
enum nss_status
_nss_dbng_endpwent(void)
{
    NSS_DBNG_LOCK();
    if(init) {
        service_cleanup(&Pwd_service);
        init = 0;
    }
    NSS_DBNG_UNLOCK();

    return NSS_STATUS_SUCCESS;
}

/**
 *
 */
enum nss_status
_nss_dbng_getpwent_r(struct passwd *pwbuf, char *buf,
                     size_t buflen, int *errnop)
{
    PASSWD_KEY key;
    PASSWD_REC rec;
    int res;
    enum nss_status status;

    NSS_DBNG_LOCK();

    if(!init)
        goto cleanup;

    res = Pwd_service.next(&Pwd_service, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        status = fill_passwd(pwbuf, buf, buflen, &Pwd_service, &rec, errnop);
        break;

    case DB_NOTFOUND:
        *errnop = ENOENT;
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;

    default:
        NSS_DEBUG("unknown status from next: %d", res);
        *errnop = ENOENT;
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;
    }

cleanup:
    NSS_DBNG_UNLOCK();
    return status;
}

/**
 *
 */
enum nss_status
_nss_dbng_getpwnam_r(const char* name, struct passwd *pwbuf,
                     char *buf, size_t buflen, int *errnop)
{
    SERVICE passwd;
    PASSWD_KEY key;
    PASSWD_REC rec;
    int res;
    enum nss_status status;

    service_init(&passwd, TYPE_PASSWD, DBNG_RO, DEFAULT_BASE);
    if(passwd.db.pri == NULL) {
        *errnop = ENOENT;
        return NSS_STATUS_UNAVAIL;
    }

    char uname[strlen(name) + 1];
    strcpy(uname, name);
    key.data.pri = uname;
    key.base.type = PRI;

    res = passwd.get(&passwd, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        NSS_DEBUG("found user by name %s", name);
        status = fill_passwd(pwbuf, buf, buflen, &passwd, &rec, errnop);
        break;

    case DB_NOTFOUND:
        *errnop = ENOENT;
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;

    default:
        NSS_DEBUG("unknown status from get: %d", res);
        *errnop = ENOENT;
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;
    }

cleanup:
    service_cleanup(&passwd);
    return status;
}

/**
 *
 */
enum nss_status
_nss_dbng_getpwuid_r(uid_t uid, struct passwd *pwbuf,
                     char *buf, size_t buflen, int *errnop)
{
    SERVICE passwd;
    PASSWD_KEY key;
    PASSWD_REC rec;
    int res;
    enum nss_status status;

    service_init(&passwd, TYPE_PASSWD, DBNG_RO, DEFAULT_BASE);
    if(passwd.db.pri == NULL) {
        *errnop = ENOENT;
        status = NSS_STATUS_UNAVAIL;
        goto cleanup;
    }

    /* Query on the secondary index. */
    key.base.type = SEC;
    key.data.sec = uid;
    res = passwd.get(&passwd, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        NSS_DEBUG("found user by uid %d", uid);
        status = fill_passwd(pwbuf, buf, buflen, &passwd, &rec, errnop);
        break;

    case DB_NOTFOUND:
        *errnop = ENOENT;
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;

    default:
        NSS_DEBUG("unknown status from get: %d", res);
        *errnop = ENOENT;
        status = NSS_STATUS_UNAVAIL;
        goto cleanup;
    }

cleanup:
    service_cleanup(&passwd);
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
