#
# platform.profile
#

# libncx version
# need some way to set versions without using
# environment vars, so it works in plain debuild, rpmbuild
#
LIBNCX_MAJOR_VERSION=1
LIBNCX_MINOR_VERSION=14
SOVERSION=$(LIBNCX_MAJOR_VERSION).$(LIBNCX_MINOR_VERSION)

# default DESTDIR is NULL; it is only used by packaging builds

# set default PREFIX to /usr
ifndef PREFIX
  PREFIX=/usr
endif

CINC=
ifdef WINDOWS
   CINC += -I/usr/i586-mingw32msvc/include
endif

CINC +=-I. -I../agt -I../mgr \
    -I../ncx -I../platform \
    -I../ydump \
    -I$(DESTDIR)$(PREFIX)/include \
    -I$(DESTDIR)$(PREFIX)/include/libxml2 \
    -I$(DESTDIR)$(PREFIX)/include/libxml2/libxml


# added /sw/include for MacOSX
ifdef MAC
# MACOSX version
  CINC +=-I/sw/include
  CFLAGS += -DMACOSX=1
endif

TBASE=../../target

ifdef DESTDIR
LBASE=$(DESTDIR)$(PREFIX)/lib
else
LBASE=$(TBASE)/lib
endif

### platform.profile.cmn included inline here!!!

#
# platform.profile.cmn
#

ifdef DESTDIR
OWNER=
else
ifdef MAC
OWNER=-oroot
else
OWNER= --owner=root
endif
endif

ifdef DEBUG
WERROR=-Werror
else
WERROR=
endif

#std=gnu99 is only used for floating point libraries
CWARN=-Wall -Wno-long-long -Wformat-y2k -Winit-self \
	-Wswitch-default -Wunused-parameter \
	-Wextra -Wundef -Wshadow -Wpointer-arith \
	-Wwrite-strings -Wbad-function-cast -Wcast-qual -Wcast-align \
	-Waggregate-return -Wstrict-prototypes -Wold-style-definition \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wpacked -Winvalid-pch \
	-Wredundant-decls -Wnested-externs -Winline -std=gnu99 $(WERROR)


# -Wunreachable-code removed due to -O3
# -O3 changed to -O2 due to code bloat from inline functions

ifdef WINDOWS
CDEFS=-DDEBUG -DWINDOWS -DGCC
else
CDEFS=-DDEBUG -DLINUX -DGCC
endif

ifndef NOFLOAT
  CDEFS += -DHAS_FLOAT
endif

ifdef P64BIT
  CDEFS += -DP64BIT
endif

CFLAGS+=$(CDEFS) $(CWARN) -fPIC

ifdef DEBUG
  CFLAGS += -ggdb3
else
  CFLAGS += -O2
endif

# memory leak debugging mode
ifdef MEMTRACE
  CFLAGS += -DMEMORY_DEBUG=1
endif

# check if debian or RPM release build
ifdef RELEASE
  CFLAGS += -DRELEASE=$(RELEASE)
endif


ifdef MAC
   GRP=
else
ifdef DESTDIR
   GRP=
else
   GRP=--group=root
endif
endif

ifdef STATIC
LIBSUFFIX=a
LIBNCXSUFFIX=a
else
ifdef MAC
LIBSUFFIX=dylib
LIBNCXSUFFIX=dylib
else
LIBSUFFIX=so
LIBNCXSUFFIX=so.$(SOVERSION)
endif
endif

ifdef WINDOWS
CC=i586-mingw32msvc-gcc
LINK=i586-mingw32msvc-gcc
LIBTOOL=i586-mingw32msvc-ar
RANLIB=i586-mingw32msvc-ranlib
else
CC=$(CROSS_TARGET)gcc
LINK=$(CC)
LIBTOOL=$(CROSS_TARGET)ar
RANLIB=$(CROSS_TARGET)ranlib
endif

LINT=splint
LINTFLAGS= '-weak -macrovarprefix "m_"'
##LIBFLAGS=-lsocket


#LFLAGS=-v --no-as-needed
LFLAGS=-lm
LPATH=-L$(LBASE)

CEES = $(wildcard *.c)

HEES = $(wildcard *.h)

################ OBJS RULE #############
OBJS = $(patsubst %.c,$(TARGET)/%.o,$(CEES))

################ DEPS RULE #############
DEPS = $(patsubst %.c,%.D,$(wildcard *.c))

######################## PLATFORM DEFINITIONS #############
PLATFORM_CPP=

.PHONY: all superclean clean test install uninstall \
        distclean depend lint notabs addheader fixcopyright

######################### MAKE RULES ######################
COMPILE.c= $(CC) $(CFLAGS) $(CPPFLAGS) $(PLATFORM_CPP) \
           $(CINC) $(SUBDIR_CPP) $(TARGET_ARCH) -c


$(TARGET)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(PLATFORM_CPP) \
        $(CINC) $(SUBDIR_CPP) $(TARGET_ARCH) -c -o $@ $< 



# Common library rule

$(LBASE)/lib%.a: $(OBJS)
	$(LIBTOOL) cr $@ $(OBJS)
	$(RANLIB) $@


#### common cleanup rules

lint:
	$(LINT) $(LINTFLAGS) $(CDEFS) $(CPPFLAGS) $(PLATFORM_CPP) \
	$(CINC) $(SUBDIR_CPP) *.c


# dependency rule to make temp .D files from .c sources
# all the .D files are collected and appended to the
# appropriate Makefile when 'make depend' is run
# this rule is kept here to make sure it matches COMPILE.c
%.D: %.c
	$(CC) -MM -MG -MT $(TARGET)/$(patsubst %.c,%.o,$<) \
	-Wall -Wcomment $(CPPFLAGS) $(PLATFORM_CPP) $(CINC) \
	$(SUBDIR_CPP) $(TARGET_ARCH) -c $< > $@


notabs:
	for c in $(CEES); do\
	  cp $$c $$c.save;\
	  expand $$c > $$c.ex;\
	  mv $$c.ex $$c;\
	done

addheader:
	if [ ! -f ../platform/header.txt]; then \
	  echo "Error: platform/header.txt is missing!"; \
	  exit 1; \
	fi
	for c in $(CEES); do\
	  cp $$c $$c.save;\
	  cp ../platform/header.txt $$c.hdr;\
	  cat $$c >> $$c.hdr;\
	  mv $$c.hdr $$c;\
	done
	for h in $(HEES); do\
	  cp $$h $$h.save;\
	  cp ../platform/header.txt $$h.hdr;\
	  cat $$h >> $$h.hdr;\
	  mv $$h.hdr $$h;\
	done


fixcopyright:
	for c in $(CEES); do\
          sed 's/Netconf Central, Inc./Andy Bierman/' $$c > $$c.tmp;\
	  mv $$c.tmp $$c;\
	done
	for h in $(HEES); do\
          sed 's/Netconf Central, Inc./Andy Bierman/' $$h > $$h.tmp;\
	  mv $$h.tmp $$h;\
	done

