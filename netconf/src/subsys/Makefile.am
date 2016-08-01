sbin_PROGRAMS = netconf-subsystem
netconf_subsystem_SOURCES = \
$(top_srcdir)/netconf/src/subsys/netconf-subsystem.c


netconf_subsystem_CPPFLAGS = -I $(top_srcdir)/netconf/src/subsys/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
netconf_subsystem_LDFLAGS = $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl

