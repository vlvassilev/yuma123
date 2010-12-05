noinst_LIBRARIES = libmgr.a

libmgr_a_SOURCES = \
	mgr.c \
	mgr_cap.c \
	mgr_hello.c \
	mgr_io.c \
	mgr_not.c \
	mgr_rpc.c \
	mgr_ses.c \
	mgr_signal.c \
	mgr_top.c \
	mgr_val_parse.c \
	mgr_xml.c

libmgr_a_CFLAGS = 
libmgr_a_CFLAGS += @LIBSSH2_CFLAGS@
libmgr_a_CFLAGS += @LIBXML_CFLAGS@ @LIBXML_CFLAGS@/libxml

INCLUDES = -I@top_srcdir@/src/includes  -I@top_srcdir@/src/ncx