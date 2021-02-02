#!/bin/bash -e

rm -rf tmp
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
  confd \
	--verbose --foreground \
	--addloadpath ${RUN_WITH_CONFD}/src/confd \
	--addloadpath ${RUN_WITH_CONFD}/src/confd \
	--addloadpath ${RUN_WITH_CONFD}/src/confd/yang \
	--addloadpath ${RUN_WITH_CONFD}/src/confd/aaa \
	--addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  export NCSERVER=localhost
  export NCPORT=830
  export NCUSER=${USER}
  export NCPASSWORD=""
  rm -f /tmp/ncxserver.sock
  /usr/sbin/netconfd --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi
sleep 1

expect get-client-modules-list.exp
python test-session-modules-list.py --module-list tmp/modlist
res=$?

kill -KILL $SERVER_PID
wait
exit ${res}
