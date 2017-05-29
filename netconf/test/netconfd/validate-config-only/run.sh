#!/bin/bash
rm -rf tmp || true
mkdir tmp

/usr/sbin/netconfd --validate-config-only --startup-error=stop --startup=startup-cfg-good.xml --module=/usr/share/yuma/modules/ietf/ietf-interfaces.yang --module=/usr/share/yuma/modules/ietf/iana-if-type.yang 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr
GOOD_RES=$?
cat tmp/netconfd.stdout
if [ "$GOOD_RES" != "0" ] ; then
    echo "Error: Failed with startup-cfg-good.xml"
    exit $GOOD_RES
else
    echo "OK: Validated with startup-cfg-good.xml"
fi

/usr/sbin/netconfd --validate-config-only --startup-error=stop --startup=startup-cfg-bad.xml --module=/usr/share/yuma/modules/ietf/ietf-interfaces.yang --module=/usr/share/yuma/modules/ietf/iana-if-type.yang 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr
BAD_RES=$?
cat tmp/netconfd.stdout
if [ "$BAD_RES" == "0" ] ; then
    echo "Error: Did not detect problem with startup-cfg-bad.xml"
    exit -1
else
   echo "OK: Detected problem with startup-cfg-bad.xml"
fi
cat tmp/netconfd.stdout

exit 0
