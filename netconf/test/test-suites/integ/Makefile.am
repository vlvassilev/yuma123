TESTS=test_yangcli_wordex
check_PROGRAMS=test_yangcli_wordex

test_yangcli_wordex_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/integ/test_yangcli_wordex.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_wordexp.c

test_yangcli_wordex_CPPFLAGS = -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_yangcli_wordex_LDFLAGS = $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses


TESTS+=base-64-tests
check_PROGRAMS+=base-64-tests

base_64_tests_SOURCES = \
$(top_srcdir)/netconf/test/integ-tests/simple-yang.cpp \
$(top_srcdir)/netconf/test/support/fixtures/test-context.cpp \
$(top_srcdir)/netconf/test/support/fixtures/abstract-global-fixture.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/yuma-op-policies.cpp \
$(top_srcdir)/netconf/test/support/callbacks/integ-cb-checker-factory.cpp \
$(top_srcdir)/netconf/test/support/fixtures/integ-fixture-helper-factory.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-query-utils.cpp \
$(top_srcdir)/netconf/test/support/nc-session/spoof-nc-session.cpp \
$(top_srcdir)/netconf/test/support/nc-session/abstract-nc-session.cpp \
$(top_srcdir)/netconf/test/support/callbacks/callback-checker.cpp \
$(top_srcdir)/netconf/test/support/nc-session/spoof-nc-session-factory.cpp \
$(top_srcdir)/netconf/test/support/callbacks/running-cb-checker.cpp \
$(top_srcdir)/netconf/test/support/callbacks/candidate-cb-checker.cpp \
$(top_srcdir)/netconf/test/support/callbacks/sil-callback-log.cpp \
$(top_srcdir)/netconf/test/support/callbacks/sil-callback-controller.cpp \
$(top_srcdir)/netconf/test/support/fixtures/integ-fixture-helper.cpp \
$(top_srcdir)/netconf/test/support/misc-util/base64.cpp \
$(top_srcdir)/netconf/test/support/misc-util/log-utils.cpp \
$(top_srcdir)/netconf/test/support/fixtures/base-suite-fixture.cpp \
$(top_srcdir)/netconf/test/test-suites/integ/base-64-tests.cpp

base_64_tests_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
base_64_tests_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

TESTS+=test_boost_unit_test_framework
check_PROGRAMS+=test_boost_unit_test_framework

test_boost_unit_test_framework_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/integ/test_boost_unit_test_framework.cpp
##-DBOOST_TEST_DYN_LINK
test_boost_unit_test_framework_CPPFLAGS = -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_boost_unit_test_framework_LDFLAGS = /usr/lib/libboost_unit_test_framework.a $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

