#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  export NCSERVER=localhost
  export NCPORT=2022
  export NCUSER=admin
  export NCPASSWORD=admin
  confdc -c mod1.yang --yangpath . -o mod1.fxs
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
  /usr/sbin/netconfd --no-startup --module=mod1.yang --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi
sleep 3
expect session.exp
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
