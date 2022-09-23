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
$(top_srcdir)/netconf/src/ydump/yangyin.c \
$(top_srcdir)/netconf/src/ydump/yangtree.c

noinst_HEADERS= \
$(top_srcdir)/netconf/src/ydump/c.h \
$(top_srcdir)/netconf/src/ydump/c_util.h \
$(top_srcdir)/netconf/src/ydump/cyang.h \
$(top_srcdir)/netconf/src/ydump/h.h \
$(top_srcdir)/netconf/src/ydump/html.h \
$(top_srcdir)/netconf/src/ydump/py_util.h \
$(top_srcdir)/netconf/src/ydump/sql.h \
$(top_srcdir)/netconf/src/ydump/tg2.h \
$(top_srcdir)/netconf/src/ydump/xsd.h \
$(top_srcdir)/netconf/src/ydump/xsd_typ.h \
$(top_srcdir)/netconf/src/ydump/xsd_util.h \
$(top_srcdir)/netconf/src/ydump/xsd_yang.h \
$(top_srcdir)/netconf/src/ydump/yangdump.h \
$(top_srcdir)/netconf/src/ydump/yangdump_util.h \
$(top_srcdir)/netconf/src/ydump/yangstats.h \
$(top_srcdir)/netconf/src/ydump/yangtree.h \
$(top_srcdir)/netconf/src/ydump/yangyin.h \
$(top_srcdir)/netconf/src/ydump/ydump.h

yangdump_CPPFLAGS = -I $(top_srcdir)/libtecla -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I$(top_srcdir)/netconf/src/yangrpc -I$(top_srcdir)/netconf/src/ydump
yangdump_LDFLAGS = $(top_builddir)/netconf/src/ncx/libyumancx.la $(LIBS)

dist_yuma123_yang_DATA = \
$(top_srcdir)/netconf/modules/yuma123/yangdump.yang
