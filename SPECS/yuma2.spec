Name:           yuma
Version:        2.1
Release:        2%{?dist}
Summary:        YANG-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.org/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
Yuma is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.
The yangcli client supports single sessions over SSH with some
script support.  The yangdump and yangdiff development tools are also
included, to compile and process YANG modules.

Requires: ncurses
Requires: libxml2
Requires: libssh2

%define debug_package %{nil}

%package dev

Summary:  YANG-based Unified Modular Automation Tools (Developer)

%description dev
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  This package contains H files, scripts,
and other files needed to create SIL code for use with
the netconfd server.

%post dev
echo "Yuma developer files installed."

%files dev
%defattr(-,root,root,-)
%{_bindir}/make_sil_dir
%{_mandir}/man1/make_sil_dir.1.gz
%{_includedir}/yuma/
%{_datadir}/yuma/util/
%{_datadir}/yuma/src/libtoaster/


%package doc

Summary:  YANG-based Unified Modular Automation Tools (Documentation)

%description doc
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  This package contains the Yuma user manuals
in PDF and HTML format.

%post doc
echo "Yuma documentation files installed."

%files doc
%defattr(-,root,root,-)
/usr/share/doc/yuma/server-call-chain.txt
/usr/share/doc/yuma/pdf/
/usr/share/doc/yuma/html/


# main package rules

%prep
%setup -q

