sbin_PROGRAMS = netconfd
netconfd_SOURCES = \
$(top_srcdir)/netconf/src/netconfd/netconfd.c


netconfd_CPPFLAGS = -I $(top_srcdir)/netconf/src/netconfd/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
netconfd_LDFLAGS = $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lm -lpthread

