sbin_PROGRAMS = netconfd
netconfd_SOURCES = \
$(top_srcdir)/netconf/src/netconfd/netconfd.c


netconfd_CPPFLAGS = -I $(top_srcdir)/netconf/src/netconfd/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump $(XML_CPPFLAGS)
netconfd_LDFLAGS = $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la

