#!/bin/sh
#
# make-src-tarball.sh <major-version> <minor-version> <release-number>
#
# make the yuma source tarball
#
# the $1 parameter must be a major version number, such as 1
# the $2 parameter must be a minor version number, such as 15
# the $3 parameter must be the release number, such as 1

if [ $# != 3 ]; then
  echo "Usage: make-src-tarball <major-version> <minor-version> <release-number>"
  echo "Example:   make-src-tarball 1 15 1 for v1.15-1"
  exit 1
fi

echo "Making yuma source tarball for $1.$2-$3"
mkdir -p ~/srctarballprep
rm -rf ~/srctarballprep/*
cd ~/srctarballprep

if [ $1 = 1 ]; then
  svn export https://yuma.svn.sourceforge.net/svnroot/yuma/branches/v1 yuma-$1.$2-$3
elif [ $1 = 2 ]; then
  svn export https://yuma.svn.sourceforge.net/svnroot/yuma/trunk yuma-$1.$2-$3
else
  echo "Error: major version must be 1 or 2"
  echo "Usage: make-src-tarball <major-version> <minor-version> <release-number>"
  echo "Example:   make-src-tarball 1 15 1 for v1.15-1"
  exit 1
fi

echo "echo \"#define RELEASE $3\" > platform/curversion.h" > \
    yuma-$1.$2-$3/netconf/src/platform/setversion.sh
tar cvf yuma-$1.$2-$3.tar yuma-$1.$2-$3
gzip yuma-$1.$2-$3.tar





