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
                         int (*key_creator)(DB *, const DBT *,
                                            const DBT *, DBT *),
                         int flags);

/**
 *
 */
extern void dbng_free(DBNG **handle);

#endif
