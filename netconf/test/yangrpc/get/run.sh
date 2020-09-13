#!/bin/bash -e

NCSERVER=localhost
NCPORT=830
NCUSER=$USER
NCPASSWORD=""

killall -KILL netconfd || true
rm -f /tmp/ncxserver.sock
/usr/sbin/netconfd --no-startup --log-level=debug3 --target=running --superuser=$USER &
SERVER_PID=$!

sleep 4
valgrind -v --log-fd=1 --num-callers=100 --leak-check=full --show-leak-kinds=all  -- ./yangrpc-example --server=$NCSERVER --port=$NCPORT --user=$NCUSER --password=$NCPASSWORD 1>session.stdout 2>session.stderr

for var in "$@"
do
    if [ "$var" == "--check-memory-leak" ] ; then
        cat tmp/server.log | grep "ERROR SUMMARY: 0 errors"        
    fi
done
