netconfmodule_LTLIBRARIES = libtest-multiple-edit-callbacks-a.la

libtest_multiple_edit_callbacks_a_la_SOURCES = multiple-edit-callbacks.c

libtest_multiple_edit_callbacks_a_la_CPPFLAGS = -DINSTANCE_A -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
libtest_multiple_edit_callbacks_a_la_LDFLAGS = -module -lyumaagt -lyumancx

netconfmodule_LTLIBRARIES += libtest-multiple-edit-callbacks-b.la

libtest_multiple_edit_callbacks_b_la_SOURCES = multiple-edit-callbacks.c

libtest_multiple_edit_callbacks_b_la_CPPFLAGS = -DINSTANCE_B -I${includedir}/yuma/agt -I${includedir}/yuma/mgr -I${includedir}/yuma/ncx -I${includedir}/yuma/platform
libtest_multiple_edit_callbacks_b_la_LDFLAGS = -module -lyumaagt -lyumancx


yang_DATA = test-multiple-edit-callbacks.yang
