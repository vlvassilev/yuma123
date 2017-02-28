#!/bin/bash -e

rm -r tmp || true
mkdir tmp
cd tmp
wget http://www.claise.be/YANG-RFC.tar
tar -xvf YANG-RFC.tar
rm YANG-RFC.tar

for file in `ls *.yang` ; do
	echo $file
	yangdump --module=$file --modpath=.
done
