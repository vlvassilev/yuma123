AC_INIT([yuma123], [2.13], [vladimir@lightside-instruments.com])

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS

PKG_PROG_PKG_CONFIG

AC_PROG_LIBTOOL
AC_SEARCH_LIBS([dlopen], [dl dld], [], [
  AC_MSG_ERROR([unable to find the dlopen() function])
])

netconfmoduledir="$libdir/yuma"
AC_SUBST(netconfmoduledir)

yuma_datarootdir=${datarootdir}/yuma
AC_SUBST(yuma_datarootdir)

yangdir=${datarootdir}/yuma/modules
AC_SUBST(yangdir)

ietf_yangdir=${datarootdir}/yuma/modules/ietf
AC_SUBST(ietf_yangdir)

nmda_modules_ietf_yangdir=${datarootdir}/yuma/nmda-modules/ietf
AC_SUBST(nmda_modules_ietf_yangdir)

ietf_draft_yangdir=${datarootdir}/yuma/modules/ietf-draft
AC_SUBST(ietf_draft_yangdir)

ietf_patched_yangdir=${datarootdir}/yuma/modules/ietf-patched
AC_SUBST(ietf_patched_yangdir)

ietf_derived_yangdir=${datarootdir}/yuma/modules/ietf-derived
AC_SUBST(ietf_derived_yangdir)

ietf_expired_yangdir=${datarootdir}/yuma/modules/ietf-expired
AC_SUBST(ietf_expired_yangdir)

netconfcentral_yangdir=${datarootdir}/yuma/modules/netconfcentral
AC_SUBST(netconfcentral_yangdir)

yuma123_yangdir=${datarootdir}/yuma/modules/yuma123
AC_SUBST(yuma123_yangdir)

examples_yangdir="$prefix/share/yuma/modules/examples"
AC_SUBST(examples_yangdir)

ncx_netconf_includedir=${includedir}/yuma/ncx
AC_SUBST(ncx_netconf_includedir)

agt_netconf_includedir=${includedir}/yuma/agt
AC_SUBST(agt_netconf_includedir)

mgr_netconf_includedir=${includedir}/yuma/mgr
AC_SUBST(mgr_netconf_includedir)

yangrpc_netconf_includedir=${includedir}/yuma/yangrpc
AC_SUBST(yangrpc_netconf_includedir)

platform_netconf_includedir=${includedir}/yuma/platform
AC_SUBST(platform_netconf_includedir)

apachemoduledir="$libdir/apache2/modules"
AC_SUBST(apachemoduledir)

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1 -DRELEASE=0 -DENABLE_DIRECT_MUST_AUGMENT_EX=1 -DHAS_FLOAT=1"
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
AM_CONDITIONAL([STANDALONE], [test x$standalone = xtrue])

PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.3.9])
XML_LIBS="$LIBXML2_LIBS"
XML_CPPFLAGS="$LIBXML2_CFLAGS"
AC_SUBST([XML_LIBS])
AC_SUBST([XML_CPPFLAGS])

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
        example-modules/ietf-interfaces/Makefile
        example-modules/ietf-system/Makefile
        example-modules/ietf-network-bridge/Makefile
        example-modules/yangcli-to-rpc/Makefile
        netconf/src/yangrpc/Makefile
        netconf/src/yangrpc/example/Makefile
        netconf/src/yangdump/Makefile
        netconf/man/Makefile
        rpm/yuma123.spec
])

# Disabled targets
#        netconf/src/yangdiff/Makefile

# Test framework
#        netconf/test/Makefile
#        netconf/test/sys-test/Makefile
#        netconf/test/integ-tests/Makefile
#        netconf/test/sys-test-python/Makefile

AC_OUTPUT
