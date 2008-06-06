#
# platform.profile
#
# DEBUG VERSION
#

### GCC + [LINUX or MACOSX]

# leave -Werror out for now
CWARN=-Wall -Wno-long-long -Wformat-y2k -Winit-self \
	-Wmissing-include-dirs -Wswitch-default -Wunused-parameter \
	-Wextra -Wundef -Wshadow -Wpointer-arith \
	-Wwrite-strings -Wbad-function-cast -Wcast-qual -Wcast-align \
	-Waggregate-return -Wstrict-prototypes -Wold-style-definition \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wpacked -Wunreachable-code -Winvalid-pch \
	-Wredundant-decls -Wnested-externs -Winline -Wconversion

# debug
ifdef BLD
  CFLAGS=-DDEBUG -DLINUX -DGCC -DHAS_FLOAT $(CWARN)
else
  CFLAGS=-ggdb3 -DDEBUG -DLINUX -DGCC -DHAS_FLOAT $(CWARN)
endif


# added /sw/include for MacOSX
ifdef MAC
# MACOSX version
CINC=-I. -I../agt -I../agtinst -I../db -I../mgr \
    -I../ncx -I../platform \
    -I/usr/include -I/usr/include/libxml2 \
    -I/usr/include/libxml2/libxml \
    -I/sw/include
else
# LINUX version
CINC=-I. -I../agt -I../agtinst -I../db -I../mgr \
    -I../ncx -I../platform \
    -I/usr/include -I/usr/include/libxml2 \
    -I/usr/include/libxml2/libxml
endif
        
CC=gcc
LINK=gcc
##LIBFLAGS=-lsocket

TBASE=../../target
LBASE=$(TBASE)/lib
LIBTOOL=ar
#LFLAGS=-v --no-as-needed
LFLAGS=
LPATH=-L$(LBASE)

################ OBJS RULE #############
OBJS = $(patsubst %.c,$(TARGET)/%.o,$(wildcard *.c))

################ DEPS RULE #############
DEPS = $(patsubst %.c,%.D,$(wildcard *.c))

######################## PLATFORM DEFINITIONS #############
PLATFORM_CPP=

.PHONY: all superclean clean test install 

######################### MAKE DEPENDENCIES ###############
COMPILE.c= $(CC) $(CFLAGS) $(CPPFLAGS) $(PLATFORM_CPP) $(CINC) $(SUBDIR_CPP) $(TARGET_ARCH) -c


$(TARGET)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(PLATFORM_CPP) $(CINC) $(SUBDIR_CPP) $(TARGET_ARCH) -c -o $@ $< 



# Common library rule

$(LBASE)/lib%.a: $(OBJS)
	$(LIBTOOL) cr $@ $(OBJS)


#### common cleanup rules

# dependency rule to make temp .D files from .c sources
# all the .D files are collected and appended to the
# appropriate Makefile when 'make depend' is run
# this rule is kept here to make sure it matches COMPILE.c
%.D: %.c
	$(CC) -MM -MG -MT $(TARGET)/$(patsubst %.c,%.o,$<) \
	-Wall -Wcomment $(CPPFLAGS) $(PLATFORM_CPP) $(CINC) \
	$(SUBDIR_CPP) $(TARGET_ARCH) -c $< > $@

