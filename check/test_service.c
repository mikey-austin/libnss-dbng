/**
 * @file test_service.h
 * @brief Test service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <dbng/service.h>

int
main(int argc, char *argv[])
{
    int res = 0;
    SERVICE *passwd;

    passwd = service_create(PASSWD, 0, TEST_BASE);
    service_free(&passwd);

    return res;
}
