netconfmodule_LTLIBRARIES = libintri-private.la

noinst_HEADERS= \
$(top_srcdir)/libintri/private/intri-device.h

libintri_private_la_SOURCES = \
$(top_srcdir)/libintri/private/intri-device.c

libintri_private_la_CPPFLAGS = \
-I$(top_srcdir)/netconf/src/agt \
-I$(top_srcdir)/netconf/src/mgr \
-I$(top_srcdir)/netconf/src/ncx \
-I$(top_srcdir)/netconf/src/platform \
-I$(top_srcdir)/netconf/src/ydump \
-I$(top_srcdir)/libintri/private \
$(XML_CPPFLAGS)

libintri_private_la_LDFLAGS = \
-module \
$(top_builddir)/netconf/src/agt/libyumaagt.la \
$(top_builddir)/netconf/src/ncx/libyumancx.la \
-L$(top_srcdir)/libintri/.libintrishare/ -lintrishare


# libintri_private_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)

# libintri_private_la_LDFLAGS = -module -lyumaagt -lyumancx -L$(top_srcdir)/libintri/.libintrishare/ -lintrishare


dist_intri_private_yang_DATA = \
$(top_srcdir)/libintri/private/intri-device.yang

