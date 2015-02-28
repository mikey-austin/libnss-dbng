/**
 * @file service-passwd.c
 * @brief Implements passwd service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>

#include "service-passwd.h"

static void pack_key(PASSWD_KEY *key, DBT *dbkey);
static void pack_rec(PASSWD_REC *rec, DBT *dbrec);
static void unpack_key(PASSWD_KEY *key, DBT *dbkey);
static void unpack_rec(PASSWD_REC *rec, DBT *dbrec);

extern SERVICE
*service_passwd_create(void)
{
    return NULL;
}

static void
pack_key(PASSWD_KEY *key, DBT *dbkey)
{
    char *buf = NULL, *s;
    int len, slen = 0;

    len = sizeof(key->base.type)
        + (key->base.type == PRI
           ? (slen = (strlen(key->data.pri) + 1))
           : (slen = sizeof(key->data.sec)));
    buf = xcalloc(len, sizeof(char));
    memcpy(buf, &(key->base.type), sizeof(key->base.type));

    switch(key->base.type) {
    case PRI:
        memcpy(buf + sizeof(key->base.type), key->data.pri, slen);
        break;

    case SEC:
        memcpy(buf + sizeof(key->base.type), &key->data.sec, slen);
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

    len = sizeof(rec->uid)
        + sizeof(rec->gid)
        + strlen(rec->name) + 1
        + strlen(rec->passwd) + 1
        + strlen(rec->gecos) + 1
        + strlen(rec->shell) + 1
        + strlen(rec->homedir) + 1;
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
