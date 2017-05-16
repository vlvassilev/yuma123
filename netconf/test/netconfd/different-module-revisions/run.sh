#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
rm /tmp/ncxserver.sock || true
/usr/sbin/netconfd --validate-config-only --module=./test-different-module-revisions-a.yang --module=./test-different-module-revisions-b.yang --no-startup --superuser=$USER
! /usr/sbin/netconfd --validate-config-only --module=./test-different-module-revisions-a.yang --module=./test-different-module-revisions-b.yang --module=./test-different-module-revisions-c.yang --no-startup --superuser=$USER

/usr/sbin/netconfd --module=./test-different-module-revisions-a.yang --module=./test-different-module-revisions-b.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
SERVER_PID=$!
sleep 3
python session.litenc.py
kill $SERVER_PID
cat tmp/server.stdout
sleep 1
