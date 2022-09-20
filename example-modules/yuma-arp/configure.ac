AC_INIT([yuma123-netconfd-module-yuma-arp], [2.13], [vladimir@lightside-instruments.com])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([-Werror foreign])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS

PKG_PROG_PKG_CONFIG

AC_PROG_LIBTOOL

netconfmoduledir="$libdir/yuma"
AC_SUBST(netconfmoduledir)

netconfcentral_yangdir="$prefix/share/yuma/modules/netconfcentral"
AC_SUBST(netconfcentral_yangdir)

ncx_netconf_includedir="$includedir/yuma/ncx"
AC_SUBST(ncx_netconf_includedir)

agt_netconf_includedir="$includedir/yuma/agt"
AC_SUBST(agt_netconf_includedir)

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1"
AC_SUBST(CFLAGS)

PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.3.9])
XML_LIBS="$LIBXML2_LIBS"
XML_CPPFLAGS="$LIBXML2_CFLAGS"
AC_SUBST([XML_LIBS])
AC_SUBST([XML_CPPFLAGS])

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
