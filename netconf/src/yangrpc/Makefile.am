lib_LTLIBRARIES = libyangrpc.la

libyangrpc_la_SOURCES = \
$(top_srcdir)/netconf/src/yangrpc/yangrpc.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_autoload.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_util.c


libyangrpc_la_CPPFLAGS = -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml -I$(top_srcdir)/netconf/src/yangrpc -I$(top_srcdir)/netconf/src/yangcli -I$(top_srcdir)/libtecla
libyangrpc_la_LDFLAGS = $(top_builddir)/libtecla/libtecla.la $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses
