AM_CPPFLAGS = -DDEFAULT_BASE='"$(DEFAULT_BASE)"'
lib_LTLIBRARIES = libdbng.la
include_HEADERS = dbng/service.h dbng/dbng.h
noinst_HEADERS = dbng/utils.h service-passwd.h service-group.h service-shadow.h

libdbng_la_SOURCES = dbng.c service.c utils.c service-passwd.c service-group.c service-shadow.c
libdbng_la_LDFLAGS = -version-info 0:0:0
