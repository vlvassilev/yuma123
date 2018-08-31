apachemodulelib_LTLIBRARIES = mod_yangrpc_example.la

mod_yangrpc_example_la_SOURCES = \
$(top_srcdir)/mod_yangrpc_example.c

mod_yangrpc_example_la_CPPFLAGS = -I$(includedir) -I$(includedir)/yuma/agt -I$(includedir)/yuma/ncx -I$(includedir)/yuma/platform -I$(includedir)/yuma/yangrpc $(XML_CPPFLAGS) -I$(includedir)/apache2 -I$(includedir)/apr-1.0
mod_yangrpc_example_la_LDFLAGS = -module -lyumancx -lyangrpc $(XML_LIBS) $(LIBS)

apachemoduleconfig_DATA = \
yangrpc_example.load \
yangrpc_example.conf
