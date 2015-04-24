/**
 * @file test_shadow_service.c
 * @brief Test shadow service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <err.h>
#include <string.h>

#include "../lib/dbng/service.h"
#include "../lib/service-shadow.h"

#define PASS 0
#define FAIL 1

static int reccmp(SHADOW_REC *, SHADOW_REC *);

int
main(int argc, char *argv[])
{
    int _result = PASS, ret;
    SERVICE shadow;

    if(service_init(&shadow, TYPE_SHADOW, 0, TEST_BASE) < 0) {
        _result = FAIL;
        warnx("could not initialize shadow service");
        goto err;
    }

    if(strcmp(shadow.pri, SHADOW_PRI)) {
        _result = FAIL;
        warnx("incorrectly initialized shadow service");
        goto err;
    }

    if(shadow.truncate(&shadow) != 0) {
        _result = FAIL;
        warnx("could not truncate shadow service");
        goto err;
    }

    /*
     * Insert a record.
     */
    SHADOW_KEY key;
    key.base.type = PRI;
    key.data.pri = "test-dbng-user";

    SHADOW_REC rec;
    rec.base.type = TYPE_SHADOW;
    rec.name = "test-dbng-user";
    rec.passwd = "$6$nLngwds2$kQ70NzYAGfo2QFubQwoUChcweCkaTVl8eL7Sj.zCn9A0Q8p3UNmty44NuQGR6GvzaiKz4N.VNMdfEpc7NoJ3/1";
    rec.lstchg = 16511;
    rec.min = 0;
    rec.max = 99999;
    rec.warn = 7;
    rec.inact = -1;
    rec.expire = 1;

    ret = shadow.set(&shadow, (KEY *) &key, (REC *) &rec);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert shadow record");
        goto err;
    }

    /*
     * Fetch the record.
     */
    SHADOW_KEY key2;
    SHADOW_REC rec2;

    key2.base.type = PRI;
    key2.data.pri = "test-dbng-user";
    ret = shadow.get(&shadow, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch shadow record");
        goto err;
    }

    if(reccmp(&rec2, &rec)) {
        _result = FAIL;
        warnx("fetched record did not match inserted record");
        goto err;
    }

    /*
     * Fetch a non-existant record.
     */
    key2.data.pri = "non-existant-user";
    memset(&rec2, 0, sizeof(rec2));
    ret = shadow.get(&shadow, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch unexpected return code");
        goto err;
    }

    /*
     * Test deletion.
     */
    key2.base.type = PRI;
    key2.data.pri = "test-dbng-user";
    ret = shadow.delete(&shadow, (KEY *) &key2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not delete shadow record");
        goto err;
    }

    /*
     * Fetch the recently deleted record by primary index.
     */
    memset(&rec2, 0, sizeof(rec2));
    ret = shadow.get(&shadow, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch unexpected return code");
        goto err;
    }

    /*
     * Test transaction rollback.
     */
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

    ret = shadow.start_txn(&shadow);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not start txn");
        goto err;
    }

    ret = shadow.set(&shadow, (KEY *) &key, (REC *) &rec);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert shadow record");
        goto err;
    }

    ret = shadow.set(&shadow, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert shadow record");
        goto err;
    }

    ret = shadow.commit(&shadow);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not commit txn");
        goto err;
    }

    /*
     * Attempt to fetch the record. Should succeed as previous
     * transaction was rolled back.
     */
    SHADOW_KEY key4;
    SHADOW_REC rec4;
    ret = shadow.get(&shadow, (KEY *) &key2, (REC *) &rec4);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch shadow record");
        goto err;
    }

    /*
     * Test iterating over records.
     */
    SHADOW_REC *rp;
    int i;
    for(i = 0;
        (ret = shadow.next(&shadow, (KEY *) &key4, (REC *) &rec4)) != DB_NOTFOUND;
        i++)
    {
        if(ret != 0 && ret != DB_NOTFOUND) {
            _result = FAIL;
            warnx("unexpected return code from next: %d", ret);
            goto err;
        }

        switch(rec4.expire) {
        case 1:
            rp = &rec;
            break;

        case 2:
            rp = &rec2;
            break;

        default:
            _result = FAIL;
            warnx("unexpected expire: %d", rec4.expire);
            goto err;
        }

        if(reccmp(&rec4, rp)) {
            _result = FAIL;
            warnx("records don't match");
            goto err;
        }
    }

    if(i != 2) {
        _result = FAIL;
        warnx("expecting 2 records, seen %d", i);
        goto err;
    }

    if(shadow.db.cursor != NULL) {
        _result = FAIL;
        warnx("cursor not cleaned up correctly");
        goto err;
    }

    service_cleanup(&shadow);

err:
    return _result;
}

static int
reccmp(SHADOW_REC *a, SHADOW_REC *b)
{
    if(a->base.type != b->base.type
       || strcmp(a->name, b->name)
       || strcmp(a->passwd, b->passwd)
       || a->lstchg != b->lstchg
       || a->min != b->min
       || a->max != b->max
       || a->warn != b->warn
       || a->inact != b->inact
       || a->expire != b->expire)
    {
        return 1;
    }

    /* The two records are equal. */
    return 0;
}
