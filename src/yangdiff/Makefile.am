modulesdir = $(libdir)/yangdiff
sbin_PROGRAMS = yangdiff
modules_LTLIBRARIES = 

yangdiff_SOURCES = \
	yangdiff.c \
	yangdiff_grp.c \
	yangdiff_obj.c \
	yangdiff_typ.c \
	yangdiff_util.c

yangdiff_CFLAGS =
yangdiff_CFLAGS += @LIBXML_CFLAGS@ @LIBXML_CFLAGS@/libxml
yangdiff_INCLUDES = 

yangdiff_LDADD = ../ncx/libncx.a

yangdiff_LDFLAGS = 

INCLUDES = -I@top_srcdir@/include -I@top_srcdir@/src/ncx

