Name:           yuma
Version:        2.0
Release:        2%{?dist}
Summary:        YANG-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.org/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: ncurses
Requires: libxml2

%description
Yuma is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.
The yangcli client supports single sessions over SSH with some
script support.  The yangdump and yangdiff development tools are also
included, to compile and process YANG modules.

%define debug_package %{nil}

%prep
%setup -q

%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
make STATIC=1 LIB64=1 RELEASE=2 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id STATIC=1 LIB64=1 RELEASE=1 DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_bindir}/yangcli
%{_bindir}/yangdump
%{_bindir}/yangdiff
%{_bindir}/make_sil_dir
%{_sbindir}/netconfd
%{_sbindir}/netconf-subsystem
%{_sysconfdir}/yuma/yangcli-sample.conf
%{_sysconfdir}/yuma/yangdiff-sample.conf
%{_sysconfdir}/yuma/yangdump-sample.conf
%{_sysconfdir}/yuma/netconfd-sample.conf
/usr/share/doc/yuma/yuma-legal-notices.pdf
/usr/share/doc/yuma/AUTHORS
/usr/share/doc/yuma/README
/usr/share/doc/yuma/yuma-user-cmn-manual.pdf
/usr/share/doc/yuma/yuma-yangcli-manual.pdf
/usr/share/doc/yuma/yuma-yangdiff-manual.pdf
/usr/share/doc/yuma/yuma-yangdump-manual.pdf
/usr/share/doc/yuma/yuma-installation-guide.pdf
/usr/share/doc/yuma/yuma-quickstart-guide.pdf
/usr/share/doc/yuma/yuma-netconfd-manual.pdf
/usr/share/doc/yuma/yuma-dev-manual.pdf
/usr/share/doc/yuma/server-call-chain.txt
%{_mandir}/man1/yangcli.1.gz
%{_mandir}/man1/yangdiff.1.gz
%{_mandir}/man1/yangdump.1.gz
%{_mandir}/man1/netconfd.1.gz
%{_mandir}/man1/netconf-subsystem.1.gz
%{_mandir}/man1/make_sil_dir.1.gz
%{_datadir}/yuma/modules/*
%{_libdir}/yuma/
%{_includedir}/yuma/
%{_datadir}/yuma/util/
%{_datadir}/yuma/src/libtoaster/

%post
echo "Yuma installed."
echo "Check the user manuals in /usr/share/doc/yuma"

%changelog
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
* Thu July 21 2011 Andy Bierman <andy at netconfcentral.org> 2.0-1 [1248]
  * initial 2.0 release
    * contains all yuma 1.15 features, plus major features
	* NETCONF base:1.1 support (RFC 6241 and RFC 6242)
	* with-defaults 'report-all-tagged' mode (RFC 6243)
	* --urltarget path selection mechanism (UrlPath)
