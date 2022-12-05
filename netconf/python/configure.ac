AC_INIT([python-yuma], [2.14], [vladimir@lightside-instruments.com])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([-Werror foreign])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL

pythondir="/usr/lib/python3/dist-packages"
AC_SUBST(pythondir)

PKG_CHECK_MODULES([PYTHON3],[python3])

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1"
AC_SUBST(CFLAGS)

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
