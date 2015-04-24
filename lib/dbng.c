/**
 * @file dbng.c
 * @brief Implements db utility functions.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>

#include "dbng/dbng.h"
#include "dbng/utils.h"

#define MAX_PATH 256

extern int
dbng_init(DBNG *handle, const char *base, const char *pri, const char *sec,
          int (*key_creator)(DB *, const DBT *, const DBT *,DBT *),
          int flags, int perms)
{
    int db_flags, ret;
    char pri_path[MAX_PATH];
    char sec_path[MAX_PATH];
    char *sep;

    strncpy(pri_path, base, MAX_PATH);
    if((sep = strrchr(base, '/')) != NULL && *(sep + 1))
        strncat(pri_path, "/", MAX_PATH);
    strncat(pri_path, pri, MAX_PATH);

    memset(handle, 0, sizeof(*handle));
    handle->txn    = NULL;
    handle->cursor = NULL;
    handle->env    = NULL;
    handle->pri    = NULL;
    handle->sec    = NULL;

    /* Open & setup primary database. */
    ret = db_create(&handle->pri, handle->env, 0);
    if(ret != 0) {
        warnx("error opening primary db: %s",
              db_strerror(ret));
        goto err;
    }

    db_flags = (flags & DBNG_RO ? DB_RDONLY : DB_CREATE);
    ret = handle->pri->open(handle->pri, NULL, pri_path, NULL, DB_BTREE,
                            db_flags, perms);
    if(ret != 0) {
        warnx("db open (%s) failed: %s", pri_path, db_strerror(ret));
        goto err;
    }

    /* Open & setup secondary database if it exists. */
    if(sec != NULL) {
        strncpy(sec_path, base, MAX_PATH);
        if((sep = strrchr(base, '/')) != NULL && *(sep + 1))
            strncat(sec_path, "/", MAX_PATH);
        strncat(sec_path, sec, MAX_PATH);

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

        ret = handle->sec->open(handle->sec, NULL, sec_path, NULL, DB_BTREE,
                                db_flags, perms);
        if(ret != 0) {
            warnx("db open (%s) failed: %s", sec_path, db_strerror(ret));
            goto err;
        }

        /* Associate the secondary with the primary. */
        handle->pri->associate(
            handle->pri, NULL, handle->sec,
            (flags & DBNG_RO ? NULL : key_creator), 0);
    }

    return 0;

err:
    if(handle->pri != NULL)
        handle->pri->close(handle->pri, 0);
    if(handle->sec != NULL)
        handle->sec->close(handle->sec, 0);
    if(handle->env != NULL)
        handle->env->close(handle->env, 0);
    return -1;
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
