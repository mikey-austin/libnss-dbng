/**
 * @file test_service.h
 * @brief Test service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <err.h>
#include <string.h>

#include <dbng/service.h>
#include <service-passwd.h>

#define PASS 0
#define FAIL 1

static int reccmp(PASSWD_REC *, PASSWD_REC *);

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

    if(strcmp(passwd->pri, PASSWD_PRI)
       || strcmp(passwd->sec, PASSWD_SEC))
    {
        _result = FAIL;
        warnx("incorrectly initialized passwd service");
        goto err;
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
    ret = passwd->get(passwd, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch unexpected return code");
        goto err;
    }

    /*
     * Fetch the record by secondary index.
     */
    key2.base.type = SEC;
    key2.data.sec = 1001;
    memset(&rec2, 0, sizeof(rec2));
    ret = passwd->get(passwd, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch passwd record");
        goto err;
    }

    if(reccmp(&rec2, &rec)) {
        _result = FAIL;
        warnx("fetched record (by uid) did not match inserted record");
        goto err;
    }

    /*
     * Fetch a non-existant record by secondary index.
     */
    key2.data.sec = 8888;
    memset(&rec2, 0, sizeof(rec2));
    ret = passwd->get(passwd, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch by uid unexpected return code");
        goto err;
    }

    /*
     * Test deletion.
     */
    key2.base.type = PRI;
    key2.data.pri = "test-dbng-user";
    ret = passwd->delete(passwd, (KEY *) &key2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not delete passwd record");
        goto err;
    }

    /*
     * Fetch the recently deleted record by primary & secondary indexes.
     */
    memset(&rec2, 0, sizeof(rec2));
    ret = passwd->get(passwd, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch unexpected return code");
        goto err;
    }

    key2.base.type = SEC;
    key2.data.sec = 1001;
    ret = passwd->get(passwd, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch by uid unexpected return code");
        goto err;
    }

    /*
     * Test transaction rollback.
     */
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

    ret = passwd->start_txn(passwd);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not start txn");
        goto err;
    }

    ret = passwd->set(passwd, (KEY *) &key, (REC *) &rec);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert passwd record");
        goto err;
    }

    ret = passwd->set(passwd, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert passwd record");
        goto err;
    }

    ret = passwd->set(passwd, (KEY *) &key3, (REC *) &rec3);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert passwd record");
        goto err;
    }

    ret = passwd->commit(passwd);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not commit txn");
        goto err;
    }

    /*
     * Test the rollback by truncating the database.
     */
    ret = passwd->start_txn(passwd);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not start txn");
        goto err;
    }

    ret = passwd->truncate(passwd);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not truncate passwd service");
        goto err;
    }

    ret = passwd->rollback(passwd);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not rollback txn");
        goto err;
    }

    /*
     * Attempt to fetch the record. Should succeed as previous
     * transaction was rolled back.
     */
    PASSWD_KEY key4;
    PASSWD_REC rec4;
    ret = passwd->get(passwd, (KEY *) &key2, (REC *) &rec4);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch passwd record");
        goto err;
    }

    /*
     * Test iterating over records.
     */
    PASSWD_REC *rp;
    int i;
    for(i = 0;
        (ret = passwd->next(passwd, (KEY *) &key4, (REC *) &rec4)) != DB_NOTFOUND;
        i++)
    {
        if(ret != 0 && ret != DB_NOTFOUND) {
            _result = FAIL;
            warnx("unexpected return code from next: %d", ret);
            goto err;                
        }

        switch(rec4.uid) {
        case 1001:
            rp = &rec;
            break;

        case 2001:
            rp = &rec2;
            break;

        case 3001:
            rp = &rec3;
            break;
        }

        if(reccmp(&rec4, rp)) {
            _result = FAIL;
            warnx("records don't match");
            goto err;                
        }
    }

    if(i != 3) {
        _result = FAIL;
        warnx("expecting 3 records, seen %d", i);
        goto err;                
    }

    if(passwd->db->cursor != NULL) {
        _result = FAIL;
        warnx("cursor not cleaned up correctly");
        goto err;                
    }

err:
    service_free(&passwd);

    return _result;
}

static int
reccmp(PASSWD_REC *a, PASSWD_REC *b)
{
    if(a->base.type != b->base.type
       || a->uid != b->uid
       || a->gid != b->gid
       || strcmp(a->name, b->name)
       || strcmp(a->passwd, b->passwd)
       || strcmp(a->gecos, b->gecos)
       || strcmp(a->shell, b->shell)
       || strcmp(a->homedir, b->homedir))
    {
        return 1;
    }

    /* The two records are equal. */
    return 0;
}
