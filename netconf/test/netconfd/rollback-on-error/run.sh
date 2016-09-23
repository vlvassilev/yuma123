#!/bin/bash -e
rm -rf tmp
mkdir tmp
cp startup-cfg.xml tmp
/usr/sbin/netconfd --module=test-rollback-on-error --startup=tmp/startup-cfg.xml --superuser=$USER 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr &
NETCONFD_PID=$!
sleep 1
python session.py
cat /tmp/std*
kill $NETCONFD_PID
