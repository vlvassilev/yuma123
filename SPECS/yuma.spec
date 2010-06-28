Name:           yuma
Version:        1.13
Release:        1%{?dist}
Summary:        YANG-based Unified Modular Automation Tools

Group:          Development/Tools
License:        IWL
URL:            http://yuma.iwl.com/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: ncurses
Requires: libssh2
Requires: libxml2

%description
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.
The yangdump and yangdiff development tools are also
included, to compile and process YANG modules.

%prep
%setup -q

%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
make STATIC=1 RELEASE=1 %{?_smp_mflags}
make STATIC=1 DEVELOPER=1 RELEASE=1 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id STATIC=1 RELEASE=1 \
DESTDIR=$RPM_BUILD_ROOT
make install LDFLAGS+=--build-id STATIC=1 RELEASE=1 DEVELOPER=1 \
DESTDIR=$RPM_BUILD_ROOT

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
/usr/share/doc/yuma/yumatools-cs-license.pdf
/usr/share/doc/yuma/yumatools-legal-notices.pdf
/usr/share/doc/yuma/AUTHORS
/usr/share/doc/yuma/README
/usr/share/doc/yuma/yuma-user-cmn-manual.pdf
/usr/share/doc/yuma/yuma-yangcli-manual.pdf
/usr/share/doc/yuma/yuma-yangdiff-manual.pdf
/usr/share/doc/yuma/yuma-yangdump-manual.pdf
/usr/share/doc/yuma/yuma-installation-guide.pdf
/usr/share/doc/yuma/yuma-quickstart-guide.pdf
/usr/share/doc/yuma/yuma-netconfd-manual.pdf
%{_mandir}/man1/yangcli.1.gz
%{_mandir}/man1/yangdiff.1.gz
%{_mandir}/man1/yangdump.1.gz
%{_mandir}/man1/netconfd.1.gz
%{_mandir}/man1/netconf-subsystem.1.gz
%{_datadir}/yuma/modules/*
%{_libdir}/yuma/

%package dev

Summary:  YANG-based Unified Modular Automation Tools (developer)
Requires: yuma

%description dev
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  This package contains H files, scripts,
and other files needed to create SIL code for use with
the netconfd server.

%files dev
%defattr(-,root,root,-)
%{_bindir}/make_sil_dir
%{_bindir}/yangdumpcode
%{_sysconfdir}/yuma/yangdumpcode-sample.conf
/usr/share/doc/yuma/yumatools-dev-license.pdf
/usr/share/doc/yuma/yumatools-legal-notices.pdf
/usr/share/doc/yuma/yuma-dev-manual.pdf
%{_mandir}/man1/yangdumpcode.1.gz
%{_mandir}/man1/make_sil_dir.1.gz
%{_includedir}/yuma/
%{_datadir}/yuma/util/
%{_datadir}/yuma/src/libtoaster/

%post
echo "Yuma Tools installed."
echo "Check the user manuals in /usr/share/doc/yuma"

%changelog
* Mon Jun 28 2010 Andy Bierman <andy at iwl.com> 1.13-1
  * Added partial lock support (RFC 57517)
  * Added partial-lock monitoring support
  * Bugfixes in XPath validation, config filtering,
    H file code generation
  * Static link of obscure libraries on debian build
  * Static link of libncx to allow install without root priv
  * Single package instead of 3 packages for tools
* Tue Jun 01 2010 Andy Bierman <andy at iwl.com> 1.12-2
  * Changed numbering to align with debian standards
  * Fixed bugs in yangcli:
     * tab completion on complex types
     * parse-def bug sometimes ignored XML prefix
     * improved ambiguous command error handling
  * Fixed yangdump HTML generation for references,
    sometimes caused invalid xHTML output
  * Fixed bugs in yangdiff causing incorrect diff results
  * Added YANG usage statistical reporting to yangdump
  * Fixed parser bug incorrectly treating list in a
    grouping from another module as an error
  * Updated YANG modules in netconfd:
      * ietf-netconf-with-defaults
      * ietf-netconf-monitoring
      * ietf-netconf
      * ietf-inet-types
      * ietf-yang-types
      * yuma-proc
  * Made all code C++ safe for yangui project
* Fri May 14 2010 Andy Bierman <andy at iwl.com> 0.12-1
  * Added :url capability support to netconfd
  * Added if, elif, else, eval, end, while, log-*
	commands to yangcli
  * Supporting yang-12 draft
  * Supporting yang-types-09 draft
  * Supporting ietf-netconf-with-defaults-07 draft
  * Added 'user' valriable to XPath
  * Added module-loaded and feature-enabled functions
    to XPath function library
  * Fixed bugs in sget, sget-config commands in yangcli
  * Fixed module search order bug that favored plain YANG
    file names over names with revision dates in them,
    and favored YANG over YIN files from a later directory
    in the search path.
  * yangcli now limits remote NETCONF operations based
    on the capabilities reported by the server
* Fri Apr 02 2010 Andy Bierman <andy at iwl.com> 0.11-2
  * Added 'stream_output' boolean to session hdr to disable
    server output streaming, if desired
  * Updated ietf-yang-types and ietf-netconf-with-defaults
    modules
  * Split yangdump SIL code generation out into yangdumpcode
  * Fixed packaging bug that put some YANG modules in the
    wrong package
  * Fixed bug in yangcli autoload feature
* Thu Mar 04 2010 Andy Bierman <andy at iwl.com> 0.11-1
  * Align with YANG draft-11
  * Changed default startup-cfg.xml creation path so
    the current directory is not used.
*  Mon Feb 22 2010  Andy Bierman <andyb at iwl.com> 0.10-2
 - Supporting new ietf-yang-types and ietf-netconf-monitoring 
   modules
 - Updated yuma-nacm module to match I-D version
 - Fixed bug in netconfd <error-path> for unknown nodes
*  Mon Feb 01 2010  Andy Bierman <andyb at iwl.com> 0.10-1
 - Supporting yang-10 draft
 - Add support for revision in module param (foo@2010-01-15)
 - Add feature CLI parms to control feature code generation
 - Fixed netconf-state bug; capabilities were not dynamic;
   changed capsval to a virtual node w/ callback
* Fri Jan 29 2010 Andy Bierman <andyb at iwl.com> 0.9.8.646
  - Align with yang-draft-10; add some bugfixes
* Sun Jan 17 2010 Andy Bierman <andyb at iwl.com> 0.9.8.636
  - First RPM build



