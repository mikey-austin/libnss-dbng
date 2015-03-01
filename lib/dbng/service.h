/**
 * @file service.h
 * @brief Service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef SERVICE_H
#define SERVICE_H

#include "dbng/dbng.h"

enum TYPE {
    PASSWD,
    SHADOW,
    GROUP
};

enum KEY_TYPE {
    PRI,
    SEC
};

typedef struct REC {
    enum TYPE type;
} REC;

typedef struct KEY {
    enum KEY_TYPE type;
} KEY;

typedef struct SERVICE SERVICE;
struct SERVICE {
    char *pri;
    char *sec;
    DBNG *db;

    /* Callback to update secondary database. */
    int (*key_creator)(DB *, const DBT *, const DBT *, DBT *);
    void (*cleanup)(SERVICE *);
    int (*get)(SERVICE *, KEY *, REC *);
    int (*next)(SERVICE *, KEY *, REC *);
    int (*set)(SERVICE *, KEY *, REC *);
    int (*delete)(SERVICE *, KEY *);
    size_t (*rec_size)(SERVICE *, const REC *);
    size_t (*key_size)(SERVICE *, const KEY *);
    void (*pack_key)(SERVICE *, const KEY *, DBT *);
    void (*pack_rec)(SERVICE *, const REC *, DBT *);
    void (*unpack_key)(SERVICE *, KEY *, const DBT *);
    void (*unpack_rec)(SERVICE *, REC *, const DBT *);

    enum TYPE type;
};

/**
 *
 */
extern SERVICE *service_create(enum TYPE type, int flags, const char *base);

/**
 *
 */
extern void service_free(SERVICE **service);

/**
 *
 */
extern int service_get_rec(SERVICE *service, KEY *key, REC *rec);

/**
 *
 */
extern int service_set_rec(SERVICE *service, KEY *key, REC *rec);

/**
 *
 */
extern int service_next_rec(SERVICE *service, KEY *key, REC *rec);

/**
 *
 */
extern int service_delete_rec(SERVICE *service, KEY *key);

#endif
