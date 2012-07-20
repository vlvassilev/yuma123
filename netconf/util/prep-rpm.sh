#!/bin/sh
#
# prep-rpm.sh [major version number]
#
# make the yuma build code and tar it up for rpmbuild
#
# 1 package:
#   yuma-version.rpm
#
# the $1 parameter must be '1' or '2'

if [ $# != 1 ]; then
  echo "Usage: prep-rpm.sh <major-version>"
  echo "Example:   prep-rpm.sh 1"
  exit 1
fi

if [ $1 = 1 ]; then
VER="1.15"
URL=https://yuma.svn.sourceforge.net/svnroot/yuma/branches/v1
SPEC=yuma.spec
elif [ $1 = 2 ]; then
VER="2.2"
URL=https://yuma.svn.sourceforge.net/svnroot/yuma/branches/v2
SPEC=yuma2.spec
elif [ $1 = 3 ]; then
VER="2.2"
URL=https://yuma.svn.sourceforge.net/svnroot/yuma/trunk
SPEC=yuma2.spec
else
  echo "Error: major version must be 1 or 2"
  echo "Usage: prep-rpm.sh <major-version>"
  echo "Example:   prep-rpm.sh 2"
  exit 1
fi

mkdir -p ~/rpmprep
rm -rf ~/rpmprep/*

cd ~/rpmprep

svn export $URL yuma-$VER

touch yuma-$VER/configure
chmod 775 yuma-$VER/configure

tar cvf yuma-$VER.tar yuma-$VER/
gzip yuma-$VER.tar

mkdir -p ~/rpmbuild/SPECS
mkdir -p ~/rpmbuild/SOURCES
mkdir -p ~/rpmbuild/SRPMS
mkdir -p ~/rpmbuild/RPMS

cp yuma-$VER.tar.gz ~/rpmbuild/SOURCES
cp yuma-$VER/SPECS/$SPEC ~/rpmbuild/SPECS
