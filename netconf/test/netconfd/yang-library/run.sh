#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  cp *.yang tmp
  cd tmp
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  confdc -c test-yang-library.yang -F test-yang-library:foo --deviation test-yang-library-deviation.yang --yangpath . -o test-yang-library.fxs
  # imported not implemented - how ?
  confdc -c test-yang-library-import.yang --yangpath . -o test-yang-library-import.fxs
  confdc -c test-yang-library-deviation.yang --yangpath . -o test-yang-library-deviation.fxs
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd --module=./test-yang-library.yang --module=./test-yang-library-deviation.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi
sleep 3
python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
