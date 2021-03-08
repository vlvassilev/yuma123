#!/bin/bash -e

rm -rf tmp
mkdir tmp

killall -KILL netconfd || true
export NCSERVER=localhost
export NCPORT=830
export NCUSER=${USER}
export NCPASSWORD=""
rm -f /tmp/ncxserver.sock
/usr/sbin/netconfd --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
SERVER_PID=$!

sleep 1

expect session.exp
res=$?

kill -KILL $SERVER_PID
wait
exit ${res}
