#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "utils.h"

#include <service-passwd.h>
#include <service-shadow.h>
#include <service-group.h>

SV *
format_rec(REC *rec)
{
    HV *out = newHV();
    AV *members = newAV();
    SV *out_ref = &PL_sv_undef;
    PASSWD_REC *prec;
    GROUP_REC *grec;
    SHADOW_REC *srec;
    int i;

    switch(rec->type) {
    case TYPE_PASSWD:
        prec = (PASSWD_REC *) rec;
        hv_store(out, "uid", strlen("uid"), newSVuv(prec->uid), 0);
        hv_store(out, "gid", strlen("gid"), newSVuv(prec->gid), 0);
        hv_store(out, "name", strlen("name"), newSVpv(prec->name, strlen(prec->name)), 0);
        hv_store(out, "passwd", strlen("passwd"), newSVpv(prec->passwd, strlen(prec->passwd)), 0);
        hv_store(out, "gecos", strlen("gecos"), newSVpv(prec->gecos, strlen(prec->gecos)), 0);
        hv_store(out, "shell", strlen("shell"), newSVpv(prec->shell, strlen(prec->shell)), 0);
        hv_store(out, "homedir", strlen("homedir"), newSVpv(prec->homedir, strlen(prec->homedir)), 0);
        out_ref = newRV_noinc((SV *) out);
        break;

    case TYPE_GROUP:
        grec = (GROUP_REC *) rec;
        hv_store(out, "name", strlen("name"), newSVpv(grec->name, strlen(grec->name)), 0);
        hv_store(out, "gid", strlen("gid"), newSVuv(grec->gid), 0);
        hv_store(out, "passwd", strlen("passwd"), newSVpv(grec->passwd, strlen(grec->passwd)), 0);
        hv_store(out, "members", strlen("members"), newRV_noinc((SV *) members), 0);

        for(i = 0; i < grec->count; i++)
            av_push(members, newSVpv(grec->members[i], strlen(grec->members[i])));

        out_ref = newRV_noinc((SV *) out);
        break;

    case TYPE_SHADOW:
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
        break;
    }

    return out_ref;
}
