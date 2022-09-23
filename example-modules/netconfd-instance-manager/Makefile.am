netconfmodule_LTLIBRARIES = libnetconfd-instance-manager.la

libnetconfd_instance_manager_la_SOURCES = \
netconfd-instance-manager.c

libnetconfd_instance_manager_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
libnetconfd_instance_manager_la_LDFLAGS = -module -lyumaagt -lyumancx

dist_yuma123_yang_DATA = yuma123-services.yang \
yuma123-services-netconfd.yang

