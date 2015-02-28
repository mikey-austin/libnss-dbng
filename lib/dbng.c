/**
 * @file dbng.c
 * @brief Implements db utility functions.
 * @author Mikey Austin
 * @date 2015
 */

#include "dbng/dbng.h"
#include "dbng/utils.h"

extern DBNG
*dbng_create(const char *pri, const char *sec,
             int (*key_creator)(DB *, const DBT *, const DBT *,DBT *),
             int flags)
{
    DBNG *handle = NULL;
    int db_flags, ret;

    handle = xmalloc(sizeof(*handle));
    handle->env = NULL;
    handle->pri = NULL;
    handle->sec = NULL;
    handle->txn = NULL;

    /* Open & setup environment. */
    db_flags = DB_CREATE
        | DB_INIT_TXN
        | DB_INIT_LOCK
        | DB_INIT_LOG
        | DB_INIT_MPOOL;

    ret = db_env_create(&handle->env, 0);
    if(ret != 0) {
        warnx("error creating db environment: %s",
              db_strerror(ret));
        goto err;
    }

    ret = handle->env->open(handle->env, DEFAULT_BASE, db_flags, 0);
    if(ret != 0) {
        warnx("error opening db environment: %s",
              db_strerror(ret));
        goto err;
    }

    /* Open & setup primary database. */
    ret = db_create(&handle->pri, handle->env, 0);
    if(ret != 0) {
        warnx("error opening primary db: %s",
              db_strerror(ret));
        goto err;
    }

    db_flags = (flags & DBNG_RO ? DB_RDONLY : DB_CREATE) | DB_AUTO_COMMIT;
    ret = handle->pri->open(handle->pri, NULL, pri, NULL, DB_BTREE,
                                db_flags, DBNG_PERMS);
    if(ret != 0) {
        warnx("db open (%s/%s) failed: %s", DEFAULT_BASE, pri,
              db_strerror(ret));
        goto err;
    }

    /* Open & setup secondary database if it exists. */
    if(sec != NULL) {
        ret = db_create(&handle->sec, handle->env, 0);
        if(ret != 0) {
            warnx("error opening secondary db: %s", db_strerror(ret));
            goto err;
        }

        ret = handle->sec->set_flags(handle->sec, DB_DUPSORT);
        if(ret != 0) {
            warnx("set_flags secondary db: %s", db_strerror(ret));
            goto err;
        }

        ret = handle->sec->open(handle->sec, NULL, sec, NULL, DB_BTREE,
                                db_flags, DBNG_PERMS);
        if(ret != 0) {
            warnx("db open (%s/%s) failed: %s", DEFAULT_BASE, sec,
                  db_strerror(ret));
            goto err;
        }

        /* Associate the secondary with the primary. */
        handle->pri->associate(handle->pri, NULL, handle->sec,
                               key_creator, 0);
    }

    return handle;

err:
    if(handle->pri != NULL)
        handle->pri->close(handle->pri, 0);
    if(handle->env != NULL)
        handle->env->close(handle->env, 0);
    xfree((void **) &handle);
    return NULL;
}

extern void
dbng_free(DBNG **handle)
{
    if(*handle != NULL) {
        if((*handle)->sec != NULL)
            (*handle)->sec->close((*handle)->sec, 0);
        if((*handle)->pri != NULL)
            (*handle)->pri->close((*handle)->pri, 0);
        if((*handle)->env != NULL)
            (*handle)->env->close((*handle)->env, 0);
        xfree((void **) handle);
    }
}
