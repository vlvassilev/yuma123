netconfmodule_LTLIBRARIES = libinterfaces-alarms.la
libinterfaces_alarms_la_SOURCES = \
interfaces-alarms.c

libinterfaces_alarms_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
libinterfaces_alarms_la_LDFLAGS = -module -lyumaagt -lyumancx -lxml2 -lz -ldl -lalarmctrl

#yang_DATA = ietf-interfaces.yang
yang_DATA = \
interfaces-alarms.yang