%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
make RELEASE=2 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id RELEASE=2 DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/yangcli
%{_bindir}/yangdump
%{_bindir}/yangdiff
%{_sbindir}/netconfd
%{_sbindir}/netconf-subsystem
%{_sysconfdir}/yuma/yangcli-sample.conf
%{_sysconfdir}/yuma/yangdiff-sample.conf
%{_sysconfdir}/yuma/yangdump-sample.conf
%{_sysconfdir}/yuma/netconfd-sample.conf
/usr/share/doc/yuma/yuma-legal-notices.pdf
/usr/share/doc/yuma/AUTHORS
/usr/share/doc/yuma/README
/usr/share/doc/yuma/COPYRIGHT
%{_mandir}/man1/yangcli.1.gz
%{_mandir}/man1/yangdiff.1.gz
%{_mandir}/man1/yangdump.1.gz
%{_mandir}/man1/netconfd.1.gz
%{_mandir}/man1/netconf-subsystem.1.gz
%{_datadir}/yuma/modules/*
%{_libdir}/libncx.so*
%{_libdir}/libagt.so*
%{_libdir}/yuma/

%post
echo "Yuma installed."
echo "Check the user manuals in /usr/share/doc/yuma"

%changelog
* Tue Sep 27 2011 Andy Bierman <andy at netconfcentral.org> 2.1-2 [1457]
  * Build
    * fix bug added recently that breaks build in libtoaster
      in a plain build (YUMA_HOME not set) and breaks CYGWIN
      build as well
* Sun Sep 25 2011 Andy Bierman <andy at netconfcentral.org> 2.1-1 [1424]
  * netconfd
    * add --runpath to netconfd.yang
    * fix bug reported by Sara Dickinson where leafref was
      not getting validated during commit; turned out leafrefs and
      instance-identifiers were not getting validated for 
      target=running or during commit
    * fix bug where <validate> parameters were not handled correctly
      and <ok> was returned without validating the target config.
    * add --factory-startup CLI parameter.
      Currently there is no way to rewrite the invisible
      startup-cfg.xml if --with-startup=false, except
      by modifying the factory settings and saving the config.
      This parameter forces the startup config and the running
      config to contain the factory default settings during
      initialization.
    * fixed bug (reported by Sara Dickinson) where must-stmt tests
      for sibling nodes were getting skipped as soon as 1 must-test
      failed.  This could result in nodes that should be invalid
      left in the running config during load_running_config
      if --startup-error=continue (the default)
    * fix bug in agt_proc.c where CPU cores were not causing
      a new val entry to be created.  Introduced with vi-cov commit
    * add yuma-arp module, implemented by Igor Smolyar
    * fixed bug 3404233
      The client and server both incorrectly accepted
      an XML node for a YANG choice or case node.
      These nodes do not exist in a YANG data tree, just
      in the YANG object tree
    * fix bug in COMMIT phase where SIL callback functions
      for nested nodes were not getting invoked for create
      and merge edit operations.
    * fixed bug introduced in last release where SIL validation
      callbacks are being called multiple times
    * fixed bug where commit callback for editop=OP_EDITOP_DELETE
      the curnode is a detached node -- the parent node is NULL.
    * fixed bug 3395740
    * source tree specified by YUMA_HOME environment variable
      no longer required to be present
    * added libagt.so to shared library install location (d: /usr/lib)
    * using shared libraries in default location for libagt and libncx
      for ubuntu and RPM packaging.  Not static libaries anymore!
    * building and installing libncx.a and libagt.a if STATIC=1 
      present in make cmd
    * updated SIL makefile so YUMA_HOME is not used unless
      FORCE_YUMA_HOME=1 is present in the make cmd;
      /usr/lib/yuma is used by default
    * make /arp node present by default, using refactored code from agt_acm.c

  * yangcli
    * add JSON output support for --display-mode=json
      and saving data with @foo.json
    * fix bug where manager session control block not checked for NULL
    * add external parameter support for RPC commands

      yangcli> some-command @filespec.xml

      filespec.xml (in YUMA_DATAPATH) == RPC input element
      <input>
        <a>10</a>
        <b>fred</b>
      </input>
    * fixed bug where an edit command (e.g., create) on
      a choice or case node would generate an XML node
      for the choice or case.  Now being removed from
      the XML payload before an <edit-config> is sent

  * yangdump:
    * added support for split SIL filesor combined (old way):
      * --format=yc : Yuma SIL C file
      * --format=yh : Yuma SIL H file
      * --format=uc : User SIL C file
      * --format=uc : User SIL H file
      * --format=c  : Combined Yuma/User SIL C file
      * --format=h  : Combined Yuma/User SIL H file
   * added support for code generation for automatic retrieval
     of ancestor-or-self key values in user SIL callbacks
    * Remove #ifdef around #include directives for generated files.
    * fixed bug 3404234
      --format=c output (SIL code) was not handling nested
        config=false containers (lists and leaf-lists still
        not handled!).  Now nested config=false containers
        will automatically be created
    * fixed bug for --format=c where edit callbacks were being
      generated for OBJ_TYP_CHOICE and OBJ_TYP_CASE nodes.
      Since these nodes never exist in a database, these callbacks
      get registered but never invoked
    * fixed bug where SIL code generated for boolean, union, and identityref
      datatypes is incorrect -- the data type will be whatever the union
      was parsed will be treated as a string; causes segfault;
      !!! not fixed in v1
      * union now passed to User SIL function as val_value_t
        instead of string
      * affected edit callback functions and notification send functions
   * added __cplusplus 'extern C' wrappers to H file generation
     for --format=h|uh|yh
  * Build:
   * added --split parameter to make_sil_dir
     * make_sil_dir --split foo  : makes files in foo/src/
        * y_foo.c : Yuma SIL C file
        * y_foo.h : Yuma SIL H file
        * u_foo.c : User SIL C file
        * u_foo.h : User SIL H file
    * building and installing libagt as a dynamic library
    * building Ubuntu package will mostly dynamic libraries
      instead of STATIC=1 and FULL_STATIC=1
    * bumped version to 2.1
    * updated Makefiles to allow debian debuild of 3 packages 
      when DEBIAN=1 set:
        no flag: build yuma package
        DEVELOPER=1: build yuma-dev package
        DOC=1: build  yuma-doc package
    * add HTML versions of manuals to install process
    * move PDFs from /usr/share/doc/yuma to /usr/share/doc/yuma/pdf
    * updated user manuals

  * YANG parse: 
    * fix error message for leafref-stmt
    * fix bug 3404231
      * incorrect handling of object type when checking
        leaf/leaf-list leafref loops, which could cause
        the wrong struct in a union to be used
    * fixed bug 3404239
      All YANG data-def constructs were being checked for
      config, default, mandatory, min/max-elements
      before refine-stmts were applied.
      Moved all relevant tests from 'resolve' phase to 
      'resolve_final' phase.
    * Added modules/test/fail/t13.yang test case for compiler
      to check mandatory+default combo after refine applied.
    * fixed bug: anyxml objects were not getting checked during
      resolve_final phase (mandatory-stmt warnings)
    * suppress error messages when invalid XPath detected during
      validation of a union datatype.  Unions are only required
      to be valid for 1 of N union types, not any particular type.

* Sun Aug 21 2011 Andy Bierman <andy at netconfcentral.org> 2.0-2 [1325]
 * yangcli
  * fixed bug where --batchmode is ignored if --run-command
    is also used
  * added support to connect to tailf confd servers over TCP;
    added --transport=ssh|tcp parameter to connect command
    and CLI parameter for startup connecting via TCP
  * Fix potential double calls to free and memory leaks resulting from
    calls to set_str(). In some paths the function set_str()
  * fixed bugs in autoload procedure

 * netconfd
  * fixed 2 framing bugs in base:1.1 mode
  * rewrote buffer code to pack incoming message buffers instead
    of using client buffer size as-is
  * fixed memory leak in new support code for malformed-message
    only occured when malformed-message error generated
  * Improve logging for debug purposes from netconf-subsys.c
    (by Mark Pashley)
  * Many bugfixes and dead code removal detected by Coverity
    static analysis (from vi-cov branch by Mark Pashley)
  * Removed potential memory leak in cache_data_rules in NACM
  * Summary of bugfixes to copy_config_validate():
    Coverity reported the following issues:
      DEAD CODE
      Code with no effect.
      Use after free
      Null pointer derference
      Resource Leaks
  * sprintf changed to snprintf and strcpy changed to strncpy
    in some cases, to make sure no buffer overrun can occur
  * add module yuma-time-filter.yang
  * add last-modified XML attribute to <rpc-reply> for <get>
    and <get-config> replies
  * add if-modified-since parameter to <get> and <get-config>
    protocol operations
  * make logging from netconf-subsystem configurable via command line options
  * updated netconfd user manual

 * yangdump
  * fix bug in format=html or format=yang where pattern may
    not get generated in the output
  * add support for path links in leafrefs in --format=html

 * YANG parse:
  * fixed bug where val_clone of enum sometimes had static enu.name
    pointing at old.enu.dname so if old was freed, new.enu.name
    would point at garbage in the heap
  * fixed some memory leaks in error corner-cases
  * fixed bug where valid patterns parsed as non-strings
    were not correctly processed and no compiled pattern
    was created
  * fixed bug where unquoted prefixed string (foo:bar) would
    not be saved correctly in the compiled pattern (bar)

 * XML parse:
  * add tracefile support to debug input fed to XML textReader

 * CLI:
  * Change the signature of all instances of main to meet the 'c'
    standard.

* Thu Jul 21 2011 Andy Bierman <andy at netconfcentral.org> 2.0-1 [1253]
  * initial 2.0 release
    * contains all yuma 1.15 features, plus major features
	* NETCONF base:1.1 support (RFC 6241 and RFC 6242)
	* with-defaults 'report-all-tagged' mode (RFC 6243)
	* --urltarget path selection mechanism (UrlPath)

