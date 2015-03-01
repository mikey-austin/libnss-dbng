/**
 * @file test_service.h
 * @brief Test service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <err.h>

#include <dbng/service.h>

#define PASS 0
#define FAIL 1

int
main(int argc, char *argv[])
{
    int _result = PASS;
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

err:
    service_free(&passwd);

    return _result;
}
