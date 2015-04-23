/**
 * @file dbng.c
 * @brief Implements db utility functions.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>

#include "dbng/dbng.h"
#include "dbng/utils.h"

extern DBNG
*dbng_init(DBNG *handle, const char *base, const char *pri, const char *sec,
             int (*key_creator)(DB *, const DBT *, const DBT *,DBT *),
             int flags)
{
    int db_flags, ret;

    memset(handle, 0, sizeof(*handle));
    handle->txn    = NULL;
    handle->cursor = NULL;
    handle->env    = NULL;
    handle->pri    = NULL;
    handle->sec    = NULL;

    /* Open & setup environment. */
    db_flags = 0;

    if(!(flags & DBNG_RO)) {
        db_flags |= DB_CREATE
            | DB_INIT_MPOOL
            | DB_INIT_TXN
            | DB_INIT_LOCK;

        ret = db_env_create(&handle->env, 0);
        if(ret != 0) {
            warnx("error creating db environment: %s",
                  db_strerror(ret));
            goto err;
        }

        ret = handle->env->open(handle->env, base, db_flags, 0);
        if(ret != 0) {
            warnx("error opening db environment: %s",
                  db_strerror(ret));
            goto err;
        }
    }

    /* Open & setup primary database. */
    ret = db_create(&handle->pri, handle->env, 0);
    if(ret != 0) {
        warnx("error opening primary db: %s",
              db_strerror(ret));
        goto err;
    }

    db_flags = (flags & DBNG_RO ? DB_RDONLY : DB_CREATE | DB_AUTO_COMMIT);
    ret = handle->pri->open(handle->pri, NULL, pri, NULL, DB_BTREE,
                                db_flags, DBNG_PERMS);
    if(ret != 0) {
        warnx("db open (%s/%s) failed: %s", base, pri,
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
            warnx("db open (%s/%s) failed: %s", base, sec,
                  db_strerror(ret));
            goto err;
        }

        /* Associate the secondary with the primary. */
        handle->pri->associate(
            handle->pri, NULL, handle->sec,
            (flags & DBNG_RO ? NULL : key_creator), 0);
    }

    return handle;

err:
    if(handle->pri != NULL)
        handle->pri->close(handle->pri, 0);
    if(handle->env != NULL)
        handle->env->close(handle->env, 0);
    return NULL;
}

extern void
dbng_cleanup(DBNG *handle)
{
    if(handle != NULL) {
        if(handle->sec != NULL)
            handle->sec->close(handle->sec, 0);
        if(handle->pri != NULL)
            handle->pri->close(handle->pri, 0);
        if(handle->env != NULL)
            handle->env->close(handle->env, 0);
    }
}
