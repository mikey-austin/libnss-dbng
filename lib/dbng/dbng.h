/**
 * @file dbng.h
 * @brief Library to maintain databases and indexes.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef DBNG_H
#define DBNG_H

#include "../nss/nss-dbng.h"

#ifdef HAVE_DB5_DB_H
#  include <db5/db.h>
#endif
#ifdef HAVE_DB4_DB_H
#  include <db4/db.h>
#endif
#ifdef HAVE_DB_H
#  include <db.h>
#endif

#define DBNG_RO 1
#define DBNG_RW 2
#define DBNG_PERMS 0644

typedef struct DBNG {
    DB_TXN *txn;
    DB_ENV *env;
    DB *pri;
    DB *sec;
    DBC *cursor;
} DBNG;

/**
 *
 */
extern DBNG *dbng_create(DBNG *handle,
                         const char *base,
                         const char *pri,
                         const char *sec,
                         int (*key_creator)(DB *, const DBT *, const DBT *, DBT *),
                         int flags);

/**
 *
 */
extern void dbng_cleanup(DBNG *handle);

#endif
