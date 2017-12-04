#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  #not implemented for confd - SKIP
  exit 77
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  cd tmp
  for module in ietf-interfaces iana-if-type ; do
    cp ../../../../modules/ietf/${module}.yang .
    confdc -c ${module}.yang --yangpath ../../../../
  done
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath ${RUN_WITH_CONFD}/etc/confd --addloadpath .  &
  SERVER_PID=$!
  cd ..
else
  NCPORT0=830
  NCPORT1=1830
  killall -KILL netconfd || true
  rm /tmp/ncxserver*.sock || true
  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules/ --module=iana-if-type --module=test-multi-instance@2017-12-01 --no-startup --superuser=$USER --ncxserver-sockname=/tmp/ncxserver.sock --port=830 &
  SERVER0_PID=$!
  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules/ --module=iana-if-type --module=test-multi-instance@2017-12-02 --no-startup --superuser=$USER --ncxserver-sockname=/tmp/ncxserver.${NCPORT1}.sock --port=1830 &
  SERVER1_PID=$!
fi

sleep 4
python session.yangcli.py --server=$NCSERVER --port0=$NCPORT0 --port1=$NCPORT1 --user=$NCUSER --password=$NCPASSWORD
#kill -KILL $SERVER0_PID $SERVER1_PID
sleep 1
