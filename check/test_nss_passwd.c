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

    /*
     * Test getpwnam_r.
     */
    char buf[MAX_BUF];
    struct passwd pwbuf;
    memset(buf, 0, sizeof(buf));

    /* First with an insufficient buffer size. */
    status = _nss_dbng_getpwnam_r("test-dbng-user", &pwbuf, buf, 1, &errnop);
    if(status != NSS_STATUS_TRYAGAIN && errnop != ERANGE) {
        warnx("expected out of space");
        goto err;
    }

    /* Now with a sufficient buffer size. */
    status = _nss_dbng_getpwnam_r("test-dbng-user", &pwbuf, buf, MAX_BUF, &errnop);
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to find the user");
        goto err;
    }

    /* Now with a non-existant user. */
    status = _nss_dbng_getpwnam_r("non-existant-user-name", &pwbuf, buf, MAX_BUF, &errnop);
    if(status != NSS_STATUS_NOTFOUND) {
        warnx("expected to not find the user");
        goto err;
    }

err:
    return result;
}

int
setup_db(void)
{
    int result = PASS, ret;
    SERVICE *passwd;

    passwd = service_create(PASSWD, 0, TEST_BASE);
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

    rec.base.type = PASSWD;
    rec.uid = 1001;
    rec.gid = 1001;
    rec.name = "test-dbng-user";
    rec.passwd = "x";
    rec.gecos = "test user";
    rec.shell = "/bin/bash";
    rec.homedir = "/home/test-dbng-user";

    PASSWD_KEY key2;
    PASSWD_REC rec2;
    key2.base.type = PRI;
    key2.data.pri = "another-test-dbng-user";

    rec2.base.type = PASSWD;
    rec2.uid = 2001;
    rec2.gid = 2001;
    rec2.name = "another-test-dbng-user";
    rec2.passwd = "x";
    rec2.gecos = "another test user";
    rec2.shell = "/bin/bash";
    rec2.homedir = "/home/another-test-dbng-user";

    PASSWD_KEY key3;
    PASSWD_REC rec3;
    key3.base.type = PRI;
    key3.data.pri = "yet-another-test-dbng-user";

    rec3.base.type = PASSWD;
    rec3.uid = 3001;
    rec3.gid = 3001;
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
