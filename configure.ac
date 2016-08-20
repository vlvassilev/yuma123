AC_INIT([yuma123], [2.9], [vladimir@transpacket.com])

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

ietf_yangdir="$prefix/share/yuma/modules/ietf"
AC_SUBST(ietf_yangdir)

ietf_draft_yangdir="$prefix/share/yuma/modules/ietf-draft"
AC_SUBST(ietf_draft_yangdir)

netconfcentral_yangdir="$prefix/share/yuma/modules/netconfcentral"
AC_SUBST(netconfcentral_yangdir)

yuma123_yangdir="$prefix/share/yuma/modules/yuma123"
AC_SUBST(yuma123_yangdir)

yang_yangdir="$prefix/share/yuma/modules/yang"
AC_SUBST(yang_yangdir)

ncx_netconf_includedir="$prefix/include/yuma/ncx"
AC_SUBST(ncx_netconf_includedir)

agt_netconf_includedir="$prefix/include/yuma/agt"
AC_SUBST(agt_netconf_includedir)

mgr_netconf_includedir="$prefix/include/yuma/mgr"
AC_SUBST(mgr_netconf_includedir)

yangrpc_netconf_includedir="$prefix/include/yuma/yangrpc"
AC_SUBST(yangrpc_netconf_includedir)

platform_netconf_includedir="$prefix/include/yuma/platform"
AC_SUBST(platform_netconf_includedir)

apachemoduledir="$libdir/apache2/modules"
AC_SUBST(apachemoduledir)

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1 -DRELEASE=0 -DENABLE_DIRECT_MUST_AUGMENT_EX=1"
AC_SUBST(CFLAGS)

#libreadline<default> or libtecla
AC_ARG_WITH(readline,
	[AS_HELP_STRING([--with-readline],
        [Use readline instead of tecla for interctive commandline yangcli input])],
	[READLINE="1"],[])
#	[AC_MSG_ERROR([--with-readline option is mandatory])])
AM_CONDITIONAL([WITH_READLINE], [test "x$READLINE" = x1])

AC_ARG_WITH(tecla,
	[AS_HELP_STRING([--with-tecla],
        [Use tecla instead of readline for interctive commandline yangcli input])],
	[TECLA="1"],[])
AM_CONDITIONAL([WITH_TECLA], [test "x$TECLA" = x1])


AC_CONFIG_FILES([
        Makefile \
        libtecla/Makefile
        netconf/src/netconfd/Makefile
        netconf/src/agt/Makefile
        netconf/modules/Makefile
        netconf/src/mgr/Makefile
        netconf/src/ncx/Makefile
        netconf/src/yangcli/Makefile
        netconf/src/subsys/Makefile
        libtoaster/src/Makefile
        example-modules/helloworld/Makefile
        netconf/src/yangrpc/Makefile
        netconf/src/yangrpc/example/Makefile
        netconf/src/yangdump/Makefile
        netconf/man/Makefile
])
# Test framework
#        netconf/test/Makefile
#        netconf/test/sys-test/Makefile
#        netconf/test/integ-tests/Makefile
#        netconf/test/sys-test-python/Makefile

AC_OUTPUT
