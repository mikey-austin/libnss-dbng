/**
 * @file service.c
 * @brief Implements service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>

#include "dbng/service.h"
#include "dbng/utils.h"
#include "service-passwd.h"
#include "service-shadow.h"
#include "service-group.h"

extern int
service_init(SERVICE *service, enum TYPE type, int flags, const char *base)
{
    int perms = 0644;

    switch(type)
    {
    case TYPE_PASSWD:
        service_passwd_init(service);
        break;

    case TYPE_SHADOW:
        perms = 0600;
        service_shadow_init(service);
        break;

    case TYPE_GROUP:
        service_group_init(service);
        break;

    default:
        warnx("unknown service type");
        goto err;
    }

    /* Initialize the database for this service. */
    return dbng_init(&service->db, base, service->pri, service->sec,
                     service->key_creator, flags, perms);

err:
    return -1;
}

extern void
service_cleanup(SERVICE *service)
{
    dbng_cleanup(&service->db);
}

extern int
service_get_rec(SERVICE *service, KEY *key, REC *rec)
{
    int ret;
    DBT dbkey, dbval;
    DB *db = (key->type == PRI
              ? service->db.pri : service->db.sec);
    int ksize = service->key_size(service, key);
    unsigned char kbuf[ksize];

    memset(kbuf, 0, ksize);
    memset(&dbkey, 0, sizeof(dbkey));
    dbkey.data = kbuf;
    dbkey.size = ksize;
    service->pack_key(service, key, &dbkey);
    memset(&dbval, 0, sizeof(dbval));

    ret = db->get(db, service->db.txn, &dbkey, &dbval, 0);
    if(ret == 0)
        service->unpack_rec(service, rec, &dbval);

    return ret;
}

extern int
service_set_rec(SERVICE *service, KEY *key, REC *rec)
{
    int ret;
    DBT dbkey, dbrec;
    DB *db = service->db.pri; /* Secondary database updated automatically. */
    int ksize = service->key_size(service, key);
    int rsize = service->rec_size(service, rec);
    unsigned char kbuf[ksize];
    unsigned char rbuf[rsize];

    memset(&dbkey, 0, sizeof(dbkey));
    memset(kbuf, 0, ksize);
    dbkey.data = kbuf;
    dbkey.size = ksize;

    memset(&dbrec, 0, sizeof(dbrec));
    memset(rbuf, 0, rsize);
    dbrec.data = rbuf;
    dbrec.size = rsize;
    
    service->pack_key(service, key, &dbkey);
    service->pack_rec(service, rec, &dbrec);
    ret = db->put(db, service->db.txn, &dbkey, &dbrec, 0);

    return ret;
}

extern int
service_delete_rec(SERVICE *service, KEY *key)
{
    int ret;
    DBT dbkey;
    DB *db = service->db.pri; /* Secondary database updated automatically. */
    int ksize = service->key_size(service, key);
    unsigned char kbuf[ksize];

    memset(kbuf, 0, ksize);
    memset(&dbkey, 0, sizeof(dbkey));
    dbkey.data = kbuf;
    dbkey.size = ksize;
    service->pack_key(service, key, &dbkey);
    ret = db->del(db, service->db.txn, &dbkey, 0);

    return ret;
}

extern int
service_next_rec(SERVICE *service, KEY *key, REC *rec)
{
    int ret;
    DBT dbkey, dbval;
    DB *db = service->db.pri;
    DBC *cursor;

    /* If the cursor is not set, create a new one. */
    if(service->db.cursor == NULL) {
        ret = db->cursor(db, service->db.txn, &service->db.cursor, 0);
        if(ret != 0) {
            service->db.cursor = NULL;
            return ret;
        }
    }

    cursor = service->db.cursor;
    memset(&dbkey, 0, sizeof(dbkey));
    memset(&dbval, 0, sizeof(dbval));

    ret = cursor->get(cursor, &dbkey, &dbval, DB_NEXT);
    switch(ret) {
    case 0:
        service->unpack_key(service, key, &dbkey);
        service->unpack_rec(service, rec, &dbval);
        break;

    case DB_NOTFOUND:
        /* We have reached the end of the iterator. */
        cursor->close(cursor);
        service->db.cursor = NULL;
        break;
    }

    return ret;
}

extern int
service_truncate(SERVICE *service)
{
    int ret, truncated;
    DB *db = service->db.pri; /* Secondary database updated automatically. */

    ret = db->truncate(db, service->db.txn, &truncated, 0);

    return ret;
}

extern int
service_start_txn(SERVICE *service)
{
    return 0;
}

extern int
service_commit_txn(SERVICE *service)
{
    return 0;
}

extern int
service_rollback_txn(SERVICE *service)
{
    return 0;
}
