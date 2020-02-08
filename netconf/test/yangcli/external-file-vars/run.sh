#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  confdc -c /usr/share/yuma/modules/ietf/ietf-interfaces@2014-05-08.yang --yangpath /usr/share/yuma/modules/ietf -o ietf-interfaces.fxs
  confdc -c /usr/share/yuma/modules/ietf/iana-if-type@2014-05-08.yang --yangpath /usr/share/yuma/modules/ietf -o iana_if_type.fxs
  export NCSERVER=localhost
  export NCPORT=2022
  export NCUSER=admin
  export NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  export NCSERVER=localhost
  export NCPORT=830
  export NCUSER=${USER}
  export NCPASSWORD=
  /usr/sbin/netconfd --module=/usr/share/yuma/modules/ietf/ietf-interfaces@2014-05-08.yang --module=/usr/share/yuma/modules/ietf/iana-if-type@2014-05-08.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi

sleep 3
yangcli --server=${NCSERVER} --ncport=${NCPORT} --user=${NCUSER} --password=${NCPASSWORD} --batch-mode --run-script=script.yangcli
#kill $SERVER_PID
cat tmp/server.log
sleep 1
