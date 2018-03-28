#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  cp *.yang tmp
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  confdc -c ../server-modules/test-unversioned-module-imports@2018-03-27.yang -o test-unversioned-module-imports@2018-03-27.fxs
  confdc -c ../server-modules/test-unversioned-module-imports-augment@2018-03-27.yang -o test-unversioned-module-imports-augment@2018-03-27.fxs
  export NCSERVER=localhost
  export NCPORT=2022
  export NCUSER=admin
  export NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  export NCSERVER=localhost
  export NCPORT=830
  export NCUSER=${USER}
  export NCPASSWORD=""
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd --modpath=./server-modules --module=./server-modules/test-unversioned-module-imports-augment@2018-03-27.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi
sleep 3
cd client-modules
expect ../session.exp
cd ..
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
