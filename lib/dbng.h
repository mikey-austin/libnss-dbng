/**
 * @file dbng.h
 * @brief Library to maintain databases and indexes.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef DBNG_H
#define DBNG_H

#ifdef HAVE_DB5_DB_H
#  include <db5/db.h>
#endif
#ifdef HAVE_DB4_DB_H
#  include <db4/db.h>
#endif
#ifdef HAVE_DB_H
#  include <db.h>
#endif

#include "nss-dbng.h"

#define DBNG_RO 0
#define DBNG_Rw 1
#define DBNG_PERMS 0600

typedef struct DBNG {
    DB_ENV *env;
    DB *pri;
    DB *sec;
    DB_TXN *txn;
} DBNG;

/**
 *
 */
extern DBNG *dbng_create(const char *pri,
                         const char *sec,
                         int (*key_creator)(DB *, DBT *, DBT *, DBT *),
                         int flags);

/**
 *
 */
extern void dbng_free(DBNG **handle);

/**
 *
 */
extern int dbng_insert(DBNG *handle, const char *key, const char *value);

/**
 *
 */
extern int dbng_get(DBNG *handle, const char *key);

/**
 *
 */
extern int dbng_delete(DBNG *handle, const char *key);

#endif
