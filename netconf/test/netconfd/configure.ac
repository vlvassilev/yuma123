AC_INIT([netconfd-test-suite], [2.15], [vladimir@lightside-instruments.com])

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

PKG_CHECK_MODULES([LIBXML2], [libxml-2.0])
XML_LIBS="$LIBXML2_LIBS"
XML_CFLAGS="$LIBXML2_CFLAGS -DLIBXML2_ENABLED"
XML_CPPFLAGS="$LIBXML2_CFLAGS -DLIBXML2_ENABLED"
AC_SUBST([XML_LIBS])
AC_SUBST([XML_CPPFLAGS])

CFLAGS="$CFLAGS -g -fPIC -DDEBUG=1 -DLINUX=1 -DGCC=1 -DRELEASE=1"
AC_SUBST(CFLAGS)

AC_CONFIG_FILES([
        Makefile
	multiple-edit-callbacks/Makefile
        rollback-on-error/Makefile
        ietf-routing/Makefile
        ietf-routing-bis/Makefile
        ietf-interfaces-bis/Makefile
        ietf-ip-bis/Makefile
        agt-commit-complete/Makefile
        val123-api/Makefile
        anyxml/Makefile
])

AC_OUTPUT

