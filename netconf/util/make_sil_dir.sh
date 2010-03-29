#!/bin/sh
#
# Generate a SIL directory structure
#
# P1 == the module name to use

if [ $# != 1 ]; then
  echo "Usage: make_sil_dir <module-name>"
  echo "Call from the parent directory where SIL dir should be created"
  exit 1
fi

if [ -f $1 ]; then
  echo "Error: File '$1' already exists"
  exit 1
fi

if [ -d $1 ]; then
  echo "Error: Directory '$1' already exists"
  exit 1
fi

mkdir $1

if [ ! -d $1 ]; then
  echo "Error: Make directory '$1' failed"
  exit 1
fi

mkdir $1/src
mkdir $1/bin
mkdir $1/lib

if [ ! -d $YUMA_HOME ]; then
  if [ ! -f /usr/share/yuma/util/makefile.sil ]; then
    echo "Error: /usr/share/yuma/util/makefile.sil not found"
    exit 1
  else
    echo "MODULE_NAME=$1" > $1/src/Makefile
    cat /usr/share/yuma/util/makefile.sil >> $1/src/Makefile
    cp /usr/share/yuma/util/makefile-top.sil $1/Makefile
  fi
else
  if [ ! -f $YUMA_HOME/util/makefile.sil ]; then
    echo "Error: $YUMA_HOME/util/makefile.sil not found"
    exit 1
  else 
    echo "MODULE_NAME=$1" > $1/src/Makefile
    cat $YUMA_HOME/util/makefile.sil >> $1/src/Makefile
    cp $YUMA_HOME/util/makefile-top.sil $1/Makefile
  fi
fi

cd $1/src
yangdumpcode format=h indent=4 module=$1 output=$1.h
yangdumpcode format=c indent=4 module=$1 output=$1.c
cd ../..




  
    
    



