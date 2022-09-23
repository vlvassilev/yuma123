AC_INIT([yuma123-netconfd-module-ietf-network-bridge], [2.13], [vladimir@lightside-instruments.com])

AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([-Werror foreign])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_LIBTOOL

AC_ARG_ENABLE([standalone],
[  --enable-standalone    Turn on standalone build],
[case "${enableval}" in
  yes) standalone=true ;;
  no)  standalone=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-standalone]) ;;
esac],[standalone=true])
AM_CONDITIONAL([STANDALONE], [test x$standalone = xtrue])

netconfmoduledir="$libdir/yuma"
AC_SUBST(netconfmoduledir)

examples_yangdir="$prefix/share/yuma/modules/examples"
AC_SUBST(examples_yangdir)

ncx_netconf_includedir="$includedir/yuma/ncx"
AC_SUBST(ncx_netconf_includedir)

agt_netconf_includedir="$includedir/yuma/agt"
AC_SUBST(agt_netconf_includedir)

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1"
AC_SUBST(CFLAGS)

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
