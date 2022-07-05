netconfmodule_LTLIBRARIES = libietf-ip.la

libietf_ip_la_SOURCES = \
ietf-ip.c


libietf_ip_la_CPPFLAGS = \
-I$(top_srcdir)/netconf/src/ \
-I$(top_srcdir)/netconf/src/agt \
-I$(top_srcdir)/netconf/src/ncx \
-I$(top_srcdir)/netconf/src/platform \
$(XML_CPPFLAGS)

libietf_ip_la_LDFLAGS = \
-module \
$(top_builddir)/netconf/src/agt/libyumaagt.la \
$(top_builddir)/netconf/src/ncx/libyumancx.la

# libietf_ip_la_LIBADD = \
# -L$(top_srcdir)/libintri/.libintrishare/ -lintrishare

dist_examples_yang_DATA = \
interfaces-notifications.yang
