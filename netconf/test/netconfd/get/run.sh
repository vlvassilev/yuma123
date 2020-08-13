#!/bin/bash -e
killall -KILL netconfd || true
rm -f /tmp/ncxserver.sock
/usr/sbin/netconfd --no-startup --superuser=$USER &
SERVER_PID=$!

sleep 4
python session.litenc.py --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD
kill $SERVER_PID
sleep 1
