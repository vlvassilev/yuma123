#!/bin/bash -e

apt-get update
apt-get -y upgrade
apt-get -y install git devscripts

#getting/building/installing
cd ~

if [ -d yuma123-git ] ; then
 git clone yuma123-git yuma123_2.10
else
 git clone https://git.code.sf.net/p/yuma123/git yuma123_2.10
fi

cd yuma123_2.10
mk-build-deps  -i -t 'apt-get -y'                                 
git clean -f

cd ..
tar -czvf yuma123_2.10.orig.tar.gz --exclude .git yuma123_2.10
cd yuma123_2.10
debuild -us -uc
dpkg -i ../*.deb

#build and install python-yuma (used in the testsuite)
cd ~
apt-get -y install rsync
rsync -rav yuma123_2.10/netconf/python/ yuma123-python_2.10
tar -czvf yuma123-python_2.10.orig.tar.gz --exclude .git yuma123-python_2.10
cd yuma123-python_2.10
mk-build-deps  -i -t 'apt-get -y'
cd ..
rm -rf yuma123-python_2.10
tar -xzvf yuma123-python_2.10.orig.tar.gz
cd yuma123-python_2.10
debuild -us -uc
apt-get -y install python-paramiko python-lxml
dpkg -i ../python-yuma*.deb

#testing
ssh-keygen -t rsa -N "" -f ~/.ssh/id_rsa
cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
ssh-keyscan -t rsa -H localhost >> ~/.ssh/known_hosts

echo "Port 830" >> /etc/ssh/sshd_config
echo "Port 1830" >> /etc/ssh/sshd_config
echo 'Subsystem netconf "/usr/sbin/netconf-subsystem --ncxserver-sockname=830@/tmp/ncxserver.sock --ncxserver-sockname=1830@/tmp/ncxserver.1830.sock"' >> /etc/ssh/sshd_config
/etc/init.d/ssh restart

cd ~/yuma123_2.10/netconf/test/netconfd
apt-get -y install python-ncclient valgrind

autoreconf -i -f
./configure --prefix=/usr
make
make install
make check

cd ~/yuma123_2.10/netconf/test/yangcli
apt-get -y install expect

autoreconf -i -f
./configure --prefix=/usr
make
make install
make check

echo "Success!"
