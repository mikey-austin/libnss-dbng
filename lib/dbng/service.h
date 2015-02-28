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

typedef struct SERVICE {
    char *pri;
    char *sec;
    DBNG *db;
    int (*key_creator)(DB *, const DBT *, const DBT *, DBT *);
    void (*cleanup)(struct SERVICE *);
    enum TYPE type;
} SERVICE;

/**
 *
 */
extern SERVICE *service_create(enum TYPE type, int flags);

/**
 *
 */
extern void service_free(SERVICE **service);

#endif
