#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  confdc -c ../mod1.yang --yangpath .. -o mod1.fxs
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd --module=mod1.yang  --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi
sleep 3

python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD

kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
