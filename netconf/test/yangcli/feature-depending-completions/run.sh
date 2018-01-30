#!/bin/bash -e
function expected_fail {
  export DID_IT_FAIL=0 ; $1 || export DID_IT_FAIL=1 true
  if [ "$DID_IT_FAIL" = "1" ] ; then
    #echo "GOOD ERROR"
    return 0
  else
    echo "ERROR: Expected FAIL returned OK."
    return -1
  fi
}

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
  export NCSERVER=localhost
  export NCPORT=830
  export NCUSER=${USER}
  export NCPASSWORD=""
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd --module=/usr/share/yuma/modules/ietf/ietf-interfaces@2014-05-08.yang --module=/usr/share/yuma/modules/ietf/iana-if-type@2014-05-08.yang --feature-disable=ietf-interfaces:if-mib --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi
sleep 3
expected_fail "expect session.exp"
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
