netconfmodule_LTLIBRARIES = libtoaster.la

noinst_HEADERS=\
$(top_srcdir)/libtoaster/src/toaster.h

libtoaster_la_SOURCES = \
$(top_srcdir)/libtoaster/src/toaster.c

libtoaster_la_CPPFLAGS = -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump $(XML_CPPFLAGS)

libtoaster_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la

