bin_PROGRAMS = yangrpc-example
yangrpc_example_SOURCES = \
$(top_srcdir)/netconf/src/yangrpc/example/yangrpc-example.c

yangrpc_example_CPPFLAGS = -I $(top_srcdir)/libtecla -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml -I$(top_srcdir)/netconf/src/yangrpc
yangrpc_example_LDFLAGS = $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la $(top_builddir)/netconf/src/yangrpc/libyangrpc.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses
