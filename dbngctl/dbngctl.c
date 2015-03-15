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
static void delete(SERVICE *);

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
            "usage: %s -s service [-b base] [-adtl]\n",
            PROGNAME);
    _exit(1);
}

static void
list(SERVICE *service)
{
    KEY key;
    REC rec;

    while(service->next(service, &key, &rec) != DB_NOTFOUND)
        service->print(service, &key, &rec);
}

static void
add(SERVICE *service)
{
}

static void
delete(SERVICE *service)
{
}

int
main(int argc, char *argv[])
{
    int option, sset = 0, flags = 0, c, prev = '\n';
    char *base = DEFAULT_BASE;
    enum CMD cmd = LIST;
    enum TYPE stype;

    while((option = getopt(argc, argv, "s:b:adtl")) != -1) {
        switch(option) {
        case 's':
            sset = 1;
            if(!strcasecmp(optarg, "passwd")) {
                stype = PASSWD;
            }
            else {
                fprintf(stderr, "unknown service %s\n\n", optarg);
                usage();
            }

            break;

        case 'b':
            base = optarg;
            break;

        case 'a':
            cmd = ADD;
            break;

        case 'd':
            cmd = DELETE;
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
        delete(service);
        break;

    case TRUNCATE:
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
