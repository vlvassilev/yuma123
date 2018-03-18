#!/bin/bash -e

rm -rf tmp || true
mkdir tmp
cd tmp
wget http://www.yang-central.org/twiki/pub/Main/YangTools/rfcstrip
for draft in draft-ietf-netconf-yang-push-15 draft-ietf-netconf-subscribed-notifications-10 draft-ietf-netconf-netconf-event-notifications-08 ; do
    wget https://tools.ietf.org/id/${draft}.txt
    sh ./rfcstrip ${draft}.txt
done
ln -sh ietf-subscribed-notifications@2018-02-23.yang ietf-subscribed-notifications.yang

pyang --ietf -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ ietf-subscribed-notifications.yang
pyang --ietf -f tree --path ./:../../../../modules/ietf-draft/:../../../../modules/ietf/ ietf-yang-push.yang

if [ "$RUN_WITH_CONFD" != "" ] ; then
  killall -KILL confd || true
  echo "Starting confd: $RUN_WITH_CONFD"
  source $RUN_WITH_CONFD/confdrc

  for module in ietf-yang-push.yang ietf-subscribed-notifications.yang ; do
    confdc -c ${module} --yangpath .:/usr/share/yuma/modules
  done
  NCPORT=2022
  NCUSER=admin
  NCPASSWORD=admin
  confd --verbose --foreground --addloadpath ${RUN_WITH_CONFD}/src/confd --addloadpath ${RUN_WITH_CONFD}/src/confd/yang --addloadpath ${RUN_WITH_CONFD}/src/confd/aaa --addloadpath ${RUN_WITH_CONFD}/etc/confd --addloadpath .
  SERVER_PID=$!
  cd ..
else
  killall -KILL netconfd || true
  rm ncxserver.sock || true

  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules --module=./ietf-subscribed-notifications.yang --module=./ietf-yang-push.yang --no-startup --validate-config-only --superuser=$USER
  /usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules --module=./ietf-subscribed-notifications.yang --module=./ietf-yang-push.yang --no-startup --superuser=$USER 2>&1 1>server.log &
   SERVER_PID=$!
  cd ..
fi

sleep 3

python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1

