#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"
#include <dbng/service.h>
#include <service-passwd.h>

typedef SERVICE * DBNG__Service;

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
    CODE:
        KEY *key = service->new_key(service);
        service->key_init(service, key, PRI, (void *) pri_key);
        if(service->delete(service, key) != 0)
            croak("could not delete %s", pri_key);

void
add(service, raw)
    DBNG::Service service
    char *raw
    CODE:
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);

        if(!service->parse(service, raw, key, rec))
            croak("could not parse passwd record");

        if(service->set(service, key, rec))
            croak("could not insert passwd record");            

SV *
passwd_get(service, pri_key)
    DBNG::Service service
    char *pri_key
    CODE:
        HV *out = newHV();
        SV *out_ref = &PL_sv_undef;
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);
        PASSWD_REC *prec;

        service->key_init(service, key, PRI, (void *) pri_key);
        if(service->get(service, key, rec) == 0) {
            prec = (PASSWD_REC *) rec;
            hv_store(out, "uid", strlen("uid"), newSVuv(prec->uid), 0);
            hv_store(out, "gid", strlen("gid"), newSVuv(prec->gid), 0);
            hv_store(out, "name", strlen("name"), newSVpv(prec->name, strlen(prec->name)), 0);
            hv_store(out, "passwd", strlen("passwd"), newSVpv(prec->passwd, strlen(prec->passwd)), 0);
            hv_store(out, "gecos", strlen("gecos"), newSVpv(prec->gecos, strlen(prec->gecos)), 0);
            hv_store(out, "shell", strlen("shell"), newSVpv(prec->shell, strlen(prec->shell)), 0);
            hv_store(out, "homedir", strlen("homedir"), newSVpv(prec->homedir, strlen(prec->homedir)), 0);
            out_ref = newRV_inc((SV *) out);
        }

        RETVAL = out_ref;
    OUTPUT:
        RETVAL        
