#!/bin/sh -e

yangdump --format=c --modpath=/usr/share/yuma/modules/ --output=mod1.c mod1.yang
yangdump --format=h --modpath=/usr/share/yuma/modules/ --output=mod1.h mod1.yang
gcc -I/usr/include/yuma/platform -I/usr/include/yuma/agt -I/usr/include/yuma/ncx/ -I/usr/include -I/usr/include/libxml2 -I/usr/include/libxml2/libxml mod1.c -lyumancx -lyumaagt -c -o mod1.o
