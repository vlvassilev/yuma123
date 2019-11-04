AC_INIT([yuma123-systemd-services], [2.12], [vladimir@lightside-instruments.com])
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE([foreign])

netconfmoduledir="$libdir/yuma"
AC_SUBST(netconfmoduledir)

yangdir="$prefix/share/yuma/modules"
AC_SUBST(yangdir)

netconf_ssh_configdir="$sysconfdir/netconf/ssh"
AC_SUBST(netconf_ssh_configdir)

AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
