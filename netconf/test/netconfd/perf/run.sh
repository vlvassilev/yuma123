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
  confdc -c /usr/share/yuma/modules/ietf/ietf-system@2014-08-06.yang --yangpath /usr/share/yuma/modules/ietf -o ietf-system.fxs
  confdc -c /usr/share/yuma/modules/ietf-draft/ietf-network-bridge.yang --yangpath /usr/share/yuma/modules/ietf-draft:/usr/share/yuma/modules/ietf -o ietf-network-bridge.fxs
  confdc -c /usr/share/yuma/modules/ietf-draft/ietf-network-bridge-flows.yang --yangpath /usr/share/yuma/modules/ietf-draft:/usr/share/yuma/modules/ietf -o ietf-network-bridge-flows.fxs
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd \
--module=/usr/share/yuma/modules/ietf/ietf-interfaces@2014-05-08.yang \
--module=/usr/share/yuma/modules/ietf/iana-if-type@2014-05-08.yang \
--module=/usr/share/yuma/modules/ietf/ietf-system@2014-08-06.yang \
--module=/usr/share/yuma/modules/ietf-draft/ietf-network-bridge.yang \
--module=/usr/share/yuma/modules/ietf-draft/ietf-network-bridge-flows.yang \
--no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi
sleep 5

STARTTIME=$(date +%s)
time python session.flows.litenc.py --skip-hello=false --connections-count=1 --interfaces-count=64 --bridge-flows-enable=true --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
ENDTIME=$(date +%s)
kill -KILL $SERVER_PID
wc tmp/server.log
sleep 1
echo "It took $(($ENDTIME-$STARTTIME)) seconds to commit the configuration" 
if [ 20 -lt $(($ENDTIME-$STARTTIME)) ] ; then
  echo "Test failed since the PASS threshold is 20 seconds"
  return 1
fi
