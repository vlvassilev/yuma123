AC_INIT([yangcli-test-suite], [2.15], [vladimir@lightside-instruments.com])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([foreign])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL

netconfmoduledir="$libdir/yuma"
AC_SUBST(netconfmoduledir)

yangdir="$prefix/share/yuma/modules"
AC_SUBST(yangdir)

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1 -DRELEASE=1"
AC_SUBST(CFLAGS)

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT

