#!/bin/bash -e

function expected_fail {
  export DID_IT_FAIL=0 ; sh -c "$1" || export DID_IT_FAIL=1 true
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
killall -KILL netconfd || true
rm /tmp/ncxserver.sock || true
#/usr/sbin/netconfd --module=/usr/share/yuma/modules/ietf/ietf-interfaces.yang --module=/usr/share/yuma/modules/ietf/iana-if-type.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
valgrind --log-fd=1 --num-callers=100 --leak-check=full  -- /usr/sbin/netconfd --module=/usr/share/yuma/modules/ietf/ietf-interfaces.yang --module=/usr/share/yuma/modules/ietf/iana-if-type.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
#--log-file=tmp/valgrind.log
SERVER_PID=$!
sleep 20
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
sleep 2
expected_fail "kill -KILL $SERVER_PID"
cat tmp/server.log
#expected_fail "cat tmp/server.log | grep 'memory corruption'"
cat tmp/server.log | grep "ERROR SUMMARY: 0 errors"
sleep 1
