/**
 * @file service-group.c
 * @brief Implements group service abstraction interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <string.h>
#include <regex.h>

#include "service-group.h"

#define ERRBUFLEN   256
#define NMATCH      4   /* A match per group column. */

static int validate(SERVICE *, const KEY *, const REC *);
static void print(SERVICE *, const KEY *, const REC *);
static int parse(SERVICE *, const char *, KEY *, REC *);
static KEY *new_key(SERVICE *);
static REC *new_rec(SERVICE *);
static void key_init(SERVICE *, KEY *, enum KEY_TYPE, void *);
static void pack_key(SERVICE *, const KEY *, DBT *);
static void pack_rec(SERVICE *, const REC *, DBT *);
static void unpack_key(SERVICE *, KEY *, const DBT *);
static void unpack_rec(SERVICE *, REC *, const DBT *);
static int key_creator(DB *, const DBT *, const DBT *, DBT *);
static size_t rec_size(SERVICE *, const REC *);
static size_t key_size(SERVICE *, const KEY *);

extern void
service_group_init(SERVICE *service)
{
    memset(service, 0, sizeof(*service));
    service->type = TYPE_GROUP;
    service->pri = GROUP_PRI;
    service->sec = GROUP_SEC;

    /* Set implemented functions. */
    service->print = print;
    service->validate = validate;
    service->parse = parse;
    service->key_creator = key_creator;
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

    /* Set inherited functions. */
    service->get = service_get_rec;
    service->set = service_set_rec;
    service->next = service_next_rec;
    service->delete = service_delete_rec;
    service->truncate = service_truncate;
    service->start_txn = service_start_txn;
    service->commit = service_commit_txn;
    service->rollback = service_rollback_txn;
}

static void
print(SERVICE *service, const KEY *key, const REC *rec)
{
    const GROUP_KEY *gkey = (const GROUP_KEY *) key;
    const GROUP_REC *grec = (const GROUP_REC *) rec;
    char **member = NULL;
    int i;

    printf("%s:%s:%ld:",
           grec->name, grec->passwd,
           (unsigned long) grec->gid);

    for(member = grec->members, i = 0;
        member != NULL && *member != NULL;
        member++, i++)
    {
        printf("%s%s", (i > 0 ? "," : ""), *member);
    }

    printf("\n");
}

