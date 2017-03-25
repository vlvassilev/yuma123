TESTS=run.sh

netconfmodule_LTLIBRARIES = libtest-rollback-on-error.la

libtest_rollback_on_error_la_SOURCES = rollback-on-error.c

libtest_rollback_on_error_la_CPPFLAGS = -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
libtest_rollback_on_error_la_LDFLAGS = -module -lyumaagt -lyumancx
