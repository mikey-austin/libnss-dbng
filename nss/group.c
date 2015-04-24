/**
 * @file group.c
 * @brief Implements the functions to retrieve group entries.
 * @author Mikey Austin
 * @date 2015
 */

#include "nss-dbng.h"

#include <grp.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "../lib/dbng/service.h"
#include "../lib/service-group.h"

#define NSS_DBNG_LOCK()                \
    do {                               \
        pthread_mutex_lock(&gmutex);   \
    } while (0)
#define NSS_DBNG_UNLOCK()              \
    do {                               \
        pthread_mutex_unlock(&gmutex); \
    } while (0)

static pthread_mutex_t gmutex = PTHREAD_MUTEX_INITIALIZER;
static SERVICE Gr_service;
static volatile int init;

static enum nss_status fill_group(struct group *, char *, size_t,
                                  SERVICE *, GROUP_REC *, int *);

enum nss_status
_nss_dbng_setgrent(void)
{
    enum nss_status status = NSS_STATUS_SUCCESS;

    NSS_DBNG_LOCK();
    if(service_init(&Gr_service, TYPE_GROUP, DBNG_RO, DEFAULT_BASE) < 0) {
        status = NSS_STATUS_UNAVAIL;
        goto cleanup;
    }
    init = 1;

cleanup:
    NSS_DBNG_UNLOCK();
    return status;
}

enum nss_status
_nss_dbng_endgrent(void)
{
    NSS_DBNG_LOCK();
    if(init == 1) {
        service_cleanup(&Gr_service);
        init = 0;
    }
    NSS_DBNG_UNLOCK();

    return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_dbng_getgrent_r(struct group *gbuf, char *buf, size_t buflen,
                     int *errnop)
{
    GROUP_KEY key;
    GROUP_REC rec;
    int res;
    enum nss_status status;

    NSS_DBNG_LOCK();

    if(init != 1) {
        *errnop = ENOENT;
        status = NSS_STATUS_NOTFOUND;
        goto cleanup;
    }

    res = Gr_service.next(&Gr_service, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        status = fill_group(gbuf, buf, buflen, &Gr_service, &rec, errnop);
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

enum nss_status
_nss_dbng_getgrnam_r(const char* name, struct group *gbuf,
                       char *buf, size_t buflen, int *errnop)
{
    SERVICE group;
    GROUP_KEY key;
    GROUP_REC rec;
    int res;
    enum nss_status status;

    if(service_init(&group, TYPE_GROUP, DBNG_RO, DEFAULT_BASE) < 0) {
        *errnop = ENOENT;
        return NSS_STATUS_UNAVAIL;
    }

    char uname[strlen(name) + 1];
    strcpy(uname, name);
    key.data.pri = uname;
    key.base.type = PRI;

    res = group.get(&group, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        NSS_DEBUG("found group by name %s", name);
        status = fill_group(gbuf, buf, buflen, &group, &rec, errnop);
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
    service_cleanup(&group);
    return status;
}


enum nss_status
_nss_dbng_getgrgid_r(gid_t gid, struct group *gbuf,
                       char *buf, size_t buflen, int *errnop)
{
    SERVICE group;
    GROUP_KEY key;
    GROUP_REC rec;
    int res;
    enum nss_status status;

    if(service_init(&group, TYPE_GROUP, DBNG_RO, DEFAULT_BASE) < 0) {
        *errnop = ENOENT;
        return NSS_STATUS_UNAVAIL;
    }

    /* Query on the secondary index. */
    key.base.type = SEC;
    key.data.sec = gid;
    res = group.get(&group, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        NSS_DEBUG("found group by gid %d", gid);
        status = fill_group(gbuf, buf, buflen, &group, &rec, errnop);
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
    service_cleanup(&group);
    return status;
}

static enum nss_status
fill_group(struct group *gbuf, char *buf, size_t buflen,
           SERVICE *service, GROUP_REC *rec, int *errnop)
{
    int i;
    char **member;

    if(buflen < service->rec_size(service, (REC *) rec)) {
        *errnop = ERANGE;
        return NSS_STATUS_TRYAGAIN;
    }

    gbuf->gr_gid = rec->gid;

    strcpy(buf, rec->name);
    gbuf->gr_name = buf;
    buf += strlen(rec->name) + 1;

    strcpy(buf, rec->passwd);
    gbuf->gr_passwd = buf;
    buf += strlen(rec->passwd) + 1;

    /* Reserve space for the pointers to the group members. */
    gbuf->gr_mem = (char **) buf;
    gbuf->gr_mem[rec->count] = NULL;
    buf += ((rec->count + 1) * sizeof(char *));

    /* Now store the actual member names. */
    for(member = rec->members, i = 0;
        member != NULL && *member != NULL && i < rec->count;
        member++, i++)
    {
        strcpy(buf, *member);
        gbuf->gr_mem[i] = buf;
        buf += strlen(*member) + 1;
    }

    return NSS_STATUS_SUCCESS;
}
