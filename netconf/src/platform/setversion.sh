#!/bin/sh
#
# get the subversion build bumber

echo "#define SVNVERSION \"`svnversion`\"" > platform/curversion.h



