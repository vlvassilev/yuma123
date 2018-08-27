AC_INIT([python-yuma], [2.12], [vladimir@transpacket.com])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([-Werror foreign])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL

pythondir="/usr/lib/python2.7/dist-packages"
AC_SUBST(pythondir)

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1"
AC_SUBST(CFLAGS)

AM_PATH_XML2

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
