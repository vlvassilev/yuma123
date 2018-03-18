#!/bin/bash -e
cd
wget https://nexus.opendaylight.org/content/groups/public/org/opendaylight/integration/distribution-karaf/0.6.3-Carbon/distribution-karaf-0.6.3-Carbon.tar.gz
tar -xzvf distribution-karaf-0.6.3-Carbon.tar.gz
cd distribution-karaf-0.6.3-Carbon/
./bin/start
#install NETCONF support
./bin/client -u karaf feature:install odl-netconf-all odl-netconf-ssh odl-netconf-connector-all
#install GUI support
./bin/client -u karaf feature:install odl-dlux-core odl-dluxapps-applications odl-dluxapps-nodes odl-dluxapps-yangman odl-dluxapps-yangui odl-dluxapps-yangutils odl-dluxapps-yangvisualizer
