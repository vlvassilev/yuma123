AC_INIT([apache-yangrpc-example-module], [2.11], [vladimir@transpacket.com])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([foreign])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL


apachemodulelibdir="$libdir/apache2/modules"
AC_SUBST(apachemodulelibdir)

apachemoduleconfigdir="/etc/apache2/mods-available"
AC_SUBST(apachemoduleconfigdir)

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1 -DRELEASE=1"
AC_SUBST(CFLAGS)

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT

