/**
 * @file service-shadow.c
 * @brief Implements shadow service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>
#include <regex.h>

#include "service-shadow.h"

#define ERRBUFLEN 256
#define NMATCH    8   /* A match per shadow column. */

#define PRINT_LONG(_l, _s) ((_l) >= 0 ? printf("%ld%s", (_l), (_s)) \
                            : printf("%s", (_s)))

static void print(SERVICE *, const KEY *, const REC *);
static int parse(SERVICE *, const char *, KEY *, REC *);
static KEY *new_key(SERVICE *);
static REC *new_rec(SERVICE *);
static void key_init(SERVICE *, KEY *, enum KEY_TYPE, void *);
static void pack_key(SERVICE *, const KEY *, DBT *);
static void pack_rec(SERVICE *, const REC *, DBT *);
static void unpack_key(SERVICE *, KEY *, const DBT *);
static void unpack_rec(SERVICE *, REC *, const DBT *);
static size_t rec_size(SERVICE *, const REC *);
static size_t key_size(SERVICE *, const KEY *);

extern void
service_shadow_init(SERVICE *service)
{
    memset(service, 0, sizeof(*service));
    service->type = TYPE_SHADOW;
    service->pri = SHADOW_PRI;
    service->sec = NULL;

    /* Set implemented functions. */
    service->print = print;
    service->parse = parse;
    service->pack_key = pack_key;
    service->unpack_key = unpack_key;
    service->pack_rec = pack_rec;
    service->unpack_rec = unpack_rec;
    service->rec_size = rec_size;
    service->key_size = key_size;
    service->new_key = new_key;
    service->new_rec = new_rec;
    service->key_init = key_init;
    service->cleanup = NULL;
    service->key_creator = NULL;

    /* Set inherited functions. */
    service->get = service_get_rec;
    service->set = service_set_rec;
    service->next = service_next_rec;
    service->delete = service_delete_rec;
    service->truncate = service_truncate;
    service->start_txn = service_start_txn;
    service->commit = service_commit_txn;
    service->rollback = service_rollback_txn;
    service->validate = service_validate;
}

static void
print(SERVICE *service, const KEY *key, const REC *rec)
{
    const SHADOW_KEY *skey = (const SHADOW_KEY *) key;
    const SHADOW_REC *srec = (const SHADOW_REC *) rec;

    printf("%s:%s:", srec->name, srec->passwd);
    PRINT_LONG(srec->lstchg, ":");
    PRINT_LONG(srec->min, ":");
    PRINT_LONG(srec->max, ":");
    PRINT_LONG(srec->warn, ":");
    PRINT_LONG(srec->inact, ":");
    PRINT_LONG(srec->expire, ":\n");
}

static int
parse(SERVICE *service, const char *raw, KEY *key, REC *rec)
{
    SHADOW_KEY *skey = (SHADOW_KEY *) key;
    SHADOW_REC *srec = (SHADOW_REC *) rec;
    int ret, res = 0, i, len;
    regex_t regex;
    regmatch_t matches[NMATCH + 1];
    char err_buf[ERRBUFLEN];
    static char buf[SERVICE_REC_MAX];
    size_t remaining = sizeof(buf);
    char *p_buf = buf;

    memset(&regex, 0, sizeof(regex));
    memset(skey, 0, sizeof(*skey));
    memset(srec, 0, sizeof(*srec));
    memset(&buf, 0, sizeof(buf));
    skey->base.type = PRI;
    srec->base.type = TYPE_SHADOW;

    ret = regcomp(&regex,
                  "([^:]+):"          /* user. */
                  "([^:]+):"          /* passwd. */
                  "([[:digit:]]*):"   /* last change. */
                  "([[:digit:]]*):"   /* min. */
                  "([[:digit:]]*):"   /* max. */
                  "([[:digit:]]*):"   /* warn. */
                  "([[:digit:]]*):"   /* inact. */
                  "([[:digit:]]*):$", /* expire. */
                  REG_EXTENDED);
    if(ret != 0) {
        regerror(ret, &regex, err_buf, ERRBUFLEN);
        warnx("regcomp: %s", err_buf);
        goto cleanup;
    }

    ret = regexec(&regex, raw, NMATCH + 1, matches, 0);
    if(ret != 0) {
        if(ret != REG_NOMATCH) {
            regerror(ret, &regex, err_buf, ERRBUFLEN);
            warnx("regcomp: %s", err_buf);
        }
        goto cleanup;
    }
    else {
        /* Successful match. */
        for(i = 1; i < NMATCH + 1; i++) {
            len = matches[i].rm_eo - matches[i].rm_so;
            if((remaining -= (len + 1)) <= 0) {
                warnx("regexec: parsed record too large");
                goto cleanup;
            }
            strncpy(p_buf, (raw + matches[i].rm_so), len);
            p_buf[len] = '\0';

            switch(i) {
            case 1:
                skey->data.pri = srec->name = p_buf;
                break;

            case 2:
                srec->passwd = p_buf;
                break;

            case 3:
                srec->lstchg = (len == 0 ? -1 : atoi(p_buf));
                break;

            case 4:
                srec->min = (len == 0 ? -1 : atoi(p_buf));
                break;

            case 5:
                srec->max = (len == 0 ? -1 : atoi(p_buf));
                break;

            case 6:
                srec->warn = (len == 0 ? -1 : atoi(p_buf));
                break;

            case 7:
                srec->inact = (len == 0 ? -1 : atoi(p_buf));
                break;

            case 8:
                srec->expire = (len == 0 ? -1 : atoi(p_buf));
                break;
            }

            p_buf += (len + 1);
        }

        res = 1;
    }

cleanup:
    regfree(&regex);
    return res;
}

