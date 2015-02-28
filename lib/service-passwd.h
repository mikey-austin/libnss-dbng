/**
 * @file service-passwd.h
 * @brief Defines passwd service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef SERVICE_PASSWD_H
#define SERVICE_PASSWD_H

#include "dbng/service.h"
#include "dbng/utils.h"

typedef struct PASSWD_REC {
    REC base;
    uid_t uid;
    gid_t gid;
    char *name;
    char *passwd;
    char *gecos;
    char *shell;
    char *homedir;
} PASSWD_REC;

typedef struct PASSWD_KEY {
    KEY base;
    union {
        char *pri;
        uid_t sec;
    } data;
} PASSWD_KEY;

/**
 *
 */
extern SERVICE *service_passwd_create(void);

#endif
