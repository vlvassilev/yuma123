netconfmodule_LTLIBRARIES = libietf-system.la

libietf_system_la_SOURCES = \
ietf-system.c

#Integrated yuma123 build
libietf_system_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
libietf_system_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl

#Standalone mode. Depends on installed libyuma-dev
#libietf_system_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
#libietf_system_la_LDFLAGS = -module -lyumaagt -lyumancx -lxml2 -lz -ldl

#yang_DATA = ietf-system.yang
