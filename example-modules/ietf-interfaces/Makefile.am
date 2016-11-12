netconfmodule_LTLIBRARIES = libietf-interfaces.la

libietf_interfaces_la_SOURCES = \
ietf-interfaces.c

libietf_interfaces_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
libietf_interfaces_la_LDFLAGS = -module -lyumaagt -lyumancx -lxml2 -lz -ldl

#yang_DATA = ietf-interfaces.yang
yang_DATA = interfaces-notifications.yang
