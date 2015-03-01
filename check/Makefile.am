AM_CPPFLAGS = -DTEST_BASE='"$(TEST_BASE)"'
TESTS = test_service
LOG_COMPILER = sh ./test-wrapper

check_PROGRAMS = test_service
test_service_LDADD = ../lib/libdbng.la
test_service_LDFLAGS = -static
test_service_CFLAGS = -I../lib
test_service_SOURCES = test_service.c
