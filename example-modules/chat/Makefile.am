netconfmodule_LTLIBRARIES = libchat.la

libchat_la_SOURCES = \
chat.c

libchat_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
libchat_la_LDFLAGS = -module -lyumaagt -lyumancx -lxml2 -lz -ldl

yang_DATA = chat.yang
