#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  confdc -c /usr/share/yuma/modules/ietf/ietf-interfaces@2014-05-08.yang --yangpath /usr/share/yuma/modules/ietf -o ietf-interfaces.fxs
  confdc -c /usr/share/yuma/modules/ietf/iana-if-type@2014-05-08.yang --yangpath /usr/share/yuma/modules/ietf -o iana-if-type.fxs

  if [[ $# -eq 0 ]] ; then
    confdc -c ../test-xpath-re-match.yang --yangpath /usr/share/yuma/modules/ietf -o test-xpath-re-match.fxs
  elif [[ "$1" -eq "concat" ]] ; then
    confdc -c ../test-xpath-re-match-concat.yang --yangpath /usr/share/yuma/modules/ietf -o test-xpath-re-match-concat.fxs
  fi

  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
#  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath .
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  if [[ $# -eq 0 ]] ; then
    /usr/sbin/netconfd --module=./test-xpath-re-match.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  elif [[ "$1" -eq "concat" ]] ; then
    /usr/sbin/netconfd --module=./test-xpath-re-match-concat.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  fi
  SERVER_PID=$!
fi
sleep 3
if [[ $# -eq 0 ]] ; then
  python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
fi
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
