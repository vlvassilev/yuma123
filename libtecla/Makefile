#-----------------------------------------------------------------------
# This is the template that the libtecla configure script uses to create
# the libtecla Makefile. It does this by replacing all instances of
# @name@ with the value of the correspondingly named configuration
# variable. You should find another file in the same directory as this
# one, called "configure.in". The latter file contains extensive comments
# explaining how this all works.
#-----------------------------------------------------------------------

# Where is the source code?

srcdir = .

# Where do you want to install the library, its header file, and the man pages?

prefix=/usr/local
exec_prefix=${prefix}
LIBDIR=${exec_prefix}/lib
INCDIR=${prefix}/include
MANDIR=${prefix}/man
BINDIR=${exec_prefix}/bin

# Which C compiler do you want to use?

CC = gcc

# If 'make' doesn't define the MAKE variable, define it here.



# To use RANLIB set the RANLIB variable to ranlib. Otherwise set it to
# :, which is the bourne shell do-nothing command.

RANLIB = ranlib

# Optional flags to pass to the linker.

LDFLAGS = 

# Optional C pre-processor flags.

CPPFLAGS = 

# The following optional defines change the characteristics of the library.
#
# USE_TERMINFO
#   Use the terminfo terminal information database when looking up
#   terminal characteristics. Most modern UNIX and UNIX-like operating
#   systems support terminfo, so this define should normally be included.
#   If in doubt leave it in, and see if the library compiles.
# USE_TERMCAP
#   If you don't have terminfo but do have the termcap database, replace
#   the -DUSE_TERMINFO with -DUSE_TERMCAP. If there is a termcap.h in
#   /usr/include/, also add -DHAVE_TERMCAP_H.
#
# If neither USE_TERMINFO nor USE_TERMCAP are included, ANSI VT100 control
# sequences will be used to control all terminal types.
#
# For Solaris and Linux, use:
#
#  DEFINES = -DUSE_TERMINFO
#

DEFINES = -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DUSE_TERMINFO=1 -DHAVE_CURSES_H=1 -DHAVE_TERM_H=1 -DHAVE_SYS_SELECT_H=1 -DHAVE_SELECT=1 

#
# The following defines are used in addition to the above when compiling
# the reentrant version of the library. Note that the definition of
# _POSIX_C_SOURCE to request reentrant functions, has the unfortunate
# side-effect on some systems of stopping the TIOCGWINSZ ioctl macro from
# getting defined. This in turn stops the library from being
# able to respond to terminal size changes. Under Solaris this can be
# remedied by adding -D__EXTENSIONS__. On linux this isn't necessary.
# If you don't get this right, the library will still work, but
# it will get confused if the terminal size gets changed and you try to
# edit a line that exceeds the terminal width.
#
# Thus on Solaris you should use:
#
#   DEFINES_R = -D_POSIX_C_SOURCE=199506L -D__EXTENSIONS__
#
# and on linux you should use:
#
#   DEFINES_R = -D_POSIX_C_SOURCE=199506L
#

DEFINES_R = -D_POSIX_C_SOURCE=199506L -DPREFER_REENTRANT

#
# The compiler optimization flags. I like to keep this separate so
# that I can set it to -g from the 'make' command line without having
# to edit this file when debugging the library. If you aren't working
# on modifying the library, leave this set to -O.
#

OPT = -O

#
# These are paranoid gcc warning flags to use when compiling new code.
# Simply invoke make with WARNING_FLAGS='$(PEDANTIC_FLAGS)'.
#
PEDANTIC_FLAGS=-Wall -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls

#
# Specify any extra compiler warning options that you want to use.
# Leave this blank unless you are porting the library to a new system,
# or modifying the library.
#

WARNING_FLAGS=

#
# If you want to compile the demo program, specify any system
# libraries that are needed for the terminal I/O functions.
#
# If you are using terminfo, you will probably only need -lcurses.
# For termcap you may need -ltermcap or -ltermlib.
#
# For Solaris, use:
#
#  LIBS = -lcurses
#
# For linux, use:
#
#  LIBS = -lncurses
#

LIBS =  -lcurses

#
# List the default target libraries. This should be one or
# both of the words "normal" and "reentrant".
#
TARGETS = normal reentrant

#
# List which types of the above libraries are required.
# This should be one or both of the words "static" and "shared".
#
TARGET_LIBS = static shared

#
# If you want the demo programs to be built, the following variable
# should be assigned the single word: demos. If it isn't assigned
# anything, the demo programs won't be built.
#
DEMOS = demos

#
# List the programs that are to be made by default.
#
PROGRAMS = enhance

#
# List programs for which reentrant versions are to be built by default.
#
PROGRAMS_R =

#-----------------------------------------------------------------------
# You shouldn't need to change anything below this line.
#-----------------------------------------------------------------------

CFLAGS = $(OPT) $(WARNING_FLAGS) $(DEFINES) -D_SVID_SOURCE -D_BSD_SOURCE  -isystem /usr/lib/gcc/i686-redhat-linux/4.4.2/include -isystem /usr/include -fpic

default: $(TARGETS)

normal:
	@$(MAKE) -f $(srcdir)/Makefile.rules TARGETS="$(TARGET_LIBS)" SUFFIX="" CFLAGS="$(CFLAGS)" CC="$(CC)" OBJDIR=normal_obj LINK_SHARED='ld -o $$@ -soname libtecla$$(SUFFIX).so.1 -shared --version-script=$$(srcdir)/libtecla.map $$(LIB_OBJECTS) $$(LIBS) -lc /usr/lib/gcc/i686-redhat-linux/4.4.2/libgcc.a' SHARED_EXT='.so.1.6.1' SHARED_ALT='.so .so.1' LIBS='$(LIBS)' srcdir='$(srcdir)' LIBDIR='$(LIBDIR)' LN_S='ln -s' DEMOS="$(DEMOS)" PROGRAMS='$(PROGRAMS)' RANLIB='$(RANLIB)' LDFLAGS=$(LDFLAGS) CPPFLAGS=$(CPPFLAGS)

