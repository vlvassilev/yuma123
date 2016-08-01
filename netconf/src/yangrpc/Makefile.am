lib_LTLIBRARIES = libyangrpc.la

libyangrpc_la_SOURCES = \
$(top_srcdir)/netconf/src/yangrpc/yangrpc.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_autoload.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_cmd.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_util.c\
$(top_srcdir)/netconf/src/yangcli/yangcli_autolock.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_cond.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_eval.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_list.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_save.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_show.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_tab.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_timer.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_alias.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_uservars.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_wordexp.c \
$(top_srcdir)/netconf/src/yangcli/yangcli.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_globals.c


libyangrpc_la_CPPFLAGS = -DSKIP_MAIN -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml -I$(top_srcdir)/netconf/src/yangrpc -I$(top_srcdir)/netconf/src/yangcli -I$(top_srcdir)/libtecla
libyangrpc_la_LDFLAGS = -version-info 2:0:0 $(top_builddir)/libtecla/libtecla.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses
