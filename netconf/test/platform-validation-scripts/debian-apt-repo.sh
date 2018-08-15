#!/bin/bash -e

apt-get update
apt-get -y upgrade

apt-get install software-properties-common
add-apt-repository "deb http://yuma123.org/repos/apt/debian sid main"
wget -O - http://yuma123.org/repos/yuma123.gpg.key | sudo apt-key add -

apt-get -y install netconfd yangcli netconfd-module-ietf-interfaces netconfd-module-ietf-system
apt-get -y install python-yuma || true
apt-get -y install libapache2-mod-yangrpc-example || true

#testing
ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa
cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
ssh-keyscan -t rsa -H localhost >> ~/.ssh/known_hosts
echo "PermitRootLogin yes" >> /etc/ssh/sshd_config
echo "Port 22" >> /etc/ssh/sshd_config
echo "Port 830" >> /etc/ssh/sshd_config
echo "Port 1830" >> /etc/ssh/sshd_config
echo "Port 2830" >> /etc/ssh/sshd_config
recho "Port 3830" >> /etc/ssh/sshd_config
echo "Port 4830" >> /etc/ssh/sshd_config
echo "Port 5830" >> /etc/ssh/sshd_config
echo "Port 6830" >> /etc/ssh/sshd_config
echo 'Subsystem netconf "/usr/sbin/netconf-subsystem --ncxserver-sockname=830@/tmp/ncxserver.sock --ncxserver-sockname=1830@/tmp/ncxserver.1830.sock --ncxserver-sockname=2830@/tmp/ncxserver.2830.sock --ncxserver-sockname=3830@/tmp/ncxserver.3830.sock --ncxserver-sockname=4830@/tmp/ncxserver.4830.sock --ncxserver-sockname=5830@/tmp/ncxserver.5830.sock --ncxserver-sockname=6830@/tmp/ncxserver.6830.sock"' >> /etc/ssh/sshd_config
/etc/init.d/ssh restart


apt-get source yuma123


cd ~/yuma123_*/netconf/test/netconfd
apt-get -y install python-ncclient valgrind

multiarch=$(dpkg-architecture -q DEB_BUILD_MULTIARCH)
prefix="/usr"
CONFIGURE_FLAGS="--prefix=/${prefix} --libdir=${prefix}/lib/${multiarch} --libexecdir=${prefix}/lib/${multiarch}"

autoreconf -i -f
./configure ${CONFIGURE_FLAGS}
make
make install
make check || true

cd ~/yuma123_*/netconf/test/yangcli
apt-get -y install expect

autoreconf -i -f
./configure ${CONFIGURE_FLAGS}
make
make install
make check || true

cd ~/yuma123_*/netconf/test/yangdump

autoreconf -i -f
./configure ${CONFIGURE_FLAGS}
make
make install
make check || true

echo "Success!"
