#!/bin/bash -e

rm -rf tmp || true
mkdir tmp
cd tmp
wget http://www.yang-central.org/twiki/pub/Main/YangTools/rfcstrip

wget https://www.rfc-editor.org/rfc/rfc8343.txt
wget https://www.rfc-editor.org/rfc/rfc8344.txt
sh ./rfcstrip rfc8343.txt
sh ./rfcstrip rfc8344.txt
pyang --ietf -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ ietf-interfaces@2018-02-20.yang
pyang -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ example-ethernet-bonding.yang
pyang -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ example-ethernet.yang
pyang -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ example-vlan.yang
pyang --ietf -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ ietf-ip@2018-02-22.yang

if [ "$RUN_WITH_CONFD" != "" ] ; then
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  for module in ietf-interfaces@2018-02-20 ietf-ip@2018-02-22 example-ethernet-bonding example-ethernet example-vlan ../../../../modules/ietf/iana-if-type@2014-05-08; do
    confdc -c ${module}.yang --yangpath .:../../../../modules/ietf
  done
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath ${RUN_WITH_CONFD}/etc/confd --addloadpath . 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true

  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules --module=iana-if-type --module=./ietf-interfaces@2018-02-20.yang --module=./ietf-ip@2018-02-22.yang --module=example-ethernet-bonding.yang  --module=example-ethernet.yang  --module=example-vlan.yang --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --with-nmda=true --modpath=.:/usr/share/yuma/modules --module=iana-if-type --module=./ietf-interfaces@2018-02-20.yang --module=./ietf-ip@2018-02-22.yang --module=example-ethernet-bonding.yang  --module=example-ethernet.yang  --module=example-vlan.yang --module=test-ietf-ip-bis --no-startup --superuser=$USER 2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
fi

sleep 3
python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
#python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
