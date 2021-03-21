#!/bin/bash -e
rm -rf tmp || true
mkdir tmp

#netconfd
killall -KILL netconfd || true
rm /tmp/ncxserver.sock || true
echo '<config/>' > tmp/startup-cfg.xml
/usr/sbin/netconfd --module=mod1.yang  --startup=tmp/startup-cfg.xml --superuser=$USER 2>&1 1>tmp/server.log &
SERVER_PID=$!

sleep 3

python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=configure-nacm

python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=create --option=foo
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=create --option=bar
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=create --option=baz
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=replace --option=foo
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=replace --option=bar
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=replace --option=baz
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=get --option=foo
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=get --option=bar
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=get --option=baz

kill -KILL $SERVER_PID
cat tmp/server.log
sleep 1

rm /tmp/ncxserver.sock || true
/usr/sbin/netconfd --module=mod1.yang  --startup=tmp/startup-cfg.xml --superuser=root 2>&1 1>tmp/server.log &
SERVER_PID=$!

sleep 3


python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=replace --option=foo
! python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=replace --option=bar || exit 1
! python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=replace --option=baz || exit 1
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=get --option=foo
python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=get --option=bar
! python session.yangcli.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD --operation=get --option=baz || exit 1

#kill -KILL $SERVER_PID
#cat tmp/server.log

