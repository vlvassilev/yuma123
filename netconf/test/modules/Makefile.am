yang_DATA = \
$(top_srcdir)/netconf/test/modules/yang/simple_yang_test.yang


netconfmodule_LTLIBRARIES = libsimple_yang_test.la

libsimple_yang_test_la_SOURCES = \
$(top_srcdir)/netconf/test/modules/build-sil/simple_yang_test/simple_yang_test.c

libsimple_yang_test_la_CPPFLAGS = -I $(top_srcdir)/netconf -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml

libsimple_yang_test_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl

