#!/bin/bash -ex
MODULES="\
/usr/share/yuma/modules/ietf/ietf-interfaces@2014-05-08.yang \
/usr/share/yuma/modules/ietf/iana-if-type@2014-05-08.yang \
./test-xpath-deref.yang"

SESSION_SCRIPT=session.litenc.py

if [ "$#" -gt 0 ] ; then
  if [ "$1" == "2" ] ; then
    MODULES="${MODULES} ./mod2.yang"
    SESSION_SCRIPT=session-2.litenc.py
  fi
fi

if [ "$RUN_WITH_CONFD" != "" ] ; then
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  for module in $MODULES ; do
    confdc -c $module --yangpath /usr/share/yuma/modules/ietf -o "`basename ${module}`".fxs
  done
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  MODULE_ARGS=""
  for module in $MODULES ; do
    MODULE_ARGS="${MODULE_ARGS} --module=${module}"
  done

  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd $MODULE_ARGS --no-startup --superuser=$USER 2>&1 1>server.log &
  SERVER_PID=$!
fi
sleep 3
python ${SESSION_SCRIPT} --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
kill -KILL $SERVER_PID
cat server.log
sleep 1
