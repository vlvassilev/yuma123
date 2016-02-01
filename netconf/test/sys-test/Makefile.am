TESTS=test-shutdown
check_PROGRAMS=test-shutdown
test_shutdown_SOURCES = \
$(top_srcdir)/netconf/test/sys-test/shutdown.cpp \
$(top_srcdir)/netconf/test/test-suites/system/shutdown-tests.cpp
test_shutdown_CPPFLAGS = -DBOOST_TEST_DYN_LINK -std=c++0x -I $(top_srcdir)/netconf -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_shutdown_LDFLAGS = -lboost_unit_test_framework $(top_builddir)/netconf/test/libyumatest.la $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses


