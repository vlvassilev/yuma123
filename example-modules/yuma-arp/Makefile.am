netconfmodule_LTLIBRARIES = libyuma-arp.la

libyuma_arp_la_SOURCES = \
yuma-arp.c

libyuma_arp_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
libyuma_arp_la_LDFLAGS = -module -lyumaagt -lyumancx

dist_netconfcentral_yang_DATA = yuma-arp.yang
