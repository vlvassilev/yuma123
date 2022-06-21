netconfmodule_LTLIBRARIES = libintri-device.la

noinst_HEADERS=\
$(top_srcdir)/libintri/private/src/intri-device/intri-device.h

libintri_device_la_SOURCES = \
$(top_srcdir)/libintri/private/src/intri-device/intri-device@2022-06-14.c

libintri_device_la_CPPFLAGS = \
-I$(top_srcdir)/netconf/src/agt \
-I$(top_srcdir)/netconf/src/mgr \
-I$(top_srcdir)/netconf/src/ncx \
-I$(top_srcdir)/netconf/src/platform \
-I$(top_srcdir)/netconf/src/ydump \
-I$(top_srcdir)/libintri/private/intri-device \
$(XML_CPPFLAGS)

libintri_device_la_LDFLAGS = \
-module \
$(top_builddir)/netconf/src/agt/libyumaagt.la \
$(top_builddir)/netconf/src/ncx/libyumancx.la

# libintri_device_la_LIBADD = \
# -L$(top_srcdir)/libintri/.libintrishare/ -lintrishare

dist_intri_private_yang_DATA = \
$(top_srcdir)/libintri/private/src/intri-device/intri-device.yang