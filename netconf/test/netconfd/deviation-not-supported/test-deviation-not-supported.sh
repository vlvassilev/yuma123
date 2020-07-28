#!/bin/bash -e
rm -rf tmp
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  exit 0
else
  modules="--module=test-deviation-not-supported-1"
  modules+=" --module=test-deviation-not-supported-2"
  modules+=" --module=test-deviation-not-supported-3"
  modules+=" --module=test-deviation-not-supported-4"
  killall -KILL netconfd || true
  rm -f /tmp/ncxserver.sock
  /usr/sbin/netconfd --modpath=`pwd` ${modules} --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --modpath=`pwd` ${modules} --no-startup --superuser=$USER 1>tmp/server.log 2>&1 &
  SERVER_PID=$!
fi

sleep 1
python test-deviation-not-supported.py
res=$?
killall -KILL netconfd || true
exit ${res}
