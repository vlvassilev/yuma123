AC_INIT([netconfd-test-suite], [2.12], [vladimir@transpacket.com])

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

AM_PATH_XML2

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
])

AC_OUTPUT

