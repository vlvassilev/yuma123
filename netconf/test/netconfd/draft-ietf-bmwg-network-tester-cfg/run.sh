#!/bin/bash -e

ip link delete veth1 || true
ip link add veth1 type veth peer name veth2
ifconfig veth1 hw ether 6C:A9:6F:00:00:01
ifconfig veth2 hw ether 6C:A9:6F:00:00:02
ifconfig veth1 192.0.2.1 up
ifconfig veth2 192.0.2.1 up

rm -rf tmp || true
mkdir tmp
cd tmp

#wget https://www.ietf.org/archive/id/draft-ietf-bmwg-network-tester-cfg-05.txt


#xml2rfc draft-ietf-bmwg-network-tester-cfg-05.xml

#rfcstrip draft-ietf-bmwg-network-tester-cfg-05.txt

#pyang -f tree --path . ietf-traffic-generator*.yang
#pyang -f tree --path . ietf-traffic-analyzer*.yang

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
  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules --module=ietf-interfaces --module=ietf-traffic-generator --module=ietf-traffic-analyzer --no-startup --superuser=$USER &
  SERVER_PID=$!
  cd ..
fi

sleep 3
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1
ip link delete veth1 || true

