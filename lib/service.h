/**
 * @file service.h
 * @brief Service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef SERVICE_H
#define SERVICE_H

#include "dbng.h"

#define SERVICE_REC_MAX 1024

enum TYPE {
    TYPE_PASSWD,
    TYPE_SHADOW,
    TYPE_GROUP
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
    DBNG db;

    /* Callback to update secondary database. */
    int (*key_creator)(DB *, const DBT *, const DBT *, DBT *);
    void (*cleanup)(SERVICE *);
    int (*get)(SERVICE *, KEY *, REC *);
    int (*next)(SERVICE *, KEY *, REC *);
    int (*set)(SERVICE *, KEY *, REC *);
    void (*print)(SERVICE *, const KEY *, const REC *);
    int (*parse)(SERVICE *, const char *, KEY *, REC *);
    int (*delete)(SERVICE *, KEY *);
    int (*truncate)(SERVICE *);
    int (*start_txn)(SERVICE *);
    int (*commit)(SERVICE *);
    int (*rollback)(SERVICE *);
    size_t (*rec_size)(SERVICE *, const REC *);
    size_t (*key_size)(SERVICE *, const KEY *);
    KEY *(*new_key)(SERVICE *);
    REC *(*new_rec)(SERVICE *);
    void (*key_init)(SERVICE *, KEY *, enum KEY_TYPE, void *);
    void (*pack_key)(SERVICE *, const KEY *, DBT *);
    void (*pack_rec)(SERVICE *, const REC *, DBT *);
    void (*unpack_key)(SERVICE *, KEY *, const DBT *);
    void (*unpack_rec)(SERVICE *, REC *, const DBT *);
    int (*validate)(SERVICE *, const KEY *, const REC *);

    enum TYPE type;
};

/**
 *
 */
extern int service_init(SERVICE *service, enum TYPE type, int flags, const char *base);

/**
 *
 */
extern void service_cleanup(SERVICE *service);

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
extern int service_validate(SERVICE *service, const KEY *key, const REC *rec);

/**
 *
 */
extern int service_delete_rec(SERVICE *service, KEY *key);

/**
 *
 */
extern int service_truncate(SERVICE *service);

/**
 *
 */
extern int service_start_txn(SERVICE *service);

/**
 *
 */
extern int service_commit_txn(SERVICE *service);

/**
 *
 */
extern int service_rollback_txn(SERVICE *service);

#endif
