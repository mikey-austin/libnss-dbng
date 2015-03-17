/**
 * @file test_nss_passwd.c
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
#include <service-passwd.h>

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
    struct passwd pwbuf;
    memset(&pwbuf, 0, sizeof(pwbuf));
    memset(buf, 0, sizeof(buf));

    /* First with an insufficient buffer size. */
    status = _nss_dbng_getpwnam_r("test-dbng-user", &pwbuf, buf, 1, &errnop);
    if(status != NSS_STATUS_TRYAGAIN && errnop != ERANGE) {
        warnx("expected out of space");
        result = FAIL;
        goto err;
    }

    /* Now with a non-existant user. */
    status = _nss_dbng_getpwnam_r("non-existant-user-name", &pwbuf, buf,
                                  MAX_BUF, &errnop);
    if(status != NSS_STATUS_NOTFOUND) {
        warnx("expected to not find the user");
        result = FAIL;
        goto err;
    }

    /* Now with a sufficient buffer size. */
    status = _nss_dbng_getpwnam_r("test-dbng-user", &pwbuf, buf, MAX_BUF, &errnop);
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to find the user");
        result = FAIL;
        goto err;
    }

    if(!(pwbuf.pw_uid == 1001
         && pwbuf.pw_gid == 1101
         && !strcmp(pwbuf.pw_name, "test-dbng-user")
         && !strcmp(pwbuf.pw_gecos, "test user")
         && !strcmp(pwbuf.pw_passwd, "x")
         && !strcmp(pwbuf.pw_shell, "/bin/bash")
         && !strcmp(pwbuf.pw_dir, "/home/test-dbng-user")))
    {
        warnx("unexpected user details from getpwnam_r");
        result = FAIL;
        goto err;
    }

    /* Test by uid. */
    memset(&pwbuf, 0, sizeof(pwbuf));
    memset(buf, 0, sizeof(buf));
    status = _nss_dbng_getpwuid_r(1001, &pwbuf, buf, MAX_BUF, &errnop);
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to find the user");
        result = FAIL;
        goto err;
    }

    if(!(pwbuf.pw_uid == 1001
         && pwbuf.pw_gid == 1101
         && !strcmp(pwbuf.pw_name, "test-dbng-user")
         && !strcmp(pwbuf.pw_gecos, "test user")
         && !strcmp(pwbuf.pw_passwd, "x")
         && !strcmp(pwbuf.pw_shell, "/bin/bash")
         && !strcmp(pwbuf.pw_dir, "/home/test-dbng-user")))
    {
        warnx("unexpected user details from getpwnam_r");
        result = FAIL;
        goto err;
    }

    /* Now with a non-existant user by uid. */
    memset(&pwbuf, 0, sizeof(pwbuf));
    memset(buf, 0, sizeof(buf));
    status = _nss_dbng_getpwuid_r(9999, &pwbuf, buf, MAX_BUF, &errnop);
    if(status != NSS_STATUS_NOTFOUND) {
        warnx("expected to not find the user by uid");
        result = FAIL;
        goto err;
    }

    /* Test the pwent interfaces. */
    status = _nss_dbng_setpwent();
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to to setpwent");
        result = FAIL;
        goto err;
    }

    status = _nss_dbng_setpwent();
    if(status != NSS_STATUS_TRYAGAIN) {
        warnx("expected to be told to try again setpwent");
        result = FAIL;
        goto err;
    }

    int i;
    for(i = 0;
        _nss_dbng_getpwent_r(&pwbuf, buf, MAX_BUF, &errnop);
        i++)
    {
        switch(pwbuf.pw_uid) {
        case 1001:
            if(!(pwbuf.pw_uid == 1001
                 && pwbuf.pw_gid == 1101
                 && !strcmp(pwbuf.pw_name, "test-dbng-user")
                 && !strcmp(pwbuf.pw_gecos, "test user")
                 && !strcmp(pwbuf.pw_passwd, "x")
                 && !strcmp(pwbuf.pw_shell, "/bin/bash")
                 && !strcmp(pwbuf.pw_dir, "/home/test-dbng-user")))
            {
                warnx("unexpected user details from getpwent_r");
                result = FAIL;
                goto err;
            }
            break;

        case 2001:
            if(!(pwbuf.pw_uid == 2001
                 && pwbuf.pw_gid == 2101
                 && !strcmp(pwbuf.pw_name, "another-test-dbng-user")
                 && !strcmp(pwbuf.pw_gecos, "another test user")
                 && !strcmp(pwbuf.pw_passwd, "x")
                 && !strcmp(pwbuf.pw_shell, "/bin/bash")
                 && !strcmp(pwbuf.pw_dir, "/home/another-test-dbng-user")))
            {
                warnx("unexpected user details from getpwent_r");
                result = FAIL;
                goto err;
            }
            break;

        case 3001:
            if(!(pwbuf.pw_uid == 3001
                 && pwbuf.pw_gid == 3101
                 && !strcmp(pwbuf.pw_name, "yet-another-test-dbng-user")
                 && !strcmp(pwbuf.pw_gecos, "yet another test user")
                 && !strcmp(pwbuf.pw_passwd, "x")
                 && !strcmp(pwbuf.pw_shell, "/bin/bash")
                 && !strcmp(pwbuf.pw_dir, "/home/yet-another-test-dbng-user")))
            {
                warnx("unexpected user details from getpwent_r");
                result = FAIL;
                goto err;
            }
            break;

        default:
            result = FAIL;
            warnx("unexpected uid: %d", pwbuf.pw_uid);
            goto err;
        }

        memset(&pwbuf, 0, sizeof(pwbuf));
        memset(buf, 0, sizeof(buf));
    }

    if(i != 3) {
        warnx("unexpected number of iterations (%d), getpwent_r", i);
        result = FAIL;
        goto err;
    }

    status = _nss_dbng_endpwent();
    if(status != NSS_STATUS_SUCCESS) {
        warnx("unexpected non-success, endpwent");
        result = FAIL;
        goto err;
    }

    status = _nss_dbng_setpwent();
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to to setpwent");
        result = FAIL;
        goto err;
    }
    _nss_dbng_endpwent();

