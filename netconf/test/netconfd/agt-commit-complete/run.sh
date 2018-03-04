#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
cp startup-cfg.xml tmp
rm /tmp/ncxserver.sock || true
/usr/sbin/netconfd --module=test-agt-commit-complete --startup=tmp/startup-cfg.xml --superuser=$USER 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr &
NETCONFD_PID=$!
sleep 1
python session.py
kill $NETCONFD_PID
cat tmp/netconfd.stdout | grep '#' > tmp/output.txt
cmp expected_output.txt tmp/output.txt
sleep 1
