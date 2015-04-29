#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"
#include <service.h>
#include <service-passwd.h>
#include <service-group.h>
#include <service-shadow.h>

#include "utils.h"

MODULE = DBNG::Service		PACKAGE = DBNG::Service		
    
PROTOTYPES: ENABLE

BOOT:
{
    HV *stash;
    stash = gv_stashpv("DBNG::Service", TRUE);
    newCONSTSUB(stash, "TYPE_PASSWD", newSViv(TYPE_PASSWD));
    newCONSTSUB(stash, "TYPE_GROUP",  newSViv(TYPE_GROUP));
    newCONSTSUB(stash, "TYPE_SHADOW", newSViv(TYPE_SHADOW));
}

SV *
new(package, type, base)
    char *package
    enum TYPE type
    char *base
    INIT:
        SERVICE *s;
    CODE:
        s = malloc(sizeof(SERVICE));
        if(service_init(s, type, DBNG_RW, base) < 0)
            croak("could not initialize service");
        SV *addr = newSViv(PTR2IV(s));
        SV * const self = newRV_noinc(addr);
        RETVAL = sv_bless(self, gv_stashpv(package, 0));
    OUTPUT:
        RETVAL

void
DESTROY(service)
    DBNG::Service service
    CODE:
        service_cleanup(service);
        free(service);

void
truncate(service)
    DBNG::Service service
    CODE:
        service->truncate(service);

void
delete(service, pri_key)
    DBNG::Service service
    char *pri_key
    INIT:
        KEY *key = service->new_key(service);
    CODE:
        service->key_init(service, key, PRI, (void *) pri_key);
        if(service->delete(service, key) != 0)
            croak("could not delete %s", pri_key);
        free(key);

void
add(service, raw)
    DBNG::Service service
    char *raw
    INIT:
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);
        int ret;
    CODE:
        if(!service->parse(service, raw, key, rec))
            croak("could not parse record");

        if((ret = service->set(service, key, rec)) != 0) {
            if(ret > 0)
                croak("could not insert record, validation failed");
            else
                croak("could not insert record %s", db_strerror(ret));
        }
        free(key);
        free(rec);

SV *
get(service, pri_key)
    DBNG::Service service
    char *pri_key
    INIT:
        SV *out_ref = &PL_sv_undef;
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);        
    CODE:
        service->key_init(service, key, PRI, (void *) pri_key);
        if(service->get(service, key, rec) == 0)
            out_ref = format_rec(rec);
        free(key);
        free(rec);
        RETVAL = out_ref;
    OUTPUT:
        RETVAL        

SV *
next(service)
    DBNG::Service service
    INIT:
        SV *out_ref = &PL_sv_undef;
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);        
    CODE:
        if(service->next(service, key, rec) == 0)
            out_ref = format_rec(rec);
        free(key);
        free(rec);
        RETVAL = out_ref;
    OUTPUT:
        RETVAL        
