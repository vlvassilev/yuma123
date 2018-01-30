#!/bin/bash -e

apt-get update
apt-get -y upgrade
apt-get -y install git devscripts

#getting/building/installing
cd ~

if [ -d yuma123-git ] ; then
 echo "Using preexisting yuma123-git"
else
 git clone https://git.code.sf.net/p/yuma123/git yuma123-git
fi

cd yuma123-git
dpkg_ver=`dpkg-parsechangelog --show-field Version`
#deb package version e.g. 2.10-0
echo "dpkg_ver=${dpkg_ver}"
#strip the deb package revision e.g. 2.10-0 -> 2.10
ver=`echo ${dpkg_ver} | sed 's/-[^-]*//'`

mk-build-deps  -i -t 'apt-get -y'
git clean -f

cd ..
ln -s yuma123-git yuma123_${ver}
tar -czvf yuma123_${ver}.orig.tar.gz --exclude .git yuma123-git
cd yuma123_${ver}
debuild -us -uc
dpkg -i ../*.deb

#build and install python-yuma (used in the testsuite)
cd ~
apt-get -y install rsync
rsync -rav yuma123_${ver}/netconf/python/ yuma123-python_${ver}
tar -czvf yuma123-python_${ver}.orig.tar.gz --exclude .git yuma123-python_${ver}
cd yuma123-python_${ver}
mk-build-deps  -i -t 'apt-get -y'
cd ..
rm -rf yuma123-python_${ver}
tar -xzvf yuma123-python_${ver}.orig.tar.gz
cd yuma123-python_${ver}
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

cd ~/yuma123_${ver}/netconf/test/netconfd
apt-get -y install python-ncclient valgrind

autoreconf -i -f
./configure --prefix=/usr
make
make install
make check

cd ~/yuma123_${ver}/netconf/test/yangcli
apt-get -y install expect

autoreconf -i -f
./configure --prefix=/usr
make
make install
make check

cd ~/yuma123_${ver}/netconf/test/yangdump

autoreconf -i -f
./configure --prefix=/usr
make
make install
make check

echo "Success!"
