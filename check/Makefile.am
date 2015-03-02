AM_CPPFLAGS = -DTEST_BASE='"$(TEST_BASE)"'
TESTS = test_passwd_service
LOG_COMPILER = sh ./test-wrapper

check_PROGRAMS = test_passwd_service
test_passwd_service_LDADD = ../lib/libdbng.la
test_passwd_service_LDFLAGS = -static
test_passwd_service_CFLAGS = -I../lib
test_passwd_service_SOURCES = test_passwd_service.c
