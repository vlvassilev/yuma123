#Tests
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
$(top_srcdir)/netconf/test/support/fixtures/simple-container-module-fixture.cpp \
$(top_srcdir)/netconf/test/support/checkers/string-presence-checkers.cpp \
$(top_srcdir)/netconf/test/support/msg-util/NCMessageBuilder.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-query-test-engine.cpp \
$(top_srcdir)/netconf/test/support/msg-util/xpo-query-builder.cpp \
$(top_srcdir)/netconf/test/support/db-models/device-test-db.cpp \
$(top_srcdir)/netconf/test/support/misc-util/ptree-utils.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-base-query-test-engine.cpp \
$(top_srcdir)/netconf/test/support/misc-util/base64.cpp \
$(top_srcdir)/netconf/test/support/misc-util/log-utils.cpp \
$(top_srcdir)/netconf/test/support/fixtures/base-suite-fixture.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-strings.cpp \
$(top_srcdir)/netconf/test/support/callbacks/system-cb-checker.cpp \
$(top_srcdir)/netconf/test/support/fixtures/system-fixture-helper.cpp \
$(top_srcdir)/netconf/test/support/fixtures/system-fixture-helper-factory.cpp \
$(top_srcdir)/netconf/test/support/callbacks/system-cb-checker-factory.cpp \
$(top_srcdir)/netconf/test/support/nc-session/sys-test-nc-session-base.cpp \
$(top_srcdir)/netconf/test/support/nc-session/remote-nc-session.cpp \
$(top_srcdir)/netconf/test/support/nc-session/remote-nc-session-factory.cpp

libyumatest_la_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
libyumatest_la_LDFLAGS=-static

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


