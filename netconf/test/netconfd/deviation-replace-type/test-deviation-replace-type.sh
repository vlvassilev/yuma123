#!/bin/bash -e

rm -rf tmp
mkdir tmp
killall -KILL netconfd || true
rm -f /tmp/ncxserver.sock

valgrind -- /usr/sbin/netconfd \
	--deviation=./test-deviation-replace-type.yang \
	--module=ietf-interfaces \
	--module=ietf-ip@2014-06-16 \
	--no-startup \
	--superuser=$USER \
	2>tmp/error.log \
	1>tmp/server.log &

SERVER=$!
sleep 5
kill %1
wait

cat tmp/server.log
if grep -q "==${SERVER}==.*typ_clean_typdef" tmp/error.log ; then
	exit 1
fi
exit 0
