/**
 * @file shadow.c
 * @brief Implements the functions to retrieve shadow entries.
 * @author Mikey Austin
 * @date 2015
 */

#include "nss-dbng.h"

#include <shadow.h>
#include <string.h>
#include <errno.h>

#include "../lib/service-shadow.h"

static enum nss_status fill_shadow(struct spwd *, char *, size_t,
                                   SERVICE *, SHADOW_REC *, int *);

enum nss_status
_nss_dbng_getspnam_r(const char* name, struct spwd *spbuf,
                     char *buf, size_t buflen, int *errnop)
{
    SERVICE shadow;
    SHADOW_KEY key;
    SHADOW_REC rec;
    int res;
    enum nss_status status;

    service_init(&shadow, TYPE_SHADOW, DBNG_RO, DEFAULT_BASE);

    char uname[strlen(name) + 1];
    strcpy(uname, name);
    key.data.pri = uname;
    key.base.type = PRI;

    res = shadow.get(&shadow, (KEY *) &key, (REC *) &rec);
    switch(res) {
    case 0:
        NSS_DEBUG("found shadow entry by name %s", name);
        status = fill_shadow(spbuf, buf, buflen, &shadow, &rec, errnop);
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
    service_cleanup(&shadow);
    return status;
}

static enum nss_status
fill_shadow(struct spwd *spbuf, char *buf, size_t buflen,
            SERVICE *service, SHADOW_REC *rec, int *errnop)
{
    if(buflen < service->rec_size(service, (REC *) rec)) {
        *errnop = ERANGE;
        return NSS_STATUS_TRYAGAIN;
    }

    strcpy(buf, rec->name);
    spbuf->sp_namp = buf;
    buf += strlen(rec->name) + 1;

    strcpy(buf, rec->passwd);
    spbuf->sp_pwdp = buf;
    buf += strlen(rec->passwd) + 1;

    spbuf->sp_lstchg = rec->lstchg;
    spbuf->sp_min = rec->min;
    spbuf->sp_max = rec->max;
    spbuf->sp_warn = rec->warn;
    spbuf->sp_inact = rec->inact;
    spbuf->sp_expire = rec->expire;

    /* Not used. */
    spbuf->sp_flag = 0;

    return NSS_STATUS_SUCCESS;
}
