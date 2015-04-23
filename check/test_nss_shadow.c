/**
 * @file test_nss_shadow.c
 * @brief Test service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <shadow.h>
#include <err.h>
#include <errno.h>
#include <string.h>

#include "../nss/nss-dbng.h"
#include "../lib/service-shadow.h"

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
    struct spwd spbuf;
    memset(&spbuf, 0, sizeof(spbuf));
    memset(buf, 0, sizeof(buf));

    /* First with an insufficient buffer size. */
    status = _nss_dbng_getspnam_r("test-dbng-user", &spbuf, buf, 1, &errnop);
    if(status != NSS_STATUS_TRYAGAIN && errnop != ERANGE) {
        warnx("expected out of space");
        result = FAIL;
        goto err;
    }

    /* Now with a non-existant user. */
    status = _nss_dbng_getspnam_r("non-existant-user-name", &spbuf, buf,
                                  MAX_BUF, &errnop);
    if(status != NSS_STATUS_NOTFOUND) {
        warnx("expected to not find the user");
        result = FAIL;
        goto err;
    }

    /* Now with a sufficient buffer size. */
    status = _nss_dbng_getspnam_r("test-dbng-user", &spbuf, buf, MAX_BUF,
                                  &errnop);
    if(status != NSS_STATUS_SUCCESS) {
        warnx("expected to find the user");
        result = FAIL;
        goto err;
    }

    if(!(!strcmp(spbuf.sp_namp, "test-dbng-user")
         && !strcmp(spbuf.sp_pwdp, "$6$nLngwds2$kQ70NzYAGfo2QFubQwoUChcweCkaTVl8eL7Sj.zCn9A0Q8p3UNmty44NuQGR6GvzaiKz4N.VNMdfEpc7NoJ3/1")
         && spbuf.sp_lstchg == 16511
         && spbuf.sp_min == 0
         && spbuf.sp_max == 99999
         && spbuf.sp_warn == 7
         && spbuf.sp_inact == -1
         && spbuf.sp_expire == 1))
    {
        warnx("unexpected user details from getspnam_r");
        result = FAIL;
        goto err;
    }

err:
    return result;
}

int
setup_db(void)
{
    int result = PASS, ret;
    SERVICE shadow;

    service_init(&shadow, TYPE_SHADOW, 0, TEST_BASE);

    if(strcmp(shadow.pri, SHADOW_PRI)) {
        result = FAIL;
        warnx("incorrectly initialized shadow service");
        goto err;
    }

    if(shadow.truncate(&shadow) != 0) {
        result = FAIL;
        warnx("could not truncate shadow service");
        goto err;
    }

    /*
     * Insert a few records.
     */
    SHADOW_KEY key;
    SHADOW_REC rec;
    key.base.type = PRI;
    key.data.pri = "test-dbng-user";

    rec.base.type = TYPE_SHADOW;
    rec.name = "test-dbng-user";
    rec.passwd = "$6$nLngwds2$kQ70NzYAGfo2QFubQwoUChcweCkaTVl8eL7Sj.zCn9A0Q8p3UNmty44NuQGR6GvzaiKz4N.VNMdfEpc7NoJ3/1";
    rec.lstchg = 16511;
    rec.min = 0;
    rec.max = 99999;
    rec.warn = 7;
    rec.inact = -1;
    rec.expire = 1;

    SHADOW_KEY key2;
    SHADOW_REC rec2;

    key2.base.type = PRI;
    key2.data.pri = "another-test-dbng-user";

    rec2.base.type = TYPE_SHADOW;
    rec2.name = "another-test-dbng-user";
    rec2.passwd = "$6$nLngwds2$kQ70NzYAGfo2QFubQwoUChcweCkaTVl8eL7Sj.zCn9A0Q8p3UNmty44NuQGR6GvzaiKz4N.VNMdfEpc7NoJ3/1";
    rec2.lstchg = 16511;
    rec2.min = 0;
    rec2.max = 99999;
    rec2.warn = 7;
    rec2.inact = 1234;
    rec2.expire = 2;

    shadow.start_txn(&shadow);
    shadow.set(&shadow, (KEY *) &key, (REC *) &rec);
    shadow.set(&shadow, (KEY *) &key2, (REC *) &rec2);
    ret = shadow.commit(&shadow);
    if(ret != 0) {
        result = FAIL;
        warnx("could not commit txn");
        goto err;
    }

    service_cleanup(&shadow);

err:
    return result;
}
