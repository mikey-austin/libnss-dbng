/**
 * @file service-shadow.h
 * @brief Defines shadow service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef SERVICE_SHADOW_H
#define SERVICE_SHADOW_H

#include "service.h"

#define SHADOW_PRI "shadow.db"

typedef struct SHADOW_REC {
    REC base;
    char *name;
    char *passwd;
    long int lstchg;
    long int min;
    long int max;
    long int warn;
    long int inact;
    long int expire;
} SHADOW_REC;

typedef struct SHADOW_KEY {
    KEY base;
    union {
        char *pri;
        uid_t sec;
    } data;
} SHADOW_KEY;

/**
 *
 */
extern void service_shadow_init(SERVICE *service);

#endif
