netconfmodule_LTLIBRARIES = libietf-network-bridge.la

libietf_network_bridge_la_SOURCES = \
ietf-network-bridge.c

if STANDALONE
  # Standalone mode. Depends on installed libyuma-dev
  libietf_network_bridge_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
  libietf_network_bridge_la_LDFLAGS = -module -lyumaagt -lyumancx
else
  # Integrated yuma123 build
  libietf_network_bridge_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform
  libietf_network_bridge_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la
endif

netconfmodule_LTLIBRARIES += libietf-network-bridge-openflow.la
libietf_network_bridge_openflow_la_SOURCES = \
ietf-network-bridge-openflow.c

if STANDALONE
  # Standalone mode. Depends on installed libyuma-dev
  libietf_network_bridge_openflow_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform -I${includedir}/openvswitch
  libietf_network_bridge_openflow_la_LDFLAGS = -module -lyumaagt -lyumancx -lopenvswitch
else
  # Integrated yuma123 build
  libietf_network_bridge_openflow_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I${includedir}/openvswitch
  libietf_network_bridge_openflow_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lopenvswitch
endif

dist_examples_yang_DATA = example-bridge.yang

