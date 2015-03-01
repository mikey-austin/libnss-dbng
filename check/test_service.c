/**
 * @file test_service.h
 * @brief Test service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <err.h>

#include <dbng/service.h>
#include <service-passwd.h>

#define PASS 0
#define FAIL 1

int
main(int argc, char *argv[])
{
    int _result = PASS, ret;
    SERVICE *passwd;

    passwd = service_create(PASSWD, 0, TEST_BASE);
    if(passwd == NULL) {
        _result = FAIL;
        warnx("passwd service is NULL");
    }

    if(passwd->truncate(passwd) != 0) {
        _result = FAIL;
        warnx("could not truncate passwd service");
        goto err;
    }

    /*
     * Insert a record.
     */
    PASSWD_KEY key;
    key.base.type = PRI;
    key.data.pri = "test-dbng-user";

    PASSWD_REC rec;
    rec.base.type = PASSWD;
    rec.uid = 1001;
    rec.gid = 1001;
    rec.name = "test-dbng-user";
    rec.passwd = "x";
    rec.gecos = "test user";
    rec.shell = "/bin/bash";
    rec.homedir = "/home/test-dbng-user";

    ret = passwd->set(passwd, (KEY *) &key, (REC *) &rec);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert passwd record");
        goto err;
    }

    /*
     * Fetch the record.
     */
    PASSWD_KEY key2;
    PASSWD_REC rec2;

    key2.base.type = PRI;
    key2.data.pri = "test-dbng-user";
    ret = passwd->get(passwd, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch passwd record");
        goto err;
    }

    if(rec2.base.type != rec.base.type
       || rec2.uid != rec.uid
       || rec2.gid != rec.gid
       || strcmp(rec2.name, rec.name)
       || strcmp(rec2.passwd, rec.passwd)
       || strcmp(rec2.gecos, rec.gecos)
       || strcmp(rec2.shell, rec.shell)
       || strcmp(rec2.homedir, rec.homedir))
    {
        _result = FAIL;
        warnx("fetched record did not match inserted record");
        goto err;
    }

err:
    service_free(&passwd);

    return _result;
}
