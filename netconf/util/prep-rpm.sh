#!/bin/sh
#
# prep-deb.sh [svnversion]
#
# make the yuma build code and tar it up for rpmbuild
#
# 2 packages:
#   yuma-version.rpm
#   yuma-dev-version.rpm
#
# the $1 parameter must be a revision number if present
# this will be used instead of HEAD in the svn export step

VER="1.15"
mkdir -p ~/rpmprep
rm -rf ~/rpmprep/*

cd ~/rpmprep

if [ $1 ]; then
  svn export -r$1 https://yuma.svn.sourceforge.net/svnroot/yuma/branches/v1 yuma-$VER
else
  svn export https://yuma.svn.sourceforge.net/svnroot/yuma/branches/v1 yuma-$VER
fi

touch yuma-$VER/configure
chmod 775 yuma-$VER/configure

tar cvf yuma-$VER.tar yuma-$VER/
gzip yuma-$VER.tar
cp yuma-$VER.tar.gz ~/rpmbuild/SOURCES
cp yuma-$VER/SPECS/yuma.spec ~/rpmbuild/SPECS
