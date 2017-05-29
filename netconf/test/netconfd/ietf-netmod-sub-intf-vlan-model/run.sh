#!/bin/bash -e
rm -rf tmp || true
mkdir tmp
if [ "$RUN_WITH_CONFD" != "" ] ; then
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  cp *.yang tmp
  cd tmp
  confdc -c /usr/share/yuma/modules/ietf/ietf-interfaces.yang --yangpath /usr/share/yuma/modules/ietf
  confdc -c /usr/share/yuma/modules/ietf/iana-if-type.yang --yangpath /usr/share/yuma/modules/ietf
  confdc -c ietf-interfaces-common@2017-03-13.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..
  confdc -c ietf-interfaces-ethernet-like@2017-03-13.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..
  confdc -c ietf-flexible-encapsulation@2017-03-13.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..
  confdc -c ietf-if-l3-vlan@2017-03-13.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..
  confdc -c ieee802-dot1q-types@2016-09-22.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..
  confdc -c ieee802-types@2016-07-24.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..
  confdc -c vlans.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..
  confdc -c composite-match.yang --yangpath /usr/share/yuma/modules/ietf --yangpath ..

  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  #confd --verbose --foreground --addloadpath /home/vladimir/transpacket/confd/root/src/confd --addloadpath . 2>&1 1>server.log &
  confd --verbose --foreground --addloadpath /home/vladimir/transpacket/confd/root/src/confd --addloadpath .
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true
  /usr/sbin/netconfd --module=./ietf-interfaces-common@2017-03-13.yang --module=./ietf-interfaces-ethernet-like@2017-03-13.yang --module=./ietf-if-l3-vlan@2017-03-13.yang --module=/usr/share/yuma/modules/ietf/iana-if-type.yang --module=./vlans.yang --module=./composite-match.yang --modpath=./:/usr/share/yuma/modules/ --no-startup --validate-config-only --superuser=$USER
  exit 0
  SERVER_PID=$!
fi

sleep 3
python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
#kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
