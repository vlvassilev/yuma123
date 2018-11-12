netconfmodule_LTLIBRARIES = libhelloworld.la

libhelloworld_la_SOURCES = helloworld.c

if STANDALONE
libhelloworld_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform $(XML_CPPFLAGS)
libhelloworld_la_LDFLAGS = -module -lyumaagt -lyumancx
else
libhelloworld_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform $(XML_CPPFLAGS)
libhelloworld_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la
endif

examples_yang_DATA = helloworld.yang
