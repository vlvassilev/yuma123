#!/bin/bash -e
rm -rf tmp
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  exit 0
else
  killall -KILL netconfd || true
  rm -f /tmp/ncxserver.sock
  /usr/sbin/netconfd --modpath=`pwd` --module=test-identity-iffeature --feature-disable=extra-identity --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --modpath=`pwd` --module=test-identity-iffeature --feature-disable=extra-identity --no-startup --superuser=$USER 1>tmp/server.log 2>&1 &
  SERVER_PID=$!
fi

sleep 3
python test-identity-iffeature.py --expectfail
res=$?
killall -KILL netconfd || true

exit ${res}
