#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"
#include <service.h>
#include <service-passwd.h>
#include <service-group.h>
#include <service-shadow.h>

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
        free(key);

void
add(service, raw)
    DBNG::Service service
    char *raw
    CODE:
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);
        int ret;

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
            out_ref = newRV_noinc((SV *) out);
        }
        free(key);
        free(rec);

        RETVAL = out_ref;
    OUTPUT:
        RETVAL        

SV *
group_get(service, pri_key)
    DBNG::Service service
    char *pri_key
    CODE:
        HV *out = newHV();
        AV *members = newAV();
        SV *out_ref = &PL_sv_undef;
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);
        GROUP_REC *grec;
        int i;

        service->key_init(service, key, PRI, (void *) pri_key);
        if(service->get(service, key, rec) == 0) {
            grec = (GROUP_REC *) rec;
            hv_store(out, "name", strlen("name"), newSVpv(grec->name, strlen(grec->name)), 0);
            hv_store(out, "gid", strlen("gid"), newSVuv(grec->gid), 0);
            hv_store(out, "passwd", strlen("passwd"), newSVpv(grec->passwd, strlen(grec->passwd)), 0);
            hv_store(out, "members", strlen("members"), newRV_noinc((SV *) members), 0);

            for(i = 0; i < grec->count; i++)
                av_push(members, newSVpv(grec->members[i], strlen(grec->members[i])));

            out_ref = newRV_noinc((SV *) out);
        }
        free(key);
        free(rec);

        RETVAL = out_ref;
    OUTPUT:
        RETVAL        

SV *
shadow_get(service, pri_key)
    DBNG::Service service
    char *pri_key
    CODE:
        HV *out = newHV();
        SV *out_ref = &PL_sv_undef;
        KEY *key = service->new_key(service);
        REC *rec = service->new_rec(service);
        SHADOW_REC *srec;

        service->key_init(service, key, PRI, (void *) pri_key);
        if(service->get(service, key, rec) == 0) {
            srec = (SHADOW_REC *) rec;
            hv_store(out, "name", strlen("name"), newSVpv(srec->name, strlen(srec->name)), 0);
            hv_store(out, "passwd", strlen("passwd"), newSVpv(srec->passwd, strlen(srec->passwd)), 0);
            hv_store(out, "lstchg", strlen("lstchg"), newSViv(srec->lstchg), 0);
            hv_store(out, "min", strlen("min"), newSViv(srec->min), 0);
            hv_store(out, "max", strlen("max"), newSViv(srec->max), 0);
            hv_store(out, "warn", strlen("warn"), newSViv(srec->warn), 0);
            hv_store(out, "inact", strlen("inact"), newSViv(srec->inact), 0);
            hv_store(out, "expire", strlen("expire"), newSViv(srec->expire), 0);
            out_ref = newRV_noinc((SV *) out);
        }
        free(key);
        free(rec);

        RETVAL = out_ref;
    OUTPUT:
        RETVAL        
