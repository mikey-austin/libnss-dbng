AM_CPPFLAGS = -DDEFAULT_BASE='"$(DEFAULT_BASE)"'
lib_LTLIBRARIES = libnss_dbng.la

libnss_dbng_la_LIBADD   = ../lib/libdbng.la
libnss_dbng_la_SOURCES = group.c passwd.c shadow.c
libnss_dbng_la_LDFLAGS = -version-info 2:0:0
