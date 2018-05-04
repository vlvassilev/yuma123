netconfmodule_LTLIBRARIES = libietf-interfaces.la

libietf_interfaces_la_SOURCES = \
ietf-interfaces.c

if STANDALONE
  # Standalone mode. Depends on installed libyuma-dev
  libietf_interfaces_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
  libietf_interfaces_la_LDFLAGS = -module -lyumaagt -lyumancx
else
  # Integrated yuma123 build
  libietf_interfaces_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump $(XML_CPPFLAGS)
  libietf_interfaces_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la
endif

yang_DATA = \
interfaces-notifications.yang

