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
#include <service-passwd.h>

#define PROGNAME "dbngctl"

extern char *optarg;

static void usage(void);

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

int
main(int argc, char *argv[])
{
    int option, sset = 0;
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

    return 0;
}
