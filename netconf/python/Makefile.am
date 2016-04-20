python_LTLIBRARIES = yuma.la

yuma_la_SOURCES = yuma.c
yuma_la_CPPFLAGS = -I/usr/include/python2.7 -I$(includedir)/yuma/agt  -I$(includedir)/yuma/ncx -I$(includedir)/yuma/platform -I$(includedir)/libxml2 -I$(includedir)/libxml2/libxml
yuma_la_LDFLAGS = -module -avoid-version -lmgr -lagt -lncx -lxml2 -lz  -ldl -lssh2 -lcrypto -lssl -lncurses -lrt -lpthread
