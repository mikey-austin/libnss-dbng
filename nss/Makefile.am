lib_LTLIBRARIES = libnss_dbng.la
noinst_LTLIBRARIES = libnss_dbng_test.la
noinst_HEADERS = nss-dbng.h

libnss_dbng_la_LIBADD   = ../lib/libdbng.la
libnss_dbng_la_SOURCES	= group.c passwd.c shadow.c
libnss_dbng_la_CPPFLAGS = -DDEFAULT_BASE='"$(DEFAULT_BASE)"'
libnss_dbng_la_LDFLAGS	= -version-info 2:0:0

libnss_dbng_test_la_LIBADD   = ../lib/libdbng.la
libnss_dbng_test_la_SOURCES	= group.c passwd.c shadow.c
libnss_dbng_test_la_CPPFLAGS = -DDEFAULT_BASE='"$(TEST_BASE)"' -DDEBUG
libnss_dbng_test_la_LDFLAGS	= -version-info 2:0:0