err:
    _nss_dbng_endpwent();

    return result;
}

int
setup_db(void)
{
    int result = PASS, ret;
    SERVICE *passwd;

    passwd = service_create(TYPE_PASSWD, 0, TEST_BASE);
    if(passwd == NULL) {
        warnx("passwd service is NULL");
        return FAIL;
    }

    if(strcmp(passwd->pri, PASSWD_PRI)
       || strcmp(passwd->sec, PASSWD_SEC))
    {
        result = FAIL;
        warnx("incorrectly initialized passwd service");
        goto err;
    }

    if(passwd->truncate(passwd) != 0) {
        result = FAIL;
        warnx("could not truncate passwd service");
        goto err;
    }

    /*
     * Insert a few records.
     */
    PASSWD_KEY key;
    PASSWD_REC rec;
    key.base.type = PRI;
    key.data.pri = "test-dbng-user";

    rec.base.type = TYPE_PASSWD;
    rec.uid = 1001;
    rec.gid = 1101;
    rec.name = "test-dbng-user";
    rec.passwd = "x";
    rec.gecos = "test user";
    rec.shell = "/bin/bash";
    rec.homedir = "/home/test-dbng-user";

    PASSWD_KEY key2;
    PASSWD_REC rec2;
    key2.base.type = PRI;
    key2.data.pri = "another-test-dbng-user";

    rec2.base.type = TYPE_PASSWD;
    rec2.uid = 2001;
    rec2.gid = 2101;
    rec2.name = "another-test-dbng-user";
    rec2.passwd = "x";
    rec2.gecos = "another test user";
    rec2.shell = "/bin/bash";
    rec2.homedir = "/home/another-test-dbng-user";

    PASSWD_KEY key3;
    PASSWD_REC rec3;
    key3.base.type = PRI;
    key3.data.pri = "yet-another-test-dbng-user";

    rec3.base.type = TYPE_PASSWD;
    rec3.uid = 3001;
    rec3.gid = 3101;
    rec3.name = "yet-another-test-dbng-user";
    rec3.passwd = "x";
    rec3.gecos = "yet another test user";
    rec3.shell = "/bin/bash";
    rec3.homedir = "/home/yet-another-test-dbng-user";

    passwd->start_txn(passwd);
    passwd->set(passwd, (KEY *) &key, (REC *) &rec);
    passwd->set(passwd, (KEY *) &key2, (REC *) &rec2);
    passwd->set(passwd, (KEY *) &key3, (REC *) &rec3);
    ret = passwd->commit(passwd);
    if(ret != 0) {
        result = FAIL;
        warnx("could not commit txn");
        goto err;
    }

err:
    service_free(&passwd);

    return result;
}
