#!/bin/sh
if [ ! -d public ] ; then
    git clone https://github.com/openconfig/public.git
fi

/usr/sbin/netconfd --validate-config-only --startup-error=stop --no-startup --modpath=public/release/models --module=openconfig-network-instance

