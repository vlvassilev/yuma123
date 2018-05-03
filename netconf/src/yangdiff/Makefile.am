bin_PROGRAMS = yangdiff

yangdiff_SOURCES = \
$(top_srcdir)/netconf/src/yangdiff/yangdiff.c \
$(top_srcdir)/netconf/src/yangdiff/yangdiff_grp.c \
$(top_srcdir)/netconf/src/yangdiff/yangdiff_obj.c \
$(top_srcdir)/netconf/src/yangdiff/yangdiff_typ.c \
$(top_srcdir)/netconf/src/yangdiff/yangdiff_util.c

yangdiff_CPPFLAGS = -I $(top_srcdir)/libtecla -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump $(XML_CPPFLAGS) -I$(top_srcdir)/netconf/src/yangrpc -I$(top_srcdir)/netconf/src/ydump
yangdiff_LDFLAGS = $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la $(top_builddir)/netconf/src/yangrpc/libyangrpc.la $(XML_LIBS) -lz  $(LIBS) -lssh2 -lncurses

netconfcentral_yang_DATA = \
$(top_srcdir)/netconf/modules/netconfcentral/yangdiff.yang
