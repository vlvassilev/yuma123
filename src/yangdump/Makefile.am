modulesdir = $(libdir)/yangdump
sbin_PROGRAMS = yangdump
modules_LTLIBRARIES = 

yangdump_SOURCES = \
	yangdump.c 


yangdump_CFLAGS =
yangdump_CFLAGS += @LIBXML_CFLAGS@ @LIBXML_CFLAGS@/libxml

yangdump_INCLUDES = 

yangdump_LDADD = ../ydump/libydump.a ../ncx/libncx.a

yangdump_LDFLAGS = -lxml2

INCLUDES = -I@top_srcdir@/include -I@top_srcdir@/src/ncx -I@top_srcdir@/src/ydump
