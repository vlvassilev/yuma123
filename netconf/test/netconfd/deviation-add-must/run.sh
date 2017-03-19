#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
rm /tmp/ncxserver.sock || true
/usr/sbin/netconfd --module=./test-deviation-add-must.yang --module=iana-if-type --no-startup --superuser=$USER 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr &

NETCONFD_PID=$!
sleep 3
python session.ncclient.py
kill $NETCONFD_PID
cat tmp/netconfd.stdout
