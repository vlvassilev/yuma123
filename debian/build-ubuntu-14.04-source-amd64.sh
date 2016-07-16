#!/bin/bash -ex
cd ..
sudo apt-get install -y devscripts
git clone yuma123-git yuma123_2.6
rm -rf yuma123_2.6/.git ; tar -czvf yuma123_2.6.orig.tar.gz yuma123_2.6
cd yuma123_2.6
#dpkg-checkbuilddeps
sudo apt-get install -y debhelper autoconf dh-autoreconf libncurses5-dev libssh2-1-dev libxml2-dev libtool libreadline-dev zlib1g-dev libssl-dev
debuild -us -uc
sudo apt-get install -y libwrap0-dev
sudo dpkg -i ../libyuma_2.6-1_amd64.deb  ../libyuma-base_2.6-1_all.deb  ../libyuma-dbg_2.6-1_amd64.deb  ../libyuma-dev_2.6-1_amd64.deb  ../netconfd_2.6-1_amd64.deb  ../yangcli_2.6-1_amd64.deb
cd netconf
tar -czvf yuma123-python_1.0.orig.tar.gz python
cd python
#dpkg-checkbuilddeps
sudo apt-get install -y python2.7-dev
debuild -us -uc
cd ..
tar -czvf yuma123-perl_1.0.orig.tar.gz perl
cd perl
debuild -us -uc
cd ../../example-modules
tar -czvf yuma123-netconfd-module-ietf-interfaces_1.0.orig.tar.gz ietf-interfaces
cd ietf-interfaces
debuild -us -uc
cd ../
tar -czvf yuma123-netconfd-module-ietf-system_1.0.orig.tar.gz ietf-system
cd ietf-system
debuild -us -uc
cd ../../../
mkdir yuma123_2.6_release
cp ./libyuma_2.6-1_amd64.deb \
./yangcli_2.6-1_amd64.deb \
./netconfd_2.6-1_amd64.deb \
./libyuma-base_2.6-1_all.deb \
./yuma123_2.6/netconf/python-yuma_1.0-1_amd64.deb \
./yuma123_2.6/netconf/libyuma-perl_1.0-1_amd64.deb \
./yuma123_2.6/example-modules/netconfd-module-ietf-system_1.0-1_amd64.deb \
./yuma123_2.6/example-modules/netconfd-module-ietf-interfaces_1.0-1_amd64.deb \
./libyuma-dev_2.6-1_amd64.deb \
./libyuma-dbg_2.6-1_amd64.deb \
./yuma123_2.6-1.* \
./yuma123_2.6.orig.tar.gz \
./yuma123_2.6/netconf/yuma123-perl_1.0-1.* \
./yuma123_2.6/netconf/yuma123-perl_1.0.orig.tar.gz \
./yuma123_2.6/netconf/yuma123-python_1.0-1.* \
./yuma123_2.6/netconf/yuma123-python_1.0.orig.tar.gz \
./yuma123_2.6/example-modules/yuma123-netconfd-module-ietf-system_1.0-1.* \
./yuma123_2.6/example-modules/yuma123-netconfd-module-ietf-system_1.0.orig.tar.gz \
./yuma123_2.6/example-modules/yuma123-netconfd-module-ietf-interfaces_1.0-1.* \
./yuma123_2.6/example-modules/yuma123-netconfd-module-ietf-interfaces_1.0.orig.tar.gz \
yuma123_2.6_release
