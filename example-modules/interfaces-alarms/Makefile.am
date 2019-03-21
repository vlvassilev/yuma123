netconfmodule_LTLIBRARIES = libinterfaces-alarms.la
libinterfaces_alarms_la_SOURCES = \
interfaces-alarms.c

libinterfaces_alarms_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
libinterfaces_alarms_la_LDFLAGS = -module -lyumaagt -lyumancx -lalarmctrl

#yang_DATA = ietf-interfaces.yang
dist_yang_DATA = \
interfaces-alarms.yang

