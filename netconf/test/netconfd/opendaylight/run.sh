#!/bin/bash -e

mkdir tmp || true
cd tmp
wget https://nexus.opendaylight.org/content/groups/public/org/opendaylight/integration/distribution-karaf/0.6.3-Carbon/distribution-karaf-0.6.3-Carbon.tar.gz
tar -xzvf distribution-karaf-0.6.3-Carbon.tar.gz
cd distribution-karaf-0.6.3-Carbon/
./bin/start
#install NETCONF support
./bin/client -u karaf feature:install odl-netconf-all odl-netconf-ssh odl-netconf-connector-all
#install GUI support
./bin/client -u karaf feature:install odl-dlux-core odl-dluxapps-applications odl-dluxapps-nodes odl-dluxapps-yangman odl-dluxapps-yangui odl-dluxapps-yangutils odl-dluxapps-yangvisualizer
cd ../../

#start netconfd nodes - avoid 1830 used by the opendaylight NETCONF server.
killall -KILL netconfd || true
NCPORT0=2830
NCPORT1=3830
rm /tmp/ncxserver.${NCPORT0}.sock || true
/usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules/ --module=iana-if-type --module=ietf-interfaces --no-startup --superuser=$USER --ncxserver-sockname=/tmp/ncxserver.${NCPORT0}.sock --port=${NCPORT0} &
SERVER0_PID=$!
rm /tmp/ncxserver.${NCPORT1}.sock || true
/usr/sbin/netconfd --modpath=.:/usr/share/yuma/modules/ --module=iana-if-type --module=ietf-interfaces --no-startup --superuser=$USER --ncxserver-sockname=/tmp/ncxserver.${NCPORT1}.sock --port=${NCPORT1} &
SERVER1_PID=$!

#configure topology - TODO
python session.litenc.py --server=localhost --port=1830 --user=admin --password=admin
yangcli --server=localhost --user=admin --ncport=1830 --run-command="xget /" --batch-mode --password=admin
#test - TODO
