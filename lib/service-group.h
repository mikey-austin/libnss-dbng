/**
 * @file service-group.h
 * @brief Defines group service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef SERVICE_GROUP_H
#define SERVICE_GROUP_H

#include <grp.h>

#include "service.h"

#define GROUP_PRI "group.db"
#define GROUP_SEC "group-gid.db"

typedef struct GROUP_REC {
    REC base;
    char *name;
    char *passwd;
    gid_t gid;
    u_int32_t count;
    char **members;
} GROUP_REC;

typedef struct GROUP_KEY {
    KEY base;
    union {
        char *pri;
        gid_t sec;
    } data;
} GROUP_KEY;

/**
 *
 */
extern void service_group_init(SERVICE *service);

#endif
