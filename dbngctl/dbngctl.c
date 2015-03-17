/**
 * @file dbngctl.c
 * @brief Database management program.
 * @author Mikey Austin
 * @date 2015
 */

#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <dbng/service.h>

#define PROGNAME "dbngctl"

extern char *optarg;

static void usage(void);
static void list(SERVICE *);
static void add(SERVICE *);
static void delete(SERVICE *, const char *);

enum CMD {
    ADD,
    DELETE,
    TRUNCATE,
    LIST
};

static void
usage(void)
{
    fprintf(stderr,
            "usage: %s -s service [-b base] [-d key] [-atly]\n",
            PROGNAME);
    _exit(1);
}

static void
list(SERVICE *service)
{
    KEY *key = service->new_key(service);
    REC *rec = service->new_rec(service);

    service->start_txn(service);
    while(service->next(service, key, rec) != DB_NOTFOUND)
        service->print(service, key, rec);
    service->commit(service);

    xfree(&key);
    xfree(&rec);
}

static void
add(SERVICE *service)
{
    KEY *key = service->new_key(service);
    REC *rec = service->new_rec(service);
    char raw[SERVICE_REC_MAX];
    char *sp, *dp;
    int nparsed = 0, nfailed = 0, overflow = 0;

    service->start_txn(service);

    while(fgets(raw, sizeof(raw), stdin) != NULL) {
        sp = raw;
        if((dp = strchr(raw, '\n')) == NULL) {
            overflow = 1;
            continue;
        }
        else if(overflow) {
            /* This is the tail end of an overly large line. */
            overflow = 0;
            continue;
        }

        /* Remove trailing white space. */
        do {
            dp--;
        }
        while(dp > sp && isspace(*dp));

        if(dp == sp)
            continue;
        else
            *(++dp) = '\0';

        /* Remove leading white space. */
        while(sp != dp && isspace(*sp))
            sp++ ;

        if(*sp == '\0')
            continue;

        /* Now we have a non-empty line. */
        if(service->parse(service, sp, key, rec) > 0
            && service->set(service, key, rec) == 0)
        {
            nparsed++;
        }
        else {
            printf("failed --> %s\n", sp);
            nfailed++;
        }
    }

    service->commit(service);

    if(nparsed > 0 || nfailed > 0) {
        printf("%d parsed, %d failed\n", nparsed, nfailed);
    }

    xfree(&key);
    xfree(&rec);
}

static void
delete(SERVICE *service, const char *data)
{
    KEY *key = service->new_key(service);

    service->key_init(service, key, PRI, (void *) data);
    if(service->delete(service, key) == 0) {
        printf("deleted %s\n", data);
    }

    xfree(&key);
}

int
main(int argc, char *argv[])
{
    int option, sset = 0, flags = 0, c, prev = '\n', yes = 0;
    char *base = DEFAULT_BASE, *key;
    enum CMD cmd = LIST;
    enum TYPE stype;

    while((option = getopt(argc, argv, "s:b:d:atly")) != -1) {
        switch(option) {
        case 's':
            sset = 1;
            if(!strcasecmp(optarg, "passwd")) {
                stype = TYPE_PASSWD;
            }
            else if(!strcasecmp(optarg, "shadow")) {
                stype = TYPE_SHADOW;
            }
            else if(!strcasecmp(optarg, "group")) {
                stype = TYPE_GROUP;
            }
            else {
                fprintf(stderr, "unknown service %s\n\n", optarg);
                usage();
            }

            break;

        case 'b':
            base = optarg;
            break;

        case 'y':
            yes = 1;
            break;

        case 'a':
            cmd = ADD;
            break;

        case 'd':
            cmd = DELETE;
            key = optarg;
            break;

        case 't':
            cmd = TRUNCATE;
            break;

        case 'l':
            flags = DBNG_RO;
            cmd = LIST;
            break;

        default:
            usage();
        }
    }

    if(!sset) {
        fprintf(stderr, "a service must be specified with -s\n\n");
        usage();
    }

    SERVICE *service = service_create(stype, flags, base);
    if(service == NULL) {
        errx(1, "could not create service");
    }

    switch(cmd) {
    case LIST:
        list(service);
        break;

    case ADD:
        add(service);
        break;

    case DELETE:
        delete(service, key);
        break;

    case TRUNCATE:
        if(yes) {
            c = 'y';
            goto confirmed;
        }

        printf("Are you sure you want to truncate the database (y/n)? ");
        while((c = getchar()) != EOF) {
            switch(c) {
            case 'y':
                goto confirmed;

            case 'n':
                printf("not truncating...\n");
                goto cleanup;

            default:
                if(prev == '\n')
                    printf("please type 'y' or 'n': ");
                prev = c;
                break;
            }
        }

    confirmed:
        if(c == 'y') {
            if(service->truncate(service) == 0) {
                printf("database truncated...\n");
            }
        }
        break;
    }

cleanup:
    service_free(&service);

    return 0;
}
