netconfmodule_LTLIBRARIES = libietf-system.la

libietf_system_la_SOURCES = \
$(top_srcdir)/libstandard/ietf-system/ietf-system.c

# Integrated yuma123 build
libietf_system_la_CPPFLAGS = \
-I $(top_srcdir)/netconf/src/ \
-I$(top_srcdir)/netconf/src/agt \
-I$(top_srcdir)/netconf/src/ncx \
-I$(top_srcdir)/netconf/src/platform \
-I$(top_srcdir)/netconf/src/ydump \
$(XML_CPPFLAGS)

libietf_system_la_LDFLAGS = \
-module \
$(top_builddir)/netconf/src/agt/libyumaagt.la \
$(top_builddir)/netconf/src/ncx/libyumancx.la

#yang_DATA = ietf-system.yang
