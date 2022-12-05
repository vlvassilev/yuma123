AC_INIT([yuma123-systemd-services], [2.14], [vladimir@lightside-instruments.com])
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([foreign])

netconfmoduledir="$libdir/yuma"
AC_SUBST(netconfmoduledir)

yangdir="$prefix/share/yuma/modules"
AC_SUBST(yangdir)

netconfd_ssh_configdir="$sysconfdir/netconf/ssh"
AC_SUBST(netconfd_ssh_configdir)

netconfd_statedir="$localstatedir/lib/netconfd"
AC_SUBST(netconfd_statedir)

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
