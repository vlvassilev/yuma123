TESTS=test_yangcli_wordex
check_PROGRAMS=test_yangcli_wordex
test_yangcli_wordex_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/integ/test_yangcli_wordex.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_wordexp.c
test_yangcli_wordex_CPPFLAGS = -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_yangcli_wordex_LDFLAGS = $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-base-64
check_PROGRAMS+=test-base-64
test_base_64_SOURCES = \
$(top_srcdir)/netconf/test/integ-tests/simple-yang.cpp \
$(top_srcdir)/netconf/test/test-suites/integ/base-64-tests.cpp
test_base_64_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_base_64_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-default-none-running
check_PROGRAMS+=test-default-none-running
test_default_none_running_SOURCES= \
$(top_srcdir)/netconf/test/test-suites/common/default-none-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/default-none-running.cpp
test_default_none_running_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_default_none_running_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-device-edit-candidate
check_PROGRAMS+=test-device-edit-candidate
test_device_edit_candidate_SOURCES= \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-misc.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-create.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-merge.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-replace.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-get.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-delete.cpp \
$(top_srcdir)/netconf/test/test-suites/integ/base-64-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/device-edit-candidate.cpp
test_device_edit_candidate_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_device_edit_candidate_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses


TESTS+=test-device-edit-running
check_PROGRAMS+=test-device-edit-running
test_device_edit_running_SOURCES= \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-misc.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-create.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-merge.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-replace.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-get.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-delete.cpp \
$(top_srcdir)/netconf/test/test-suites/integ/base-64-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/device-edit-running.cpp
test_device_edit_running_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_device_edit_running_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-discard-changes
check_PROGRAMS+=test-discard-changes
test_discard_changes_SOURCES= \
$(top_srcdir)/netconf/test/test-suites/common/discard-changes-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/discard-changes.cpp
test_discard_changes_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_discard_changes_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-lock-load-candidate
check_PROGRAMS+=test-lock-load-candidate
test_lock_load_candidate_SOURCES= \
$(top_srcdir)/netconf/test/test-suites/common/module-load-test-suite.cpp \
$(top_srcdir)/netconf/test/test-suites/common/db-lock-test-suite-common.cpp \
$(top_srcdir)/netconf/test/test-suites/common/db-lock-test-suite-candidate.cpp \
$(top_srcdir)/netconf/test/integ-tests/lock-load-candidate.cpp
test_lock_load_candidate_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_lock_load_candidate_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-lock-load-running
check_PROGRAMS+=test-lock-load-running
test_lock_load_running_SOURCES= \
$(top_srcdir)/netconf/test/test-suites/common/module-load-test-suite.cpp \
$(top_srcdir)/netconf/test/test-suites/common/db-lock-test-suite-common.cpp \
$(top_srcdir)/netconf/test/test-suites/common/db-lock-test-suite-running.cpp \
$(top_srcdir)/netconf/test/integ-tests/lock-load-running.cpp
test_lock_load_running_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_lock_load_running_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-simple-edit-candidate
check_PROGRAMS+=test-simple-edit-candidate
test_simple_edit_candidate_SOURCES= \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests-candidate.cpp \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests-validate.cpp \
$(top_srcdir)/netconf/test/test-suites/common/discard-changes-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/common/default-none-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/common/callback-failures-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/simple-edit-candidate.cpp
test_simple_edit_candidate_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_simple_edit_candidate_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-simple-edit-running
check_PROGRAMS+=test-simple-edit-running
test_simple_edit_running_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/common/default-none-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/simple-edit-running.cpp
test_simple_edit_running_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_simple_edit_running_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-simple-edit-startup-false
check_PROGRAMS+=test-simple-edit-startup-false
test_simple_edit_startup_false_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests-startup.cpp \
$(top_srcdir)/netconf/test/integ-tests/simple-edit-startup-false.cpp
test_simple_edit_startup_false_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_simple_edit_startup_false_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-simple-edit-startup-true
check_PROGRAMS+=test-simple-edit-startup-true
test_simple_edit_startup_true_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests-startup.cpp \
$(top_srcdir)/netconf/test/test-suites/common/startup-delete-config-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/simple-edit-startup-true.cpp
test_simple_edit_startup_true_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_simple_edit_startup_true_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-simple-yang
check_PROGRAMS+=test-simple-yang
test_simple_yang_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/simple-choice-tests.cpp \
$(top_srcdir)/netconf/test/test-suites/common/simple-must-tests.cpp \
$(top_srcdir)/netconf/test/integ-tests/simple-yang.cpp
test_simple_yang_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_simple_yang_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-state-edit-candidate
check_PROGRAMS+=test-state-edit-candidate
test_state_edit_candidate_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/state-data-tests-get.cpp \
$(top_srcdir)/netconf/test/integ-tests/state-edit-candidate.cpp
test_state_edit_candidate_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_state_edit_candidate_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

TESTS+=test-state-edit-running
check_PROGRAMS+=test-state-edit-running
test_state_edit_running_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/common/state-data-tests-get.cpp \
$(top_srcdir)/netconf/test/integ-tests/state-edit-running.cpp
test_state_edit_running_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I${includedir}/libxml2 -I${includedir}/libxml2/libxml
test_state_edit_running_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lxml2 -lz  -ldl -lssh2 -lncurses

