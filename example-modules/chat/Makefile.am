netconfmodule_LTLIBRARIES = libchat.la

libchat_la_SOURCES = \
chat.c

libchat_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
libchat_la_LDFLAGS = -module -lyumaagt -lyumancx

dist_yang_DATA = chat.yang
