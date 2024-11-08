netconfmodule_LTLIBRARIES = libaether-packet-xcvr.la

libaether_packet_xcvr_la_SOURCES = aether-packet-xcvr@2024-10-31.c

if STANDALONE
libaether_packet_xcvr_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
libaether_packet_xcvr_la_LDFLAGS = -module -lyumaagt -lyumancx
else
libaether_packet_xcvr_la_CPPFLAGS = -I $(top_srcdir)/netconf/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform
libaether_packet_xcvr_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la
endif

dist_examples_yang_DATA = aether-packet-xcvr.yang

include_HEADERS= \
aether-packet-xcvr.h
