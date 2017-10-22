#!/bin/sh -e
rm -rf tmp || true
mkdir tmp
cd tmp
wget http://www.claise.be/YANG-RFC.tar
tar -C ../../../../modules/ietf/ -xvf YANG-RFC.tar
wget http://www.claise.be/YANG-drafts.tar
tar -C ../../../../modules/ietf-draft/ -xvf YANG-drafts.tar

