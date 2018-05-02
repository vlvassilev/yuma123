netconfmodule_LTLIBRARIES = libietf-alarms.la

libietf_alarms_la_SOURCES = \
ietf-alarms.c

libietf_alarms_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
libietf_alarms_la_LDFLAGS = -module -lyumaagt -lyumancx $(XML_LIBS) -lz -ldl

#ietf_draft_yang_DATA = \
#ietf-alarms@2016-10-27.yang \
#ietf-alarms-x733@2016-10-05.yang

#public alarm control library
lib_LTLIBRARIES = libalarmctrl.la
libalarmctrl_la_SOURCES = \
alarmctrl.c

libalarmctrl_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
libalarmctrl_la_LDFLAGS = -module -lyumaagt -lyumancx

include_HEADERS= \
alarmctrl.h
