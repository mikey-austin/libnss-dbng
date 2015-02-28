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
    int (*insert)(SERVICE *, KEY *, REC *);
    void (*delete)(SERVICE *, KEY *);
    size_t (*rec_size)(SERVICE *, REC *);
    size_t (*key_size)(SERVICE *, KEY *);

    enum TYPE type;
};

/**
 *
 */
extern SERVICE *service_create(enum TYPE type, int flags);

/**
 *
 */
extern void service_free(SERVICE **service);

#endif
