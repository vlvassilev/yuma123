#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  cd tmp
  for module in ietf-interfaces iana-if-type ietf-ip ietf-routing@2016-11-04 ietf-ipv4-unicast-routing@2016-11-04 ; do
    cp ../../../../modules/ietf/${module}.yang .
    confdc -c ${module}.yang --yangpath ../../../../
  done
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath ${RUN_WITH_CONFD}/etc/confd --addloadpath .
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd --module=iana-if-type --modpath=../../../modules/ietf --module=../../../modules/ietf/ietf-ipv4-unicast-routing@2016-11-04.yang --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --module=iana-if-type --modpath=../../../modules/ietf --module=../../../modules/ietf/ietf-ipv4-unicast-routing@2016-11-04.yang --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
  SERVER_PID=$!
fi

sleep 3
python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
