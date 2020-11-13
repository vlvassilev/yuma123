#!/bin/bash -e
rm -rf tmp
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  exit 0
else
  modules="--module=test-deviation-delete-ext"
  killall -KILL netconfd > /dev/null 2>&1 || true
  rm -f /tmp/ncxserver.sock

  # Even with --validate-config-only, this will abort if the problem is
  # not fixed so don't bother starting a daemon we might have to kill at
  # the end of this script.
  /usr/sbin/netconfd --modpath=`pwd` ${modules} --no-startup --superuser=$USER \
                     --validate-config-only 1>tmp/server.log 2>&1
fi
exit 0
