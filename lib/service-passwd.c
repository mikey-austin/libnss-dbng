/**
 * @file service-passwd.c
 * @brief Implements passwd service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>
#include <regex.h>

#include "service-passwd.h"

#define ERRBUFLEN 256
#define NMATCH    7   /* A match per passwd column. */

static void print(SERVICE *, const KEY *, const REC *);
static int parse(SERVICE *, const char *, KEY *, REC *);
static KEY *new_key(SERVICE *);
static REC *new_rec(SERVICE *);
static void pack_key(SERVICE *, const KEY *, DBT *);
static void pack_rec(SERVICE *, const REC *, DBT *);
static void unpack_key(SERVICE *, KEY *, const DBT *);
static void unpack_rec(SERVICE *, REC *, const DBT *);
static int key_creator(DB *, const DBT *, const DBT *, DBT *);
static size_t rec_size(SERVICE *, const REC *);
static size_t key_size(SERVICE *, const KEY *);

extern SERVICE
*service_passwd_create(void)
{
    SERVICE *service = NULL;

    service = xmalloc(sizeof(*service));
    memset(service, 0, sizeof(*service));
    service->type = PASSWD;
    service->pri = PASSWD_PRI;
    service->sec = PASSWD_SEC;

    /* Set implemented functions. */
    service->print = print;
    service->parse = parse;
    service->key_creator = key_creator;
    service->pack_key = pack_key;
    service->unpack_key = unpack_key;
    service->pack_rec = pack_rec;
    service->unpack_rec = unpack_rec;
    service->rec_size = rec_size;
    service->key_size = key_size;
    service->new_key = new_key;
    service->new_rec = new_rec;
    service->cleanup = NULL;

    /* Set inherited functions. */
    service->get = service_get_rec;
    service->set = service_set_rec;
    service->next = service_next_rec;
    service->delete = service_delete_rec;
    service->truncate = service_truncate;
    service->start_txn = service_start_txn;
    service->commit = service_commit_txn;
    service->rollback = service_rollback_txn;

    return (SERVICE *) service;
}

static void
print(SERVICE *service, const KEY *key, const REC *rec)
{
    const PASSWD_KEY *pkey = (const PASSWD_KEY *) key;
    const PASSWD_REC *prec = (const PASSWD_REC *) rec;

    printf("%s:%s:%ld:%ld:%s:%s:%s\n",
           prec->name, prec->passwd,
           (unsigned long) prec->uid,
           (unsigned long) prec->gid,
           prec->gecos, prec->homedir,
           prec->shell);
}

static int
parse(SERVICE *service, const char *raw, KEY *key, REC *rec)
{
    PASSWD_KEY *pkey = (PASSWD_KEY *) key;
    PASSWD_REC *prec = (PASSWD_REC *) rec;
    int ret, res = 0, i, len;
    regex_t regex;
    regmatch_t matches[NMATCH + 1];
    char err_buf[ERRBUFLEN];
    static char buf[SERVICE_REC_MAX];
    size_t remaining = sizeof(buf);
    char *p_buf = buf;

    memset(&regex, 0, sizeof(regex));
    memset(pkey, 0, sizeof(*pkey));
    memset(prec, 0, sizeof(*prec));
    memset(&buf, 0, sizeof(buf));
    pkey->base.type = PRI;
    prec->base.type = PASSWD;

    ret = regcomp(&regex,
                  "([^:]+):"        /* user. */
                  "([^:]+):"        /* passwd. */
                  "([[:digit:]]+):" /* uid. */
                  "([[:digit:]]+):" /* gid. */
                  "([^:]*):"        /* gecos (may be empty). */
                  "([^:]+):"        /* homedir. */
                  "([^:]+)$",       /* shell. */
                  REG_EXTENDED);
    if(ret != 0) {
        regerror(ret, &regex, err_buf, ERRBUFLEN);
        warnx("regcomp: %s", err_buf);
        goto cleanup;
    }

    ret = regexec(&regex, raw, NMATCH + 1, matches, 0);
    if(ret != 0) {
        if(ret != REG_NOMATCH) {
            regerror(ret, &regex, err_buf, ERRBUFLEN);
            warnx("regcomp: %s", err_buf);
        }
        goto cleanup;
    }
    else {
        /* Successful match. */
        for(i = 1; i < NMATCH + 1; i++) {
            len = matches[i].rm_eo - matches[i].rm_so;
            if((remaining -= (len + 1)) <= 0) {
                warnx("regexec: parsed record too large");
                goto cleanup;
            }
            strncpy(p_buf, (raw + matches[i].rm_so), len);
            p_buf[len] = '\0';

            switch(i) {
            case 1:
                pkey->data.pri = prec->name = p_buf;
                break;

            case 2:
                prec->passwd = p_buf;
                break;

            case 3:
                prec->uid = atoi(p_buf);
                break;

            case 4:
                prec->gid = atoi(p_buf);
                break;

            case 5:
                prec->gecos = p_buf;
                break;

            case 6:
                prec->homedir = p_buf;
                break;

            case 7:
                prec->shell = p_buf;
                break;
            }

            p_buf += (len + 1);
        }

        res = 1;
    }

cleanup:
    regfree(&regex);
    return res;
}

