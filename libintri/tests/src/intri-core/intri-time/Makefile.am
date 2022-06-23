netconfmodule_LTLIBRARIES = libintri-time.la

noinst_HEADERS=\
$(top_srcdir)/libintri/tests/src/intri-core/intri-time/intri-time.h

libintri_time_la_SOURCES = \
$(top_srcdir)/libintri/tests/src/intri-core/intri-time/intri-time@2022-06-14.c

libintri_time_la_CPPFLAGS = \
-I$(top_srcdir)/netconf/src/agt \
-I$(top_srcdir)/netconf/src/mgr \
-I$(top_srcdir)/netconf/src/ncx \
-I$(top_srcdir)/netconf/src/platform \
-I$(top_srcdir)/netconf/src/ydump \
-I$(top_srcdir)/libintri/private/intri-core/intri-time \
$(XML_CPPFLAGS)

libintri_time_la_LDFLAGS = \
-module \
$(top_builddir)/netconf/src/agt/libyumaagt.la \
$(top_builddir)/netconf/src/ncx/libyumancx.la

# libintri_time_la_LIBADD = \
# -L$(top_srcdir)/libintri/.libintrishare/ -lintrishare

dist_intri_private_yang_DATA = \
$(top_srcdir)/libintri/tests/src/intri-core/intri-time/intri-time.yang