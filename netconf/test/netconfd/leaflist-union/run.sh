#!/bin/bash -e
rm -rf tmp
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  exit 0
else
  killall -KILL netconfd || true
  rm -f /tmp/ncxserver.sock
  /usr/sbin/netconfd --module=ietf-inet-types --module=leaflist-union-test --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --module=ietf-inet-types --module=leaflist-union-test --no-startup --superuser=$USER 1>tmp/server.log 2>&1 &
  SERVER_PID=$!
fi

sleep 3
python session.litenc.py

cat tmp/server.log
sleep 1

# it's ok for the grep command below to fail
set +e
grep -q 'get_instance_string.*Assertion' tmp/server.log && exit 1
exit 0
