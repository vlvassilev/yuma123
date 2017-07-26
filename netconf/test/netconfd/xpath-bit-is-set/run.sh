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
  cp *.yang tmp
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  confdc -c /usr/share/yuma/modules/ietf/ietf-interfaces.yang --yangpath /usr/share/yuma/modules/ietf -o ietf-interfaces.fxs
  confdc -c /usr/share/yuma/modules/ietf/iana-if-type.yang --yangpath /usr/share/yuma/modules/ietf -o iana-if-type.fxs
  confdc -c test-xpath-bit-is-set.yang --yangpath /usr/share/yuma/modules/ietf -o test-xpath-bit-is-set.fxs
  confdc -c test-xpath-bit-is-set-bad-bit-name.yang --yangpath /usr/share/yuma/modules/ietf -o test-xpath-bit-is-set-bad-bit-name.fxs
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  sleep 3
  echo "Loading good.xml ..."
  confd_load -l ../good.xml
  echo "Loading bad.xml"
  expected_fail "confd_load -l ../bad.xml"
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd --module=./test-xpath-bit-is-set.yang --validate-config-only --no-startup
  expected_fail "/usr/sbin/netconfd --module=./test-xpath-bit-is-set-bad-bit-name.yang --validate-config-only --no-startup"
  /usr/sbin/netconfd --module=/usr/share/yuma/modules/ietf/iana-if-type.yang --module=./test-xpath-bit-is-set.yang --validate-config-only --startup=good.xml
  expected_fail "/usr/sbin/netconfd --module=/usr/share/yuma/modules/ietf/iana-if-type.yang --module=./test-xpath-bit-is-set.yang --validate-config-only --startup=bad.xml"
fi
