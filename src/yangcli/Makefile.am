modulesdir = $(libdir)/yangcli
sbin_PROGRAMS = yangcli
modules_LTLIBRARIES = 

yangcli_SOURCES = \
	yangcli.c \
	yangcli_autoload.c \
	yangcli_autolock.c \
	yangcli_cmd.c \
	yangcli_cond.c \
	yangcli_eval.c \
	yangcli_list.c \
	yangcli_save.c \
	yangcli_show.c \
	yangcli_tab.c \
	yangcli_util.c


yangcli_CFLAGS = 
yangcli_CFLAGS += @LIBXML_CFLAGS@ @LIBXML_CFLAGS@/libxml
yangcli_CFLAGS += @LIBSSH2_CFLAGS@

yangcli_LDADD = ../ncx/libncx.a ../mgr/libmgr.a 

yangcli_LDFLAGS = -L../../libtecla -ltecla

INCLUDES = -I@top_srcdir@/include  -I@top_srcdir@/src/ncx  -I@top_srcdir@/src/mgr -I@top_srcdir@/libtecla 