static int
parse(SERVICE *service, const char *raw, KEY *key, REC *rec)
{
    GROUP_KEY *gkey = (GROUP_KEY *) key;
    GROUP_REC *grec = (GROUP_REC *) rec;
    int ret, res = 0, i, len;
    regex_t regex;
    regmatch_t matches[NMATCH + 1];
    char err_buf[ERRBUFLEN];
    static char buf[SERVICE_REC_MAX];
    size_t remaining = sizeof(buf);
    char *p_buf = buf, *c_buf, *member;

    memset(&regex, 0, sizeof(regex));
    memset(gkey, 0, sizeof(*gkey));
    memset(grec, 0, sizeof(*grec));
    memset(&buf, 0, sizeof(buf));
    gkey->base.type = PRI;
    grec->base.type = TYPE_GROUP;

    ret = regcomp(&regex,
                  "([^:]+):"        /* group. */
                  "([^:]+):"        /* passwd. */
                  "([[:digit:]]+):" /* gid. */
                  "([^:]*)$",       /* members (may be empty). */
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
        for(i = 1; i < NMATCH; i++) {
            len = matches[i].rm_eo - matches[i].rm_so;
            if((remaining -= (len + 1)) <= 0) {
                warnx("regexec: parsed record too large");
                goto cleanup;
            }
            strncpy(p_buf, (raw + matches[i].rm_so), len);
            p_buf[len] = '\0';

            switch(i) {
            case 1:
                gkey->data.pri = grec->name = p_buf;
                break;

            case 2:
                grec->passwd = p_buf;
                break;

            case 3:
                grec->gid = atoi(p_buf);
                break;
            }

            p_buf += (len + 1);
        }

        /* Count the number of members by counting the number of commas. */
        grec->count = (strlen(raw + matches[4].rm_so) > 0 ? 1 : 0);
        c_buf = strchr(raw + matches[4].rm_so, ',');
        while(c_buf != NULL) {
            grec->count++;
            c_buf = strchr(++c_buf, ',');
        }

        len = ((grec->count + 1) * sizeof(char *));
        remaining -= len;
        if(grec->count > 0 && remaining > 0) {
            /* Reserve space for pointers. */
            grec->members = (char **) p_buf;
            p_buf += len;

            /* Copy the actual strings. */
            len = strlen(raw + matches[4].rm_so);
            char members[len + 1];
            strcpy(members, raw + matches[4].rm_so);
            members[len] = '\0';

            for(i = 0, member = strtok(members, ",");
                remaining > 0 && i < grec->count && member != NULL;
                i++, member = strtok(NULL, ","))
            {
                grec->members[i] = p_buf;
                strcpy(p_buf, member);
                len = strlen(member);
                p_buf[len] = '\0';
                p_buf += (len + 1);
                remaining -= (len + 1);
            }
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
    return xmalloc(sizeof(GROUP_KEY));
}

static REC
*new_rec(SERVICE *service)
{
    return xmalloc(sizeof(GROUP_REC));
}

static void
key_init(SERVICE *service, KEY *key, enum KEY_TYPE type, void *data)
{
    GROUP_KEY *gkey = (GROUP_KEY *) key;
    gkey->base.type = type;
    gkey->data.pri = (char *) data;
}

static int
key_creator(DB *dbp, const DBT *gkey, const DBT *gdata, DBT *skey)
{
    GROUP_KEY key;
    GROUP_REC rec;

    /* Create the secondary index on the gid. */
    unpack_rec(NULL, (REC *) &rec, gdata);
    key.base.type = SEC;
    key.data.sec = rec.gid;
    int size = key_size(NULL, (KEY *) &key);

    skey->data = xcalloc(1, size);
    skey->size = size;
    skey->flags = DB_DBT_APPMALLOC;
    pack_key(NULL, (KEY *) &key, skey);

    return 0;
}

static size_t
rec_size(SERVICE *service, const REC *rec)
{
    GROUP_REC *grec = (GROUP_REC *) rec;
    size_t size;
    char **member;

    size = sizeof(grec->gid)
        + sizeof(grec->count)
        + ((grec->count + 1) * sizeof(char *))
        + strlen(grec->name) + 1
        + strlen(grec->passwd) + 1;

    for(member = grec->members;
        member != NULL && *member != NULL;
        member++)
    {
        size += strlen(*member) + 1;
    }

    return size;
}

static size_t
key_size(SERVICE *service, const KEY *key)
{
    GROUP_KEY *gkey = (GROUP_KEY *) key;

    return sizeof(gkey->base.type)
        + (gkey->base.type == PRI
           ? (strlen(gkey->data.pri) + 1)
           : sizeof(gkey->data.sec));
}

static void
pack_key(SERVICE *service, const KEY *key, DBT *dbkey)
{
    GROUP_KEY *gkey = (GROUP_KEY *) key;
    char *buf = dbkey->data, *s;

    switch(gkey->base.type) {
    case PRI:
        memcpy(buf + sizeof(gkey->base.type), gkey->data.pri,
               strlen(gkey->data.pri) + 1);
        break;

    case SEC:
        memcpy(buf + sizeof(gkey->base.type), &gkey->data.sec,
               sizeof(gkey->data.sec));
        break;
    }
}

static void
pack_rec(SERVICE *service, const REC *rec, DBT *dbrec)
{
    GROUP_REC *grec = (GROUP_REC *) rec;
    char *buf = NULL, *s, **member;
    int len, slen = 0;

    /* Reserve space for the record and the number of group members. */
    buf = dbrec->data;
    len = dbrec->size;

    s = buf;
    memcpy(s, &grec->gid, (slen = sizeof(grec->gid)));
    s += slen;

    memcpy(s, grec->name, (slen = (strlen(grec->name) + 1)));
    s += slen;

    memcpy(s, grec->passwd, (slen = (strlen(grec->passwd) + 1)));
    s += slen;

    /* Reserve space for count and member pointers. */
    memcpy(s, &grec->count, (slen = sizeof(grec->count)));
    s += slen + ((grec->count + 1) * sizeof(char *));

    for(member = grec->members;
        member != NULL && *member != NULL;
        member++)
    {
        memcpy(s, *member, (slen = (strlen(*member) + 1)));
        s += slen;
    }

    memset(dbrec, 0, sizeof(*dbrec));
    dbrec->data = buf;
    dbrec->size = len;
}

static void
unpack_key(SERVICE *service, KEY *key, const DBT *dbkey)
{
    GROUP_KEY *gkey = (GROUP_KEY *) key;
    char *buf = (char *) dbkey->data;

    memset(gkey, 0, sizeof(*gkey));
    gkey->base.type = *((enum KEY_TYPE *) buf);
    buf += sizeof(gkey->base.type);

    switch(gkey->base.type) {
    case PRI:
        gkey->data.pri = buf;
        break;

    case SEC:
        memcpy(&gkey->data.sec, buf, sizeof(gkey->data.sec));
        break;
    }
}

static void
unpack_rec(SERVICE *service, REC *rec, const DBT *dbrec)
{
    GROUP_REC *grec = (GROUP_REC *) rec;
    char *buf = (char *) dbrec->data;
    int i, len, remaining = dbrec->size;

    memset(grec, 0, sizeof(*grec));
    grec->base.type = TYPE_GROUP;

    memcpy(&grec->gid, buf, sizeof(grec->gid));
    buf += sizeof(grec->gid);
    remaining -= sizeof(grec->gid);

    grec->name = buf;
    buf += (len = (strlen(grec->name) + 1));
    remaining -= len;

    grec->passwd = buf;
    buf += (len = (strlen(grec->passwd) + 1));
    remaining -= len;

    memcpy(&grec->count, buf, sizeof(grec->count));
    buf += sizeof(grec->count);
    remaining -= sizeof(grec->count);

    grec->members = (char **) buf;
    buf += (len = ((grec->count + 1) * sizeof(char *)));
    remaining -= len;

    for(i = 0; i < grec->count && remaining > 0; i++) {
        grec->members[i] = buf;
        buf += (len = (strlen(grec->members[i]) + 1));
        remaining -= len;
    }

    grec->members[i] = NULL;
}

static int
validate(SERVICE *service, const KEY *key, const REC *rec)
{
    const GROUP_REC *prec = (const GROUP_REC *) rec;
    gid_t gid = getgid();
 
    if(MIN_GID > 0) {
        return ((gid < MIN_GID && prec->gid >= MIN_GID) || prec->gid == gid);
    }
    else {
        return 1;
    }
}