reentrant:
	@$(MAKE) -f $(srcdir)/Makefile.rules TARGETS="$(TARGET_LIBS)" SUFFIX="_r" CFLAGS="$(CFLAGS) $(DEFINES_R)" CC="$(CC)" OBJDIR=reentrant_obj LINK_SHARED='ld -o $$@ -soname libtecla$$(SUFFIX).so.1 -shared --version-script=$$(srcdir)/libtecla.map $$(LIB_OBJECTS) $$(LIBS) -lc /usr/lib/gcc/i686-redhat-linux/4.4.2/libgcc.a' SHARED_EXT='.so.1.6.1' SHARED_ALT='.so .so.1' LIBS='$(LIBS)' srcdir='$(srcdir)' LIBDIR='$(LIBDIR)' LN_S='ln -s' DEMOS="$(DEMOS)" PROGRAMS='$(PROGRAMS_R)' RANLIB='$(RANLIB)' LDFLAGS=$(LDFLAGS) CPPFLAGS=$(CPPFLAGS)

demos: normal

demos_r: reentrant

clean:
	rm -rf *.o normal_obj reentrant_obj libtecla*.a demo demo[0-9] demo_r demo[0-9]_r enhance *~ man/*~ man/*/*~ html/*~ compile_reentrant compile_normal `/bin/ls -1 man/*/*.in | sed 's/\.in$$//'`
	@endings=".so.1.6.1 .so .so.1" ; \
	for alt in $$endings ; do \
	  lib="libtecla*$$alt" ; \
	  rm -f $$lib; echo rm -f $$lib ; \
	done

distclean: clean
	rm -rf config.cache config.status config.log Makefile libtecla.map.opt \
               autom*.cache
	cp $(srcdir)/Makefile.stub Makefile

install_lib: $(TARGETS) $(LIBDIR)
	@for lib in libtecla.a libtecla_r.a ; do \
	   if [ -f $$lib ] ; then \
	     cp $$lib $(LIBDIR)/ ; chmod ugo+r $(LIBDIR)/$$lib; \
	     echo "cp $$lib $(LIBDIR)/ ; chmod ugo+r $(LIBDIR)/$$lib"; \
	   fi ; \
	done
	@for lib in libtecla libtecla_r ; do \
	   src="$$lib.so.1.6.1"; \
	   if [ -f $$src ] ; then \
	     dst="$(LIBDIR)/$$src"; \
	     cp -f $$src $$dst; chmod a=rX $$dst; \
	     echo "cp -f $$src $$dst ; chmod a=rX $$dst"; \
	     endings=".so .so.1" ; \
	     for alt in $$endings ; do \
	       lnk="$$lib$$alt"; \
	       (cd $(LIBDIR); rm -f $$lnk; ln -s $$src $$lnk); \
	       echo "(cd $(LIBDIR); rm -f $$lnk; ln -s $$src $$lnk)"; \
	     done ; \
	   fi ; \
	done

install_inc: $(INCDIR)
	@if [ -f $(srcdir)/libtecla.h ]; then \
	  cp $(srcdir)/libtecla.h $(INCDIR)/ ; chmod ugo+r $(INCDIR)/libtecla.h; \
	  echo "cp $(srcdir)/libtecla.h $(INCDIR)/ ; chmod ugo+r $(INCDIR)/libtecla.h"; \
	fi

install_man: $(MANDIR) libr_man func_man prog_man misc_man file_man

libr_man:
	if test "yes"_ = "yes"_; then \
	  for template in man/libr/*.in; do \
	    page=`basename "$$template" .in`; \
	    $(srcdir)/install-sh -c -m 644 man/libr/$$page ${MANDIR}/man3/$$page.3; \
	  done ; \
	fi

func_man:
	if test "yes"_ = "yes"_; then \
	  for template in man/func/*.in; do \
	    page=`basename "$$template" .in`; \
	    $(srcdir)/install-sh -c -m 644 man/func/$$page ${MANDIR}/man3/$$page.3; \
	  done ; \
	fi

prog_man:
	if test "yes"_ = "yes"_; then \
	  for template in man/prog/*.in; do \
	    page=`basename "$$template" .in`; \
	    $(srcdir)/install-sh -c -m 644 man/prog/$$page ${MANDIR}/man1/$$page.1; \
	  done ; \
	fi

misc_man:
	if test "yes"_ = "yes"_; then \
	  for template in man/misc/*.in; do \
	    page=`basename "$$template" .in`; \
	    $(srcdir)/install-sh -c -m 644 man/misc/$$page ${MANDIR}/man7/$$page.7; \
	  done ; \
	fi

file_man:
	if test "yes"_ = "yes"_; then \
	  for template in man/file/*.in; do \
	    page=`basename "$$template" .in`; \
	    $(srcdir)/install-sh -c -m 644 man/file/$$page ${MANDIR}/man5/$$page.5; \
	  done ; \
	fi

install_bin: $(BINDIR) $(PROGRAMS) $(PROGRAMS_R)
	progs="$(PROGRAMS) $(PROGRAMS_R)"; \
	for prog in $$progs; do \
	  $(srcdir)/install-sh -c -m 755 -s $$prog $(BINDIR)/; \
	done

install: install_lib install_inc install_man install_bin

# Make any missing installation directories.

$(MANDIR) $(LIBDIR) $(INCDIR) $(BINDIR):
	$(srcdir)/install-sh -d $@
	chmod ugo+rx $@
