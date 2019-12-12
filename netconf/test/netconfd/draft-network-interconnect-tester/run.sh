#!/bin/bash -e

rm -rf tmp || true
mkdir tmp
cd tmp
wget http://www.yang-central.org/twiki/pub/Main/YangTools/rfcstrip

wget https://www.rfc-editor.org/rfc/rfc6991.txt
wget https://www.rfc-editor.org/rfc/rfc7224.txt
wget https://www.rfc-editor.org/rfc/rfc8343.txt
wget https://www.ietf.org/id/draft-vassilev-bmwg-network-interconnect-tester-02.txt

#xml2rfc draft-vassilev-bmwg-network-interconnect-tester-00.xml
sh ./rfcstrip rfc6991.txt
sh ./rfcstrip rfc7224.txt
sh ./rfcstrip rfc8343.txt
sh ./rfcstrip draft-vassilev-bmwg-network-interconnect-tester-02.txt

pyang -f tree --path . ietf-traffic-generator*.yang
pyang -f tree --path . ietf-traffic-analyzer*.yang
pyang -f tree --path . ietf-loopback*.yang

if [ "$RUN_WITH_CONFD" != "" ] ; then
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc
  for module in `find ./ -name '*.yang' -exec basename {} .yang \;` ; do
    echo confdc -c ${module}.yang --yangpath .
    confdc -c ${module}.yang --yangpath .
  done
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath ${RUN_WITH_CONFD}/etc/confd --addloadpath .
  #2>&1 1>server.log &
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm /tmp/ncxserver.sock || true

  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules --module=ietf-traffic-generator --module=ietf-traffic-analyzer --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules --module=ietf-traffic-generator --module=ietf-traffic-analyzer --no-startup --superuser=$USER
  SERVER_PID=$!
  cd ..
fi

sleep 3
#python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
#python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
#kill -KILL $SERVER_PID
#cat tmp/server.log
sleep 1