static KEY
*new_key(SERVICE *service)
{
    return xmalloc(sizeof(PASSWD_KEY));
}

static REC
*new_rec(SERVICE *service)
{
    return xmalloc(sizeof(PASSWD_REC));
}

static int
key_creator(DB *dbp, const DBT *pkey, const DBT *pdata, DBT *skey)
{
    PASSWD_KEY key;
    PASSWD_REC rec;

    /* Create the secondary index on the uid. */
    unpack_rec(NULL, (REC *) &rec, pdata);
    key.base.type = SEC;
    key.data.sec = rec.uid;
    pack_key(NULL, (KEY *) &key, skey);
    skey->flags = DB_DBT_APPMALLOC;

    return 0;
}

static size_t
rec_size(SERVICE *service, const REC *rec)
{
    PASSWD_REC *prec = (PASSWD_REC *) rec;

    return sizeof(prec->uid)
        + sizeof(prec->gid)
        + strlen(prec->name) + 1
        + strlen(prec->passwd) + 1
        + strlen(prec->gecos) + 1
        + strlen(prec->shell) + 1
        + strlen(prec->homedir) + 1;
}

static size_t
key_size(SERVICE *service, const KEY *key)
{
    PASSWD_KEY *pkey = (PASSWD_KEY *) key;

    return sizeof(pkey->base.type)
        + (pkey->base.type == PRI
           ? (strlen(pkey->data.pri) + 1)
           : sizeof(pkey->data.sec));
}

static void
pack_key(SERVICE *service, const KEY *key, DBT *dbkey)
{
    PASSWD_KEY *pkey = (PASSWD_KEY *) key;
    char *buf = NULL, *s;
    int len;

    len = key_size(NULL, key);
    buf = xcalloc(len, sizeof(char));
    memcpy(buf, &(pkey->base.type), sizeof(pkey->base.type));

    switch(pkey->base.type) {
    case PRI:
        memcpy(buf + sizeof(pkey->base.type), pkey->data.pri, strlen(pkey->data.pri) + 1);
        break;

    case SEC:
        memcpy(buf + sizeof(pkey->base.type), &pkey->data.sec, sizeof(pkey->data.sec));
        break;
    }

    memset(dbkey, 0, sizeof(*dbkey));
    dbkey->data = buf;
    dbkey->size = len;
}

static void
pack_rec(SERVICE *service, const REC *rec, DBT *dbrec)
{
    PASSWD_REC *prec = (PASSWD_REC *) rec;
    char *buf = NULL, *s;
    int len, slen = 0;

    len = rec_size(NULL, rec);
    buf = xcalloc(sizeof(char), len);

    s = buf;
    memcpy(s, &prec->uid, (slen = sizeof(prec->uid)));
    s += slen;

    memcpy(s, &prec->gid, (slen = sizeof(prec->gid)));
    s += slen;

    memcpy(s, prec->name, (slen = (strlen(prec->name) + 1)));
    s += slen;

    memcpy(s, prec->passwd, (slen = (strlen(prec->passwd) + 1)));
    s += slen;

    memcpy(s, prec->gecos, (slen = (strlen(prec->gecos) + 1)));
    s += slen;

    memcpy(s, prec->shell, (slen = (strlen(prec->shell) + 1)));
    s += slen;

    memcpy(s, prec->homedir, (slen = (strlen(prec->homedir) + 1)));
    s += slen;

    memset(dbrec, 0, sizeof(*dbrec));
    dbrec->data = buf;
    dbrec->size = len;
}

static void
unpack_key(SERVICE *service, KEY *key, const DBT *dbkey)
{
    PASSWD_KEY *pkey = (PASSWD_KEY *) key;
    char *buf = (char *) dbkey->data;

    memset(pkey, 0, sizeof(*pkey));
    pkey->base.type = *((enum KEY_TYPE *) buf);
    buf += sizeof(pkey->base.type);

   switch(pkey->base.type) {
    case PRI:
        pkey->data.pri = buf;
        break;

    case SEC:
        memcpy(&pkey->data.sec, buf, sizeof(pkey->data.sec));
        break;
    }
}

static void
unpack_rec(SERVICE *service, REC *rec, const DBT *dbrec)
{
    PASSWD_REC *prec = (PASSWD_REC *) rec;
    char *buf = (char *) dbrec->data;

    memset(prec, 0, sizeof(*prec));
    prec->base.type = PASSWD;

    memcpy(&prec->uid, buf, sizeof(prec->uid));
    buf += sizeof(prec->uid);

    memcpy(&prec->gid, buf, sizeof(prec->gid));
    buf += sizeof(prec->gid);

    prec->name = buf;
    buf += strlen(prec->name) + 1;

    prec->passwd = buf;
    buf += strlen(prec->passwd) + 1;

    prec->gecos = buf;
    buf += strlen(prec->gecos) + 1;

    prec->shell = buf;
    buf += strlen(prec->shell) + 1;

    prec->homedir = buf;
    buf += strlen(prec->homedir) + 1;
}
