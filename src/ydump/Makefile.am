noinst_LIBRARIES = libydump.a

libydump_a_SOURCES = \
	c.c \
	c_util.c \
	cyang.c \
	h.c \
	html.c \
	py_util.c \
	sql.c \
	tg2.c \
	xsd.c \
	xsd_typ.c \
	xsd_util.c \
	xsd_yang.c \
	yangdump_util.c \
	yangstats.c \
	yangyin.c \
	ydump.c

libydump_a_CFLAGS = 
libydump_a_CFLAGS += @LIBXML_CFLAGS@ @LIBXML_CFLAGS@/libxml

libydump_a_LIBADD = ../ncx/libncx.a

INCLUDES = -I@top_srcdir@/src/includes -I@top_srcdir@/src/ncx