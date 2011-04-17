#!/bin/sh
#
# make-svn-src-tarball.sh
#
# make the debug yuma source tarball 
#
if [ $# != 0 ]; then
  echo "Usage: make-svn-src-tarball"
  echo "Creates a debug yuma source tarball"
  exit 1
fi

SVNVER=`svnversion`

echo "Making debug yuma source tarball for SVN revision $SVNVER"
mkdir -p ~/svntarballprep
rm -rf ~/svntarballprep/*
cd ~/svntarballprep

svn export http://svn.netconfcentral.org/svn/yuma/trunk yuma$SVNVER
echo "echo \"#define SVNVERSION \\\"$SVNVER\\\"\" > platform/curversion.h" > \
    yuma$SVNVER/netconf/src/platform/setversion.sh
tar cvf yuma$SVNVER.tar yuma$SVNVER
gzip yuma$SVNVER.tar





