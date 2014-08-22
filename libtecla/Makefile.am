noinst_LTLIBRARIES = libtecla.la

libtecla_la_SOURCES = \
$(top_srcdir)/libtecla/chrqueue.c \
$(top_srcdir)/libtecla/cplfile.c \
$(top_srcdir)/libtecla/cplmatch.c \
$(top_srcdir)/libtecla/direader.c \
$(top_srcdir)/libtecla/enhance.c \
$(top_srcdir)/libtecla/errmsg.c \
$(top_srcdir)/libtecla/expand.c \
$(top_srcdir)/libtecla/freelist.c \
$(top_srcdir)/libtecla/getline.c \
$(top_srcdir)/libtecla/hash.c \
$(top_srcdir)/libtecla/history.c \
$(top_srcdir)/libtecla/homedir.c \
$(top_srcdir)/libtecla/ioutil.c \
$(top_srcdir)/libtecla/keytab.c \
$(top_srcdir)/libtecla/pathutil.c \
$(top_srcdir)/libtecla/pcache.c \
$(top_srcdir)/libtecla/stringrp.c \
$(top_srcdir)/libtecla/strngmem.c \
$(top_srcdir)/libtecla/version.c

libtecla_la_CPPFLAGS = -I$(top_srcdir)/netconf/src/agt -I$(top_srcdir)/netconf/src/mgr -I$(top_srcdir)/libtecla -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DUSE_TERMINFO=1 -DHAVE_CURSES_H=1 -DHAVE_TERM_H=1 -DHAVE_SYS_SELECT_H=1 -DHAVE_SELECT=1 -DHAVE_SYSV_PTY=1  -D_SVID_SOURCE -D_BSD_SOURCE
libtecla_la_LDFLAGS = -static

