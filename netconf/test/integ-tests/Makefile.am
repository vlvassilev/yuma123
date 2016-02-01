TESTS=test_yangcli_wordex
check_PROGRAMS=test_yangcli_wordex
test_yangcli_wordex_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/integ/test_yangcli_wordex.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_wordexp.c
test_yangcli_wordex_CPPFLAGS = -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_yangcli_wordex_LDFLAGS = $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses


TESTS+=test-base-64
check_PROGRAMS+=test-base-64
test_base_64_SOURCES = \
$(top_srcdir)/netconf/test/integ-tests/simple-yang.cpp \
$(top_srcdir)/netconf/test/test-suites/integ/base-64-tests.cpp
test_base_64_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_base_64_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

TESTS+=test-device-get
check_PROGRAMS+=test-device-get
test_device_get_SOURCES = \
$(top_srcdir)/netconf/test/integ-tests/device-edit-candidate.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-get.cpp
test_device_get_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_device_get_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

TESTS+=test-simple-edit-running
check_PROGRAMS+=test-simple-edit-running
test_simple_edit_running_SOURCES = \
$(top_srcdir)/netconf/test/integ-tests/simple-edit-running.cpp \
$(top_srcdir)/netconf/test/test-suites/common/simple-edit-tests.cpp
test_simple_edit_running_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_simple_edit_running_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses


