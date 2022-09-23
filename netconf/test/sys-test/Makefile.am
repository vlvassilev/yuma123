TESTS=test-simple-edit-candidate
check_PROGRAMS=test-simple-edit-candidate
test_simple_edit_candidate_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests-candidate.cpp \
$(top_srcdir)/netconf/test/test-suites/common/discard-changes-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/common/default-none-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/system/simple-edit-tests-confirmed-commit.cpp \
$(top_srcdir)/netconf/test/sys-test/simple-edit-candidate.cpp
test_simple_edit_candidate_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
test_simple_edit_candidate_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS) -lssh2 -lncurses

TESTS+=test-device-edit-candidate
check_PROGRAMS+=test-device-edit-candidate
test_device_edit_candidate_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-misc.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-create.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-merge.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-replace.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-get.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-delete.cpp \
$(top_srcdir)/netconf/test/sys-test/device-edit-candidate.cpp
test_device_edit_candidate_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
test_device_edit_candidate_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS) -lssh2 -lncurses

TESTS+=test-lock-load-candidate
check_PROGRAMS+=test-lock-load-candidate
test_lock_load_candidate_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/module-load-test-suite.cpp \
$(top_srcdir)/netconf/test/test-suites/common/db-lock-test-suite-common.cpp \
$(top_srcdir)/netconf/test/test-suites/common/db-lock-test-suite-candidate.cpp \
$(top_srcdir)/netconf/test/test-suites/system//my-session-tests.cpp \
$(top_srcdir)/netconf/test/sys-test/lock-load-candidate.cpp
test_lock_load_candidate_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
test_lock_load_candidate_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS) -lssh2 -lncurses

TESTS+=test-shutdown
check_PROGRAMS+=test-shutdown
test_shutdown_SOURCES = \
$(top_srcdir)/netconf/test/sys-test/shutdown.cpp \
$(top_srcdir)/netconf/test/test-suites/system/shutdown-tests.cpp
test_shutdown_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
test_shutdown_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS) -lssh2 -lncurses
