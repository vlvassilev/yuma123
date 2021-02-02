#!/bin/bash -e

rm -rf tmp
mkdir tmp
killall -KILL netconfd || true
rm -f /tmp/ncxserver.sock

/usr/sbin/netconfd \
	--deviation=./test-deviation-in-modules-state.yang \
	--module=ietf-interfaces \
	--no-startup \
	--superuser=$USER \
	2>tmp/error.log \
	1>tmp/server.log &

sleep 1

python test-deviation-in-modules-state.py
res=$?

kill %1
wait
exit ${res}
