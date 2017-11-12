#!/bin/bash -e

#rm -rf tmp || true
#mkdir tmp
cd tmp
#wget https://tools.ietf.org/id/draft-ietf-netmod-rfc8022bis-01.txt
wget https://tools.ietf.org/id/draft-ietf-netconf-nmda-netconf-01.txt
wget https://www.ietf.org/id/draft-ietf-netmod-revised-datastores-06.txt
#wget http://www.yang-central.org/twiki/pub/Main/YangTools/rfcstrip
sh rfcstrip  draft-ietf-netmod-rfc8022bis-01.txt
sh rfcstrip  draft-ietf-netconf-nmda-netconf-01.txt
sh rfcstrip  draft-ietf-netmod-revised-datastores-06.txt

#pyang --ietf -f tree --path ../../../../modules/ietf-draft/:../../../../modules/ietf/ ietf-ipv4-unicast-routing@2017-10-14.yang
#pyang --ietf -f tree --path ../../../../modules/ietf-draft/:../../../../modules/ietf/ ietf-ipv6-unicast-routing@2017-10-14.yang
#pyang --ietf -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ ietf-routing@2017-10-14.yang
pyang -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ example-rip.yang

#exit 0
#pyang --ietf -f tree --path ../../../modules/ietf-draft/:../../../modules/ietf/ ../../../modules/ietf-draft/ietf-ipv4-unicast-routing@2017-10-14.yang
#exit 0

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

  /usr/sbin/netconfd --module=iana-if-type --module=../../../../modules/ietf-draft/ietf-ip@2017-08-21.yang --modpath=.:../../../../modules/ietf:../../../../modules/ietf-draft --module=ietf-ipv4-unicast-routing@2017-10-14 --module=ietf-netconf-datastores --module=ietf-origin  --module=ietf-datastores --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --module=iana-if-type --module=../../../../modules/ietf-draft/ietf-ip@2017-08-21.yang --modpath=.:../../../../modules/ietf:../../../../modules/ietf-draft --module=ietf-ipv4-unicast-routing@2017-10-14 --module=test-ietf-routing-bis --module=ietf-netconf-datastores --module=ietf-origin  --module=ietf-datastores --no-startup --superuser=$USER 2>&1 1>tmp/server.log &
   SERVER_PID=$!
fi

sleep 3
python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
#kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
