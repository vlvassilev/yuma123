python_LTLIBRARIES = yuma.la
yuma_la_SOURCES = yuma.c
yuma_la_CPPFLAGS = -I/usr/include/python2.7 -I$(includedir)/yuma/agt  -I$(includedir)/yuma/ncx -I$(includedir)/yuma/platform -I$(includedir)/libxml2 -I$(includedir)/libxml2/libxml
yuma_la_LDFLAGS = -module -avoid-version -lmgr -lagt -lncx -lxml2 -lz  -ldl -lssh2 -lcrypto -lssl -lncurses -lrt -lpthread

python_LTLIBRARIES += yangrpc.la
yangrpc_la_SOURCES = yangrpc.c
yangrpc_la_CPPFLAGS = -I/usr/include/python2.7 -I $(top_srcdir)/netconf/src/yangcli/ -I $(top_srcdir)/netconf/src/yangrpc/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(includedir)/libxml2 -I$(includedir)/libxml2/libxml
yangrpc_la_LDFLAGS = -module -avoid-version $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lcrypto -lssl -lncurses -lrt -lpthread -lyangrpc

