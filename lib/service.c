/**
 * @file service.c
 * @brief Implements service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include "service.h"
#include "utils.h"
#include "service-passwd.h"
#include "service-shadow.h"
#include "service-group.h"

/**
 *
 */
extern SERVICE
*service_create(enum TYPE type, int flags)
{
    SERVICE *service = NULL;

    switch(type)
    {
    case PASSWD:
        service = service_passwd_create();
        break;

    case SHADOW:
        service = service_shadow_create();
        break;

    case GROUP:
        service = service_group_create();
        break;

    default:
        warnx("unknown service type");
        goto err;
    }

    /* Initialize the database for this service. */
    service->db = dbng_create(service->pri,
                              service->sec,
                              service->key_creator,
                              flags);
    if(service->db == NULL)
        goto err;

    return service;

err:
    xfree(&service);
    return NULL;
}

/**
 *
 */
extern void service_free(SERVICE **service)
{
    if(service == NULL || *service == NULL)
        return;

    (*service)->cleanup(*service);
    dbng_free(&(*service)->db);
    xfree(service);
}
