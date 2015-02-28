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

enum TYPE {
    PASSWD,
    SHADOW,
    GROUP
};

typedef struct DBNG DBNG;
struct DBNG {
    enum TYPE type;
    DB_ENV *env;
    DB *db_main;
    DB *db_sec;
    DB_TXN *txn;
};

struct DBNG_REC {
    char *key;
    char *value;
};

extern DBNG *dbng_create(enum TYPE type);
extern void dbng_free(DBNG **handle);
extern int dbng_insert(DBNG *handle, const char *key, const char *value);
extern int dbng_delete(DBNG *handle, const char *key);
extern int dbng_fetch_all(DBNG *handle, struct DBNG_REC **values, int count);

#endif
