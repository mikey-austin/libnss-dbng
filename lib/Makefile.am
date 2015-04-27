AM_CPPFLAGS = -DDEFAULT_BASE='"$(DEFAULT_BASE)"' -DMIN_UID='$(MIN_UID)' -DMIN_GID='$(MIN_GID)'
lib_LTLIBRARIES = libdbng.la
include_HEADERS = service.h dbng.h service-passwd.h service-group.h service-shadow.h
noinst_HEADERS = utils.h

libdbng_la_SOURCES = dbng.c service.c utils.c service-passwd.c service-group.c service-shadow.c
libdbng_la_LDFLAGS = -version-info 0:0:0
