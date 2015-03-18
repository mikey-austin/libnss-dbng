AM_CPPFLAGS = -DTEST_BASE='"$(TEST_BASE)"'
TESTS = test_passwd_service test_group_service test_shadow_service test_nss_passwd test_nss_shadow test_nss_group test_dbngctl.sh
TEST_EXTENSIONS = .sh
LOG_COMPILER = $(BASH) ./test-wrapper
SH_LOG_COMPILER = $(BASH)

check_PROGRAMS = test_passwd_service test_shadow_service test_group_service test_nss_passwd test_nss_shadow test_nss_group

test_passwd_service_LDADD = ../lib/libdbng.la
test_passwd_service_LDFLAGS = -static
test_passwd_service_CFLAGS = -I../lib
test_passwd_service_SOURCES = test_passwd_service.c

test_shadow_service_LDADD = ../lib/libdbng.la
test_shadow_service_LDFLAGS = -static
test_shadow_service_CFLAGS = -I../lib
test_shadow_service_SOURCES = test_shadow_service.c

test_group_service_LDADD = ../lib/libdbng.la
test_group_service_LDFLAGS = -static
test_group_service_CFLAGS = -I../lib
test_group_service_SOURCES = test_group_service.c

test_nss_passwd_LDADD = ../nss/libnss_dbng_test.la
test_nss_passwd_LDFLAGS = -static
test_nss_passwd_CFLAGS = -I../lib -I../nss
test_nss_passwd_SOURCES = test_nss_passwd.c

test_nss_shadow_LDADD = ../nss/libnss_dbng_test.la
test_nss_shadow_LDFLAGS = -static
test_nss_shadow_CFLAGS = -I../lib -I../nss
test_nss_shadow_SOURCES = test_nss_shadow.c

test_nss_group_LDADD = ../nss/libnss_dbng_test.la
test_nss_group_LDFLAGS = -static
test_nss_group_CFLAGS = -I../lib -I../nss
test_nss_group_SOURCES = test_nss_group.c
