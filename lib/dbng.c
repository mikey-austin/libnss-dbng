#include "dbng.h"
#include "utils.h"

static void get_db_name(enum TYPE, const char **, const char **);

extern DBNG
*dbng_create(enum TYPE type, int flags)
{
    DBNG *handle = NULL;
    char *main, *sec;
    int db_flags, ret;
    
    handle = xmalloc(sizeof(*handle));
    handle->env = handle->db_main = handle->db_sec = handle->txn = NULL;

    /* Open environment. */
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

    get_db_name(type, &main, &sec);
    if(main == NULL)
        goto err;

    ret = handle->env->open(handle->env, DEFAULT_BASE, db_flags, 0);
    if(ret != 0) {
        warnx("error opening db environment: %s",
              db_strerror(ret));
        goto err;
    }

    ret = db_create(&handle->db_main, handle->env, 0);
    if(ret != 0) {
        warnx("error opening main db: %s",
              db_strerror(ret));
        handle->env->close(handle->env, 0);
        goto err;
    }

    db_flags = (flags & DBNG_RO ? DB_RDONLY : DB_CREATE) | DB_AUTO_COMMIT;
    ret = handle->db_main->open(handle->db_main, NULL, main, NULL, DB_BTREE,
                                db_flags, 0600);
    if(ret != 0) {
        i_warning("db open (%s) failed: %s", db_name, db_strerror(ret));
        goto cleanup;
    }

    return handle;

err:
    xfree(&handle);
    return NULL;
}

extern void
dbng_free(DBNG **handle)
{
    xfree(handle);
}

extern int
dbng_insert(DBNG *handle, const char *key, const char *value)
{
    return 0;
}

extern int
dbng_delete(DBNG *handle, const char *key)
{
    return 0;
}

extern int
dbng_fetch_all(DBNG *handle, struct DBNG_REC **values, int count)
{
    return 0;
}

/**
 * Fetch the main and secondary database paths for the specified
 * service type.
 */
static void
get_db_name(enum TYPE type, const char **main, const char **sec)
{
    switch(type)
    {
    case PASSWD:
        *main = DBNG_PASSWD;
        *sec = DBNG_PASSWD_UID;
        break;

    case SHADOW:
        *main = DBNG_SHADOW;
        *sec = DBNG_SHADOW_UID;
        break;

    case GROUP:
        *main = DBNG_GROUP;
        *sec = DBNG_GROUP_UID;
        break;

    default:
        *main = *sec = NULL;
    }
}
