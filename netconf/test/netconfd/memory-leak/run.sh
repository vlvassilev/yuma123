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
/usr/sbin/netconfd --module=/usr/share/yuma/modules/ietf/ietf-interfaces.yang --module=/usr/share/yuma/modules/ietf/iana-if-type.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
SERVER_PID=$!
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
sleep 2
expected_fail "kill -KILL $SERVER_PID"
expected_fail "cat tmp/server.log | grep 'memory corruption'"
sleep 1
