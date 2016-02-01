#Tests
TESTS=test_yangcli_wordex
check_PROGRAMS=test_yangcli_wordex

test_yangcli_wordex_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/integ/test_yangcli_wordex.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_wordexp.c

test_yangcli_wordex_CPPFLAGS = -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_yangcli_wordex_LDFLAGS = $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

lib_LTLIBRARIES = libyumatest.la

libyumatest_la_SOURCES = \
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
$(top_srcdir)/netconf/test/support/fixtures/query-suite-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/device-module-common-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/device-get-module-fixture.cpp \
$(top_srcdir)/netconf/test/support/checkers/string-presence-checkers.cpp \
$(top_srcdir)/netconf/test/support/msg-util/NCMessageBuilder.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-query-test-engine.cpp \
$(top_srcdir)/netconf/test/support/msg-util/xpo-query-builder.cpp \
$(top_srcdir)/netconf/test/support/db-models/device-test-db.cpp \
$(top_srcdir)/netconf/test/support/misc-util/ptree-utils.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-base-query-test-engine.cpp \
$(top_srcdir)/netconf/test/support/misc-util/base64.cpp \
$(top_srcdir)/netconf/test/support/misc-util/log-utils.cpp \
$(top_srcdir)/netconf/test/support/fixtures/base-suite-fixture.cpp

libyumatest_la_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
libyumatest_la_LDFLAGS=-static

TESTS+=base-64-tests
check_PROGRAMS+=base-64-tests

base_64_tests_SOURCES = \
$(top_srcdir)/netconf/test/integ-tests/simple-yang.cpp \
$(top_srcdir)/netconf/test/test-suites/integ/base-64-tests.cpp

base_64_tests_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
base_64_tests_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/test-suites/integ/libyumatest.la $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

TESTS+=device-tests-get
check_PROGRAMS+=device-tests-get

#$(top_srcdir)/netconf/test/integ-tests/simple-yang.cpp

device_tests_get_SOURCES = \
$(top_srcdir)/netconf/test/integ-tests/device-edit-candidate.cpp \
$(top_srcdir)/netconf/test/test-suites/common/device-tests-get.cpp

device_tests_get_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
device_tests_get_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/test-suites/integ/libyumatest.la $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses

#Test modules - YANGs
yang_DATA = \
$(top_srcdir)/netconf/test/modules/yang/simple_yang_test.yang \
$(top_srcdir)/netconf/test/modules/yang/simple_list_test.yang \
$(top_srcdir)/netconf/test/modules/yang/device_test.yang

#Test modules - SILs
netconfmodule_LTLIBRARIES = libsimple_yang_test.la
libsimple_yang_test_la_SOURCES = \
$(top_srcdir)/netconf/test/modules/build-sil/simple_yang_test/simple_yang_test.cpp \
$(top_srcdir)/netconf/test/support/callbacks/sil-callback-log.cpp
libsimple_yang_test_la_CPPFLAGS = -I $(top_srcdir)/netconf -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
libsimple_yang_test_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl

netconfmodule_LTLIBRARIES+=libsimple_list_test.la
libsimple_list_test_la_SOURCES = \
$(top_srcdir)/netconf/test/modules/build-sil/simple_list_test/simple_list_test.cpp \
$(top_srcdir)/netconf/test/support/callbacks/sil-callback-log.cpp
libsimple_list_test_la_CPPFLAGS = -I $(top_srcdir)/netconf -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
libsimple_list_test_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl

netconfmodule_LTLIBRARIES+=libdevice_test.la
libdevice_test_la_SOURCES = \
$(top_srcdir)/netconf/test/modules/build-sil/device_test/device_test.cpp \
$(top_srcdir)/netconf/test/support/callbacks/sil-callback-log.cpp
libdevice_test_la_CPPFLAGS = -I $(top_srcdir)/netconf -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
libdevice_test_la_LDFLAGS = -module $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl


