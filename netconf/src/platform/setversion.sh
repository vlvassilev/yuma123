#!/bin/sh
#
# get the subversion build number

echo "#define SVNVERSION \"`svnversion`\"" > platform/curversion.h



