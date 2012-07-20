lib_LTLIBRARIES = libmgr.la

libmgr_la_SOURCES = \
$(top_srcdir)/netconf/src/mgr/mgr.c \
$(top_srcdir)/netconf/src/mgr/mgr_cap.c \
$(top_srcdir)/netconf/src/mgr/mgr_hello.c \
$(top_srcdir)/netconf/src/mgr/mgr_io.c \
$(top_srcdir)/netconf/src/mgr/mgr_load.c \
$(top_srcdir)/netconf/src/mgr/mgr_not.c \
$(top_srcdir)/netconf/src/mgr/mgr_rpc.c \
$(top_srcdir)/netconf/src/mgr/mgr_ses.c \
$(top_srcdir)/netconf/src/mgr/mgr_signal.c \
$(top_srcdir)/netconf/src/mgr/mgr_top.c \
$(top_srcdir)/netconf/src/mgr/mgr_val_parse.c \
$(top_srcdir)/netconf/src/mgr/mgr_xml.c


libmgr_la_CPPFLAGS = -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
libmgr_la_LDFLAGS = -lxml2 -lz  -ldl

