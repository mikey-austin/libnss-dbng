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
#include "service-group.h"

extern SERVICE
*service_create(enum TYPE type, int flags, const char *base)
{
    SERVICE *service = NULL;

    switch(type)
    {
    case PASSWD:
        service = service_passwd_create();
        break;

    case GROUP:
        service = service_group_create();
        break;

    default:
        warnx("unknown service type");
        goto err;
    }

    /* Initialize the database for this service. */
    service->db = dbng_create(base,
                              service->pri,
                              service->sec,
                              service->key_creator,
                              flags);
    if(service->db == NULL)
        goto err;

    return service;

err:
    xfree((void **) &service);
    return NULL;
}

extern void
service_free(SERVICE **service)
{
    if(service == NULL || *service == NULL)
        return;

    if((*service)->cleanup != NULL)
        (*service)->cleanup(*service);

    dbng_free(&(*service)->db);
    xfree((void **) service);
}

extern int
service_get_rec(SERVICE *service, KEY *key, REC *rec)
{
    int ret;
    DBT dbkey, dbval;
    DB *db = (key->type == PRI
              ? service->db->pri : service->db->sec);

    service->pack_key(service, key, &dbkey);
    memset(&dbval, 0, sizeof(dbval));

    ret = db->get(db, service->db->txn, &dbkey, &dbval, 0);
    if(ret == 0)
        service->unpack_rec(service, rec, &dbval);

    /* Cleanup packed key data. */
    xfree((void **) &dbkey.data);

    return ret;
}

extern int
service_set_rec(SERVICE *service, KEY *key, REC *rec)
{
    int ret;
    DBT dbkey, dbrec;
    DB *db = service->db->pri; /* Secondary database updated automatically. */

    service->pack_key(service, key, &dbkey);
    service->pack_rec(service, rec, &dbrec);
    ret = db->put(db, service->db->txn, &dbkey, &dbrec, 0);
    xfree((void **) &dbkey.data);
    xfree((void **) &dbrec.data);

    return ret;
}

extern int
service_delete_rec(SERVICE *service, KEY *key)
{
    int ret;
    DBT dbkey;
    DB *db = service->db->pri; /* Secondary database updated automatically. */

    service->pack_key(service, key, &dbkey);
    ret = db->del(db, service->db->txn, &dbkey, 0);
    xfree((void **) &dbkey.data);

    return ret;
}

extern int
service_next_rec(SERVICE *service, KEY *key, REC *rec)
{
    int ret;
    DBT dbkey, dbval;
    DB *db = service->db->pri;
    DBC *cursor;

    /* If the cursor is not set, create a new one. */
    if(service->db->cursor == NULL) {
        ret = db->cursor(db, service->db->txn, &service->db->cursor, 0);
        if(ret != 0) {
            service->db->cursor = NULL;
            return ret;
        }
    }

    cursor = service->db->cursor;
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
        service->db->cursor = NULL;
        break;
    }

    return ret;
}

extern int
service_truncate(SERVICE *service)
{
    int ret, truncated;
    DB *db = service->db->pri; /* Secondary database updated automatically. */

    ret = db->truncate(db, service->db->txn, &truncated, 0);

    return ret;
}

extern int
service_start_txn(SERVICE *service)
{
    int ret;

    if(service->db->txn != NULL)
        return -1;

    ret = service->db->env->txn_begin(service->db->env, NULL,
                                      &service->db->txn, 0);
    if(ret != 0)
        warnx("could not create transaction");

    return ret;
}

extern int
service_commit_txn(SERVICE *service)
{
    int ret;

    if(service->db->txn == NULL)
        return -1;

    ret = service->db->txn->commit(service->db->txn, 0);
    if(ret != 0)
        warnx("could not commit transaction");
    service->db->txn = NULL;

    return ret;
}

extern int
service_rollback_txn(SERVICE *service)
{
    int ret;

    if(service->db->txn == NULL)
        return -1;

    ret = service->db->txn->abort(service->db->txn);
    if(ret != 0)
        warnx("could not rollback transaction");
    service->db->txn = NULL;

    return ret;
}
