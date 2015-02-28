/**
 * @file service-passwd.c
 * @brief Implements passwd service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>

#include "service-passwd.h"

static void pack_key(const PASSWD_KEY *, DBT *);
static void pack_rec(const PASSWD_REC *, DBT *);
static void unpack_key(PASSWD_KEY *, const DBT *);
static void unpack_rec(PASSWD_REC *, const DBT *);
static int key_creator(DB *, const DBT *, const DBT *, DBT *);
static void cleanup(SERVICE *);
static int get(SERVICE *, KEY *, REC *);
static int next(SERVICE *, KEY *, REC *);
static int insert(SERVICE *, KEY *, REC *);
static void delete(SERVICE *, KEY *);
static size_t rec_size(SERVICE *, REC *);
static size_t key_size(SERVICE *, KEY *);

extern SERVICE
*service_passwd_create(void)
{
    SERVICE *service = NULL;

    service = xmalloc(sizeof(*service));
    service->type = PASSWD;
    service->rec_size = rec_size;
    service->key_size = key_size;

    return (SERVICE *) service;
}

static int
key_creator(DB *dbp, const DBT *pkey, const DBT *pdata, DBT *skey)
{
    PASSWD_KEY key;
    PASSWD_REC rec;

    /* Create the secondary index on the uid. */
    unpack_rec(&rec, pdata);
    key.base.type = SEC;
    key.data.sec = rec.uid;
    pack_key(&key, skey);

    return 0;
}

static void
cleanup(SERVICE *service)
{
    /* NOOP. */
}

static int
get(SERVICE *service, KEY *key, REC *rec)
{
    int ret;
    PASSWD_REC *prec = (PASSWD_REC *) rec;
    PASSWD_KEY *pkey = (PASSWD_KEY *) key;
    DBT dbkey, dbval;
    DB *db = (pkey->base.type == PRI
              ? service->db->pri : service->db->sec);

    pack_key(pkey, &dbkey);
    memset(&dbval, 0, sizeof(dbval));

    ret = db->get(db, service->db->txn, &dbkey, &dbval, 0);
    if(ret == 0)
        unpack_rec(prec, &dbval);

    /* Cleanup packed key data. */
    free(dbkey.data);

    return ret;
}

static int
next(SERVICE *service, KEY *key, REC *rec)
{
    return 0;
}

static int
insert(SERVICE *service, KEY *key, REC *rec)
{
    return -1;
}

static void
delete(SERVICE *service, KEY *key)
{
}

static size_t
rec_size(SERVICE *service, REC *rec)
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
key_size(SERVICE *service, KEY *key)
{
    PASSWD_KEY *pkey = (PASSWD_KEY *) key;

    return sizeof(pkey->base.type)
        + (pkey->base.type == PRI
           ? (strlen(pkey->data.pri) + 1)
           : sizeof(pkey->data.sec));
}

static void
pack_key(const PASSWD_KEY *key, DBT *dbkey)
{
    char *buf = NULL, *s;
    int len;

    len = key_size(NULL, (KEY *) key);
    buf = xcalloc(len, sizeof(char));
    memcpy(buf, &(key->base.type), sizeof(key->base.type));

    switch(key->base.type) {
    case PRI:
        memcpy(buf + sizeof(key->base.type), key->data.pri, strlen(key->data.pri) + 1);
        break;

    case SEC:
        memcpy(buf + sizeof(key->base.type), &key->data.sec, sizeof(key->data.sec));
        break;
    }

    memset(dbkey, 0, sizeof(*dbkey));
    dbkey->data = buf;
    dbkey->size = len;
}

static void
pack_rec(const PASSWD_REC *rec, DBT *dbrec)
{
    char *buf = NULL, *s;
    int len, slen = 0;

    len = rec_size(NULL, (REC *) rec);
    buf = xcalloc(sizeof(char), len);

    s = buf;
    memcpy(s, &rec->uid, (slen = sizeof(rec->uid)));
    s += slen;

    memcpy(s, &rec->gid, (slen = sizeof(rec->gid)));
    s += slen;

    memcpy(s, rec->name, (slen = (strlen(rec->name) + 1)));
    s += slen;

    memcpy(s, rec->passwd, (slen = (strlen(rec->passwd) + 1)));
    s += slen;

    memcpy(s, rec->gecos, (slen = (strlen(rec->gecos) + 1)));
    s += slen;

    memcpy(s, rec->shell, (slen = (strlen(rec->shell) + 1)));
    s += slen;

    memcpy(s, rec->homedir, (slen = (strlen(rec->homedir) + 1)));
    s += slen;

    memset(dbrec, 0, sizeof(*dbrec));
    dbrec->data = buf;
    dbrec->size = len;
}

static void
unpack_key(PASSWD_KEY *key, const DBT *dbkey)
{
    char *buf = (char *) dbkey->data;

    memset(key, 0, sizeof(*key));
    key->base.type = *((enum KEY_TYPE *) buf);
    buf += sizeof(key->base.type);

   switch(key->base.type) {
    case PRI:
        key->data.pri = buf;
        break;

    case SEC:
        memcpy(&key->data.sec, buf, sizeof(key->data.sec));
        break;
    }
}

static void
unpack_rec(PASSWD_REC *rec, const DBT *dbrec)
{
    char *buf = (char *) dbrec->data;

    memset(rec, 0, sizeof(*rec));
    rec->base.type = PASSWD;

    memcpy(&rec->uid, buf, sizeof(rec->uid));
    buf += sizeof(rec->uid);

    memcpy(&rec->gid, buf, sizeof(rec->gid));
    buf += sizeof(rec->gid);

    rec->name = buf;
    buf += strlen(rec->name) + 1;

    rec->passwd = buf;
    buf += strlen(rec->passwd) + 1;

    rec->gecos = buf;
    buf += strlen(rec->gecos) + 1;

    rec->shell = buf;
    buf += strlen(rec->shell) + 1;

    rec->homedir = buf;
    buf += strlen(rec->homedir) + 1;
}
