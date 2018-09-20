#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
rm /tmp/ncxserver.sock || true
/usr/sbin/netconfd --module=test-val123-api --no-startup --superuser=$USER 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr &
NETCONFD_PID=$!
sleep 2
python session.py
kill $NETCONFD_PID