static KEY
*new_key(SERVICE *service)
{
    return xmalloc(sizeof(SHADOW_KEY));
}

static REC
*new_rec(SERVICE *service)
{
    return xmalloc(sizeof(SHADOW_REC));
}

static void
key_init(SERVICE *service, KEY *key, enum KEY_TYPE type, void *data)
{
    SHADOW_KEY *skey = (SHADOW_KEY *) key;
    skey->base.type = type;
    skey->data.pri = (char *) data;
}

static size_t
rec_size(SERVICE *service, const REC *rec)
{
    SHADOW_REC *srec = (SHADOW_REC *) rec;

    return strlen(srec->name) + 1
        + strlen(srec->passwd) + 1
        + sizeof(srec->lstchg)
        + sizeof(srec->min)
        + sizeof(srec->max)
        + sizeof(srec->warn)
        + sizeof(srec->inact)
        + sizeof(srec->expire);
}

static size_t
key_size(SERVICE *service, const KEY *key)
{
    SHADOW_KEY *skey = (SHADOW_KEY *) key;

    return sizeof(skey->base.type)
        + strlen(skey->data.pri) + 1;
}

static void
pack_key(SERVICE *service, const KEY *key, DBT *dbkey)
{
    SHADOW_KEY *skey = (SHADOW_KEY *) key;
    char *buf = dbkey->data, *s;

    memcpy(buf, &(skey->base.type), sizeof(skey->base.type));
    memcpy(buf + sizeof(skey->base.type), skey->data.pri,
           strlen(skey->data.pri) + 1);
}

static void
pack_rec(SERVICE *service, const REC *rec, DBT *dbrec)
{
    SHADOW_REC *srec = (SHADOW_REC *) rec;
    char *buf = NULL, *s;
    int len, slen = 0;

    buf = dbrec->data;
    len = dbrec->size;

    s = buf;
    memcpy(s, srec->name, (slen = (strlen(srec->name) + 1)));
    s += slen;

    memcpy(s, srec->passwd, (slen = (strlen(srec->passwd) + 1)));
    s += slen;

    memcpy(s, &srec->lstchg, (slen = sizeof(srec->lstchg)));
    s += slen;

    memcpy(s, &srec->min, (slen = sizeof(srec->min)));
    s += slen;

    memcpy(s, &srec->max, (slen = sizeof(srec->max)));
    s += slen;

    memcpy(s, &srec->warn, (slen = sizeof(srec->warn)));
    s += slen;

    memcpy(s, &srec->inact, (slen = sizeof(srec->inact)));
    s += slen;

    memcpy(s, &srec->expire, (slen = sizeof(srec->expire)));
    s += slen;

    memset(dbrec, 0, sizeof(*dbrec));
    dbrec->data = buf;
    dbrec->size = len;
}

static void
unpack_key(SERVICE *service, KEY *key, const DBT *dbkey)
{
    SHADOW_KEY *skey = (SHADOW_KEY *) key;
    char *buf = (char *) dbkey->data;

    memset(skey, 0, sizeof(*skey));
    skey->base.type = *((enum KEY_TYPE *) buf);
    buf += sizeof(skey->base.type);
    skey->data.pri = buf;
}

static void
unpack_rec(SERVICE *service, REC *rec, const DBT *dbrec)
{
    SHADOW_REC *srec = (SHADOW_REC *) rec;
    char *buf = (char *) dbrec->data;

    memset(srec, 0, sizeof(*srec));
    srec->base.type = TYPE_SHADOW;

    srec->name = buf;
    buf += strlen(srec->name) + 1;

    srec->passwd = buf;
    buf += strlen(srec->passwd) + 1;

    memcpy(&srec->lstchg, buf, sizeof(srec->lstchg));
    buf += sizeof(srec->lstchg);

    memcpy(&srec->min, buf, sizeof(srec->min));
    buf += sizeof(srec->min);

    memcpy(&srec->max, buf, sizeof(srec->max));
    buf += sizeof(srec->max);

    memcpy(&srec->warn, buf, sizeof(srec->warn));
    buf += sizeof(srec->warn);

    memcpy(&srec->inact, buf, sizeof(srec->inact));
    buf += sizeof(srec->inact);

    memcpy(&srec->expire, buf, sizeof(srec->expire));
    buf += sizeof(srec->expire);
}
