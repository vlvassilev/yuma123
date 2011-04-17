#!/bin/sh
#
# make-src-tarball.sh <version-string> <release-number>
#
# make the yuma source tarball
#
# the $1 parameter must be a major and minor version
# number, such as 1.14
# the $2 parameter must be the release number, such as 6

if [ $# != 2 ]; then
  echo "Usage: make-src-tarball <version-string> <release-number>"
  echo "version-string is the full version, such as 1.14 6"
  exit 1
fi

echo "Making yuma source tarball for $1-$2"
mkdir -p ~/srctarballprep
rm -rf ~/srctarballprep/*
cd ~/srctarballprep

svn export http://svn.netconfcentral.org/svn/yuma/trunk yuma-$1-$2
echo "echo \"#define RELEASE $2\" > platform/curversion.h" > \
    yuma-$1-$2/netconf/src/platform/setversion.sh
tar cvf yuma-$1-$2.tar yuma-$1-$2
gzip yuma-$1-$2.tar




