#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  confdc -c ../main-module.yang --yangpath /usr/share/yuma/modules/ietf:.. -o main-module.fxs
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . --addloadpath .. 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm -f /tmp/ncxserver.sock
  /usr/sbin/netconfd --module=main-module.yang --no-startup --superuser=$USER 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr &
  SERVER_PID=$!
fi

sleep 10
python session.litenc.py
kill $SERVER_PID
cat tmp/netconfd.stdout
sleep 1
