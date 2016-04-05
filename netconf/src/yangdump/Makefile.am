bin_PROGRAMS = yangdump
yangdump_SOURCES = \
$(top_srcdir)/netconf/src/yangdump/yangdump.c \
$(top_srcdir)/netconf/src/ydump/ydump.c \
$(top_srcdir)/netconf/src/ydump/c.c \
$(top_srcdir)/netconf/src/ydump/c_util.c \
$(top_srcdir)/netconf/src/ydump/cyang.c \
$(top_srcdir)/netconf/src/ydump/h.c \
$(top_srcdir)/netconf/src/ydump/html.c \
$(top_srcdir)/netconf/src/ydump/py_util.c \
$(top_srcdir)/netconf/src/ydump/sql.c \
$(top_srcdir)/netconf/src/ydump/tg2.c \
$(top_srcdir)/netconf/src/ydump/xsd.c \
$(top_srcdir)/netconf/src/ydump/xsd_typ.c \
$(top_srcdir)/netconf/src/ydump/xsd_util.c \
$(top_srcdir)/netconf/src/ydump/xsd_yang.c \
$(top_srcdir)/netconf/src/ydump/yangdump_util.c \
$(top_srcdir)/netconf/src/ydump/yangstats.c \
$(top_srcdir)/netconf/src/ydump/yangyin.c

yangdump_CPPFLAGS = -I $(top_srcdir)/libtecla -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml -I$(top_srcdir)/netconf/src/yangrpc -I$(top_srcdir)/netconf/src/ydump
yangdump_LDFLAGS = $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la $(top_builddir)/netconf/src/yangrpc/libyangrpc.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

netconfcentral_yang_DATA = \
$(top_srcdir)/netconf/modules/netconfcentral/yangdump.yang
