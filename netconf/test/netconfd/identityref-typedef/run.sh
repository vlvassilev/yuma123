#!/bin/bash
killall -KILL netconfd || true
rm /tmp/ncxserver.sock || true
/usr/sbin/netconfd --module=./b.yang --modpath=./:/usr/share/yuma/modules/ --startup=./good.xml --startup-error=stop --validate-config-only --superuser=$USER
GOOD_RES=$?
if [ "$GOOD_RES" != "0" ] ; then
    echo "Error: Failed with good.xml"
    exit $GOOD_RES
else
    echo "OK: Succeeded with good.xml"
fi
/usr/sbin/netconfd --module=./b.yang --modpath=./:/usr/share/yuma/modules/ --startup=./bad.xml --startup-error=stop --validate-config-only --superuser=$USER
BAD_RES=$?
if [ "$BAD_RES" == "0" ] ; then
    echo "Error: Did not detect problem with bad.xml"
    exit -1
else
    echo "OK: Detected problem with bad.xml"
fi
exit 0
