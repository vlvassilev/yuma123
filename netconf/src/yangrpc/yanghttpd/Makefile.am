apachemodule_LTLIBRARIES = mod_yanghttpd.la

mod_yanghttpd_la_SOURCES = \
$(top_srcdir)/netconf/src/yangrpc/yanghttpd/mod_yanghttpd.c

mod_yanghttpd_la_CPPFLAGS =  -I $(top_srcdir)/libtecla -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I$(includedir)/libxml2 -I$(includedir)/libxml2/libxml -I$(top_srcdir)/netconf/src/yangrpc -I$(includedir)/apache2 -I$(includedir)/apr-1.0
mod_yanghttpd_la_LDFLAGS = -shared -module -rdynamic $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la $(top_builddir)/netconf/src/yangrpc/libyangrpc.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses
