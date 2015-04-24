/**
 * @file test_group_service.c
 * @brief Test group service interface.
 * @author Mikey Austin
 * @date 2015
 */

#include <err.h>
#include <string.h>

#include "../lib/dbng/service.h"
#include "../lib/service-group.h"

#define PASS 0
#define FAIL 1

static int reccmp(GROUP_REC *, GROUP_REC *);

int
main(int argc, char *argv[])
{
    int _result = PASS, ret;
    SERVICE group;

    if(service_init(&group, TYPE_GROUP, 0, TEST_BASE) < 0) {
        _result = FAIL;
        warnx("could not initialize service...");
        goto err;
    }

    if(strcmp(group.pri, GROUP_PRI)
       || strcmp(group.sec, GROUP_SEC))
    {
        _result = FAIL;
        warnx("incorrectly initialized group service");
        goto err;
    }

    if(group.truncate(&group) != 0) {
        _result = FAIL;
        warnx("could not truncate group service");
        goto err;
    }

    /*
     * Insert a record.
     */
    GROUP_KEY key;
    key.base.type = PRI;
    key.data.pri = "test-dbng-group";

    GROUP_REC rec;
    char *group1_members[4] = { "member1", "member2", "member3", NULL };
    rec.base.type = TYPE_GROUP;
    rec.gid = 1001;
    rec.name = "test-dbng-group";
    rec.passwd = "x";
    rec.count = 4;
    rec.members = group1_members;

    ret = group.set(&group, (KEY *) &key, (REC *) &rec);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert group record");
        goto err;
    }

    /*
     * Fetch the record.
     */
    GROUP_KEY key2;
    GROUP_REC rec2;

    key2.base.type = PRI;
    key2.data.pri = "test-dbng-group";
    ret = group.get(&group, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch group record");
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
    key2.data.pri = "non-existant-group";
    memset(&rec2, 0, sizeof(rec2));
    ret = group.get(&group, (KEY *) &key2, (REC *) &rec2);
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
    ret = group.get(&group, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch group record");
        goto err;
    }

    if(reccmp(&rec2, &rec)) {
        _result = FAIL;
        warnx("fetched record (by gid) did not match inserted record");
        goto err;
    }

    /*
     * Fetch a non-existant record by secondary index.
     */
    key2.data.sec = 8888;
    memset(&rec2, 0, sizeof(rec2));
    ret = group.get(&group, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch by gid unexpected return code");
        goto err;
    }

    /*
     * Test deletion.
     */
    key2.base.type = PRI;
    key2.data.pri = "test-dbng-group";
    ret = group.delete(&group, (KEY *) &key2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not delete group record");
        goto err;
    }

    /*
     * Fetch the recently deleted record by primary & secondary indexes.
     */
    memset(&rec2, 0, sizeof(rec2));
    ret = group.get(&group, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch unexpected return code");
        goto err;
    }

    key2.base.type = SEC;
    key2.data.sec = 1001;
    ret = group.get(&group, (KEY *) &key2, (REC *) &rec2);
    if(ret != DB_NOTFOUND) {
        _result = FAIL;
        warnx("fetch by gid unexpected return code");
        goto err;
    }

    /*
     * Test transaction rollback.
     */
    key2.base.type = PRI;
    key2.data.pri = "another-test-dbng-group";

    rec2.base.type = TYPE_GROUP;
    rec2.gid = 2001;
    rec2.name = "another-test-dbng-group";
    rec2.passwd = "x";
    rec2.count = 4;
    rec2.members = group1_members;

    ret = group.start_txn(&group);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not start txn");
        goto err;
    }

    ret = group.set(&group, (KEY *) &key, (REC *) &rec);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert group record");
        goto err;
    }

    ret = group.set(&group, (KEY *) &key2, (REC *) &rec2);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not insert group record");
        goto err;
    }

    ret = group.commit(&group);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not commit txn");
        goto err;
    }

    /*
     * Attempt to fetch the record. Should succeed as previous
     * transaction was rolled back.
     */
    GROUP_KEY key4;
    GROUP_REC rec4;
    ret = group.get(&group, (KEY *) &key2, (REC *) &rec4);
    if(ret != 0) {
        _result = FAIL;
        warnx("could not fetch group record");
        goto err;
    }

    /*
     * Test iterating over records.
     */
    GROUP_REC *rp;
    int i;
    for(i = 0;
        (ret = group.next(&group, (KEY *) &key4, (REC *) &rec4))
            != DB_NOTFOUND;
        i++)
    {
        if(ret != 0 && ret != DB_NOTFOUND) {
            _result = FAIL;
            warnx("unexpected return code from next: %d", ret);
            goto err;
        }

        switch(rec4.gid) {
        case 1001:
            rp = &rec;
            break;

        case 2001:
            rp = &rec2;
            break;

        default:
            _result = FAIL;
            warnx("unexpected gid: %d", rec4.gid);
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

    if(group.db.cursor != NULL) {
        _result = FAIL;
        warnx("cursor not cleaned up correctly");
        goto err;
    }

    service_cleanup(&group);

err:
    return _result;
}

static int
reccmp(GROUP_REC *a, GROUP_REC *b)
{
    int i;

    if(a->base.type != b->base.type
       || a->gid != b->gid
       || a->count != b->count
       || strcmp(a->name, b->name)
       || strcmp(a->passwd, b->passwd))
    {
        for(i = 0; i < a->count; i++) {
            if(strcmp(a->members[i], b->members[i]) != 0)
                return 0;
        }

        return 1;
    }

    /* The two records are equal. */
    return 0;
}
