AM_CPPFLAGS = -DTEST_BASE='"$(TEST_BASE)"'
TESTS = test_passwd_service test_nss_passwd test_dbngctl.sh
TEST_EXTENSIONS = .sh
LOG_COMPILER = $(BASH) ./test-wrapper
SH_LOG_COMPILER = $(BASH)

check_PROGRAMS = test_passwd_service test_nss_passwd
test_passwd_service_LDADD = ../lib/libdbng.la
test_passwd_service_LDFLAGS = -static
test_passwd_service_CFLAGS = -I../lib
test_passwd_service_SOURCES = test_passwd_service.c

test_nss_passwd_LDADD = ../nss/libnss_dbng_test.la
test_nss_passwd_LDFLAGS = -static
test_nss_passwd_CFLAGS = -I../lib -I../nss
test_nss_passwd_SOURCES = test_nss_passwd.c
