AC_PREREQ([2.68])
AC_INIT([libnss-dbng], [0.1.0], [mikey@jackiemclean.net])
AM_INIT_AUTOMAKE
LT_INIT([dlopen])
AC_CONFIG_SRCDIR([lib/dbng.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PATH_PROG([VALGRIND], [valgrind])
AC_PATH_PROG([BASH], [bash])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h syslog.h unistd.h])

AC_ARG_VAR([DEFAULT_BASE], [default base for database files])
if test -z ${DEFAULT_BASE}; then
    DEFAULT_BASE="${localstatedir}/dbng"
fi

AC_ARG_VAR([TEST_BASE], [test base for database files])
if test -z ${TEST_BASE}; then
    TEST_BASE="/tmp/"
fi

AC_ARG_ENABLE([debug], [AS_HELP_STRING([--enable-debug], [build the library in debug mode])],
    [debug=yes], [debug=no])

if test "x${debug}" = xyes; then
    AC_DEFINE([DEBUG], [1], [Debug mode])
fi

have_bdb=no
AC_SEARCH_LIBS([db_env_create], [db-5.3 db5 db4 db], [have_bdb=yes])
if test "x${have_bdb}" = xyes; then
    AC_CHECK_HEADERS([db5/db.h db4/db.h db.h], [have_bdb=yes;break], [have_bdb=no])
fi

if test "x${have_bdb}" = xyes; then
    AC_DEFINE([HAVE_LIBDB], [1], [Berkeley DB development library])
else
    AC_MSG_FAILURE([libdb is required])
fi

AC_CHECK_FUNCS([strerror])
AC_CONFIG_FILES([Makefile lib/Makefile nss/Makefile check/Makefile dbngctl/Makefile check/test-wrapper check/test_dbngctl.sh])

AC_OUTPUT
