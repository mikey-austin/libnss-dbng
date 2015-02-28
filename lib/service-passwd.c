/**
 * @file service-passwd.c
 * @brief Implements passwd service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>

#include "service-passwd.h"

static void pack_key(PASSWD_KEY *, DBT *);
static void pack_rec(PASSWD_REC *, DBT *);
static void unpack_key(PASSWD_KEY *, DBT *);
static void unpack_rec(PASSWD_REC *, DBT *);
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
pack_key(PASSWD_KEY *key, DBT *dbkey)
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
pack_rec(PASSWD_REC *rec, DBT *dbrec)
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
unpack_key(PASSWD_KEY *key, DBT *dbkey)
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
unpack_rec(PASSWD_REC *rec, DBT *dbrec)
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
