/**
 * @file test_nss_group.c
 * @brief Test service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <pwd.h>
#include <err.h>
#include <errno.h>
#include <string.h>

#include <nss-dbng.h>
#include <dbng/service.h>
#include <service-group.h>

#define PASS 0
#define FAIL 1
#define MAX_BUF 2048

int
main(int argc, char *argv[])
{
    int result = PASS, errnop;
    enum nss_status status;

    if((result = setup_db()) != PASS)
        goto err;

    char buf[MAX_BUF];
    struct group gbuf;
    memset(&gbuf, 0, sizeof(gbuf));
    memset(buf, 0, sizeof(buf));

    /* First with an insufficient buffer size. */
    status = _nss_dbng_getgrnam_r("test-dbng-group", &gbuf, buf, 1, &errnop);
    if(status != NSS_STATUS_TRYAGAIN && errnop != ERANGE) {
        warnx("expected out of space");
        result = FAIL;
        goto err;
    }

    /* Now with a non-existant group. */
    status = _nss_dbng_getgrnam_r("non-existant-group-name", &gbuf, buf,
                                  MAX_BUF, &errnop);
    if(status != NSS_STATUS_NOTFOUND) {
        warnx("expected to not find the group");
        result = FAIL;
        goto err;
    }

    /* Now with a sufficient buffer size. */
    status = _nss_dbng_getgrnam_r("test-dbng-group", &gbuf, buf, MAX_BUF,
                                  &errnop);
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to find the group");
        result = FAIL;
        goto err;
    }

    if(!(gbuf.gr_gid == 1101
         && !strcmp(gbuf.gr_name, "test-dbng-group")
         && !strcmp(gbuf.gr_passwd, "x")))
    {
        warnx("unexpected group details from getgrnam_r");
        result = FAIL;
        goto err;
    }

    /* Test by gid. */
    memset(&gbuf, 0, sizeof(gbuf));
    memset(buf, 0, sizeof(buf));
    status = _nss_dbng_getgrgid_r(1101, &gbuf, buf, MAX_BUF, &errnop);
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to find the group");
        result = FAIL;
        goto err;
    }

    if(!(gbuf.gr_gid == 1101
         && !strcmp(gbuf.gr_name, "test-dbng-group")
         && !strcmp(gbuf.gr_passwd, "x")))
    {
        warnx("unexpected group details from getgrnam_r");
        result = FAIL;
        goto err;
    }

    /* Now with a non-existant group by uid. */
    memset(&gbuf, 0, sizeof(gbuf));
    memset(buf, 0, sizeof(buf));
    status = _nss_dbng_getgrgid_r(9999, &gbuf, buf, MAX_BUF, &errnop);
    if(status != NSS_STATUS_NOTFOUND) {
        warnx("expected to not find the group by uid");
        result = FAIL;
        goto err;
    }

    /* Test the pwent interfaces. */
    status = _nss_dbng_setgrent();
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to to setgrent");
        result = FAIL;
        goto err;
    }

    status = _nss_dbng_setgrent();
    if(status != NSS_STATUS_TRYAGAIN) {
        warnx("expected to be told to try again setgrent");
        result = FAIL;
        goto err;
    }

    int i;
    for(i = 0;
        _nss_dbng_getgrent_r(&gbuf, buf, MAX_BUF, &errnop);
        i++)
    {
        switch(gbuf.gr_gid) {
        case 1101:
            if(!(gbuf.gr_gid == 1101
                 && !strcmp(gbuf.gr_name, "test-dbng-group")
                 && !strcmp(gbuf.gr_passwd, "x")))
            {
                warnx("unexpected group details from getgrent_r");
                result = FAIL;
                goto err;
            }
            break;

        case 2101:
            if(!(gbuf.gr_gid == 2101
                 && !strcmp(gbuf.gr_name, "another-test-dbng-group")
                 && !strcmp(gbuf.gr_passwd, "x")))
            {
                warnx("unexpected group details from getgrent_r");
                result = FAIL;
                goto err;
            }
            break;

        default:
            result = FAIL;
            warnx("unexpected gid: %d", gbuf.gr_gid);
            goto err;
        }

        memset(&gbuf, 0, sizeof(gbuf));
        memset(buf, 0, sizeof(buf));
    }

    if(i != 2) {
        warnx("unexpected number of iterations (%d), getgrent_r", i);
        result = FAIL;
        goto err;
    }

    status = _nss_dbng_endgrent();
    if(status != NSS_STATUS_SUCCESS) {
        warnx("unexpected non-success, endgrent");
        result = FAIL;
        goto err;
    }

    status = _nss_dbng_setgrent();
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to to setgrent");
        result = FAIL;
        goto err;
    }
    _nss_dbng_endgrent();

err:
    _nss_dbng_endgrent();
    return result;
}

int
setup_db(void)
{
    int result = PASS, ret;
    SERVICE *group;

    group = service_create(TYPE_GROUP, 0, TEST_BASE);
    if(group == NULL) {
        warnx("group service is NULL");
        return FAIL;
    }

    if(strcmp(group->pri, GROUP_PRI)
       || strcmp(group->sec, GROUP_SEC))
    {
        result = FAIL;
        warnx("incorrectly initialized group service");
        goto err;
    }

    if(group->truncate(group) != 0) {
        result = FAIL;
        warnx("could not truncate group service");
        goto err;
    }

    /*
     * Insert a few records.
     */
    GROUP_KEY key;
    GROUP_REC rec;
    key.base.type = PRI;
    key.data.pri = "test-dbng-group";

    rec.base.type = TYPE_GROUP;
    rec.name = "test-dbng-group";
    rec.passwd = "x";
    rec.gid = 1101;
    rec.count = 4;
    char *mem1[] = { "member1", "member2", "member3", "member4", NULL };
    rec.members = mem1;

    GROUP_KEY key2;
    GROUP_REC rec2;
    key2.base.type = PRI;
    key2.data.pri = "another-test-dbng-group";

    rec2.base.type = TYPE_GROUP;
    rec2.name = "another-test-dbng-group";
    rec2.passwd = "x";
    rec2.gid = 2101;
    rec2.count = 2;
    char *mem2[] = { "member1", "member2", NULL };
    rec2.members = mem2;

    group->start_txn(group);
    group->set(group, (KEY *) &key, (REC *) &rec);
    group->set(group, (KEY *) &key2, (REC *) &rec2);
    ret = group->commit(group);
    if(ret != 0) {
        result = FAIL;
        warnx("could not commit txn");
        goto err;
    }

err:
    service_free(&group);
    return result;
}
