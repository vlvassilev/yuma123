#Tests
lib_LTLIBRARIES = libyumatest.la

libyumatest_la_SOURCES = \
$(top_srcdir)/netconf/test/support/nc-query-util/yuma-op-policies.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-default-operation-config.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-query-test-engine.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-strings.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-query-utils.cpp \
$(top_srcdir)/netconf/test/support/nc-query-util/nc-base-query-test-engine.cpp \
$(top_srcdir)/netconf/test/support/checkers/log-entry-presence-checkers.cpp \
$(top_srcdir)/netconf/test/support/checkers/checker-group.cpp \
$(top_srcdir)/netconf/test/support/checkers/string-presence-checkers.cpp \
$(top_srcdir)/netconf/test/support/callbacks/callback-checker.cpp \
$(top_srcdir)/netconf/test/support/callbacks/sil-callback-controller.cpp \
$(top_srcdir)/netconf/test/support/callbacks/system-cb-checker.cpp \
$(top_srcdir)/netconf/test/support/callbacks/sil-callback-log.cpp \
$(top_srcdir)/netconf/test/support/callbacks/candidate-cb-checker.cpp \
$(top_srcdir)/netconf/test/support/callbacks/running-cb-checker.cpp \
$(top_srcdir)/netconf/test/support/callbacks/system-cb-checker-factory.cpp \
$(top_srcdir)/netconf/test/support/callbacks/integ-cb-checker-factory.cpp \
$(top_srcdir)/netconf/test/support/msg-util/xpo-query-builder.cpp \
$(top_srcdir)/netconf/test/support/msg-util/NCMessageBuilder.cpp \
$(top_srcdir)/netconf/test/support/msg-util/state-data-query-builder.cpp \
$(top_srcdir)/netconf/test/support/db-models/device-test-db-debug.cpp \
$(top_srcdir)/netconf/test/support/db-models/state-data-test-db.cpp \
$(top_srcdir)/netconf/test/support/db-models/device-test-db.cpp \
$(top_srcdir)/netconf/test/support/fixtures/device-module-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/state-data-module-common-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/integ-fixture-helper-factory.cpp \
$(top_srcdir)/netconf/test/support/fixtures/abstract-global-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/test-context.cpp \
$(top_srcdir)/netconf/test/support/fixtures/system-fixture-helper-factory.cpp \
$(top_srcdir)/netconf/test/support/fixtures/system-fixture-helper.cpp \
$(top_srcdir)/netconf/test/support/fixtures/integ-fixture-helper.cpp \
$(top_srcdir)/netconf/test/support/fixtures/state-get-module-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/simple-container-module-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/base-suite-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/query-suite-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/device-module-common-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/simple-yang-fixture.cpp \
$(top_srcdir)/netconf/test/support/fixtures/device-get-module-fixture.cpp \
$(top_srcdir)/netconf/test/support/nc-session/sys-test-nc-session-base.cpp \
$(top_srcdir)/netconf/test/support/nc-session/abstract-nc-session.cpp \
$(top_srcdir)/netconf/test/support/nc-session/remote-nc-session-factory.cpp \
$(top_srcdir)/netconf/test/support/nc-session/remote-nc-session.cpp \
$(top_srcdir)/netconf/test/support/nc-session/spoof-nc-session.cpp \
$(top_srcdir)/netconf/test/support/nc-session/spoof-nc-session-factory.cpp \
$(top_srcdir)/netconf/test/support/misc-util/cpp-unit-op-formatter.cpp \
$(top_srcdir)/netconf/test/support/misc-util/log-utils.cpp \
$(top_srcdir)/netconf/test/support/misc-util/ptree-utils.cpp \
$(top_srcdir)/netconf/test/support/misc-util/base64.cpp

libyumatest_la_CPPFLAGS = $(LIBXML2_CFLAGS) -DLIBXML2_ENABLED -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
libyumatest_la_LDFLAGS=-lboost_unit_test_framework  $(top_builddir)/netconf/src/mgr/libyumamgr.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS) -lssh2 -lncurses

#Test modules - YANGs
yang_DATA = \
$(top_srcdir)/netconf/test/modules/yang/simple_yang_test.yang \
$(top_srcdir)/netconf/test/modules/yang/simple_list_test.yang \
$(top_srcdir)/netconf/test/modules/yang/device_test.yang

#Test modules - SILs
netconfmodule_LTLIBRARIES = libsimple_yang_test.la
libsimple_yang_test_la_SOURCES = \
$(top_srcdir)/netconf/test/modules/build-sil/simple_yang_test/simple_yang_test.cpp
libsimple_yang_test_la_CPPFLAGS = $(LIBXML2_CFLAGS) -DLIBXML2_ENABLED -I $(top_srcdir)/netconf -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
libsimple_yang_test_la_LDFLAGS = -module $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS)

netconfmodule_LTLIBRARIES+=libsimple_list_test.la
libsimple_list_test_la_SOURCES = \
$(top_srcdir)/netconf/test/modules/build-sil/simple_list_test/simple_list_test.cpp
libsimple_list_test_la_CPPFLAGS = $(LIBXML2_CFLAGS) -DLIBXML2_ENABLED -I $(top_srcdir)/netconf -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
libsimple_list_test_la_LDFLAGS = -module $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS)

netconfmodule_LTLIBRARIES+=libdevice_test.la
libdevice_test_la_SOURCES = \
$(top_srcdir)/netconf/test/modules/build-sil/device_test/device_test.cpp
libdevice_test_la_CPPFLAGS = $(LIBXML2_CFLAGS) -DLIBXML2_ENABLED -I $(top_srcdir)/netconf -I $(top_srcdir)/libtoaster/src/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump
libdevice_test_la_LDFLAGS = -module $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/agt/libyumaagt.la $(top_builddir)/netconf/src/ncx/libyumancx.la -lz  $(LIBS)


