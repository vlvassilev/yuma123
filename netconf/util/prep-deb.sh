#!/bin/sh
#
# prep-deb.sh [major version number]
#
# make the yuma build code and tar it up for debian
#
# the $1 parameter must be '1' or '2'

if [ $# != 1 ]; then
  echo "Usage: prep-deb.sh <major-version>"
  echo "Example:   prep-deb.sh 1"
  exit 1
fi

if [ $1 = 1 ]; then
VER="1.15"
URL=https://yuma.svn.sourceforge.net/svnroot/yuma/branches/v1
elif [ $1 = 2 ]; then
VER="2.1"
URL=https://yuma.svn.sourceforge.net/svnroot/yuma/trunk
else
  echo "Error: major version must be 1 or 2"
  echo "Usage: prep-deb.sh <major-version>"
  echo "Example:   prep-deb.sh 2"
  exit 1
fi

mkdir -p ~/build
mkdir -p ~/rpmprep
rm -rf ~/rpmprep/*

cd ~/rpmprep

svn export $URL yuma-$VER

tar cvf yuma_$VER.tar yuma-$VER/
gzip yuma_$VER.tar
cp yuma_$VER.tar.gz ~/build

cd ~/build
if [ ! -f yuma_$VER.orig.tar.gz ]; then
  cp yuma_$VER.tar.gz yuma_$VER.orig.tar.gz
fi

tar xvf yuma_$VER.tar.gz





