TESTS=test_yangcli_wordex
check_PROGRAMS = test_yangcli_wordex

test_yangcli_wordex_SOURCES = \
$(top_srcdir)/netconf/test/test-suites/integ/test_yangcli_wordex.c \
$(top_srcdir)/netconf/src/yangcli/yangcli_wordexp.c

test_yangcli_wordex_CPPFLAGS = -I $(top_srcdir)/netconf/src/yangcli/ -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/netconf/src/ncx -I$(top_srcdir)/netconf/src/platform -I$(top_srcdir)/netconf/src/ydump -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
test_yangcli_wordex_LDFLAGS = $(top_builddir)/netconf/src/mgr/libmgr.la $(top_builddir)/netconf/src/agt/libagt.la $(top_builddir)/netconf/src/ncx/libncx.la -lxml2 -lz  -ldl -lssh2 -lssl -lcrypto -lncurses
