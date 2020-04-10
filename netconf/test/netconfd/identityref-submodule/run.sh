#!/bin/bash
killall -KILL netconfd || true
rm /tmp/ncxserver.sock || true
/usr/sbin/netconfd --module=./mod1.yang --modpath=./:/usr/share/yuma/modules/ --startup-error=stop --validate-config-only --superuser=$USER
