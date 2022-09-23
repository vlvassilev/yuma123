python_LTLIBRARIES = yuma.la
yuma_la_SOURCES = yuma.c
yuma_la_CPPFLAGS = -I/usr/include/python3.9 -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
yuma_la_LDFLAGS = -module -avoid-version -lyumancx -lyumaagt

python_LTLIBRARIES += yangrpc.la
yangrpc_la_SOURCES = yangrpc.c
yangrpc_la_CPPFLAGS = -I/usr/include/python3.9 -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform -I${includedir}/yuma/yangrpc
yangrpc_la_LDFLAGS = -module -avoid-version -lyumancx -lyangrpc

bin_SCRIPTS=yangcli-python
