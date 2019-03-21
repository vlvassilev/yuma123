netconfmodule_LTLIBRARIES = libyangcli-to-rpc.la

libyangcli_to_rpc_la_SOURCES = \
yangcli-to-rpc.c

if STANDALONE
  # Standalone mode. Depends on installed libyuma-dev
  libyangcli_to_rpc_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
  libyangcli_to_rpc_la_LDFLAGS = -module -lyumaagt -lyumancx
else
  # Integrated yuma123 build
  libyangcli_to_rpc_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform $(XML_CPPFLAGS)
  libyangcli_to_rpc_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la
endif

dist_yuma123_yang_DATA = yuma123-yangcli-to-rpc.yang
