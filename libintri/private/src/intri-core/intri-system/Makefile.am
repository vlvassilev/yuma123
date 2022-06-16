netconfmodule_LTLIBRARIES = libintri-system.la

noinst_HEADERS=\
$(top_srcdir)/libintri/private/src/intri-core/intri-system/intri-system.h

libintri_system_la_SOURCES = \
$(top_srcdir)/libintri/private/src/intri-core/intri-system/intri-system@2022-06-14.c

libintri_system_la_CPPFLAGS = \
-I$(top_srcdir)/netconf/src/agt \
-I$(top_srcdir)/netconf/src/mgr \
-I$(top_srcdir)/netconf/src/ncx \
-I$(top_srcdir)/netconf/src/platform \
-I$(top_srcdir)/netconf/src/ydump \
-I$(top_srcdir)/libintri/private/intri-core/intri-system \
$(XML_CPPFLAGS)

libintri_system_la_LDFLAGS = \
-module \
$(top_builddir)/netconf/src/agt/libyumaagt.la \
$(top_builddir)/netconf/src/ncx/libyumancx.la

libintri_system_la_LIBADD = \
-L$(top_builddir)/libintri/.libintrishare/ -lintrishare

dist_intri_private_yang_DATA = \
$(top_srcdir)/libintri/private/src/intri-core/intri-system/intri-system.yang