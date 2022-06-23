netconfmodule_LTLIBRARIES = libintri-vlan.la

noinst_HEADERS=\
$(top_srcdir)/libintri/private/src/intri-core/intri-vlan/intri-vlan-trans.h \
$(top_srcdir)/libintri/private/src/intri-core/intri-vlan/intri-vlan.h

libintri_vlan_la_SOURCES = \
$(top_srcdir)/libintri/private/src/intri-core/intri-vlan/intri-vlan-trans.c \
$(top_srcdir)/libintri/private/src/intri-core/intri-vlan/intri-vlan@2022-06-23.c

libintri_vlan_la_CPPFLAGS = \
-I$(top_srcdir)/netconf/src/agt \
-I$(top_srcdir)/netconf/src/mgr \
-I$(top_srcdir)/netconf/src/ncx \
-I$(top_srcdir)/netconf/src/platform \
-I$(top_srcdir)/netconf/src/ydump \
-I$(top_srcdir)/libintri/private/intri-core/intri-vlan \
$(XML_CPPFLAGS)

libintri_vlan_la_LDFLAGS = \
-module \
$(top_builddir)/netconf/src/agt/libyumaagt.la \
$(top_builddir)/netconf/src/ncx/libyumancx.la

# libintri_vlan_la_LIBADD = \
# -L$(top_srcdir)/libintri/.libintrishare/ -lintrishare
