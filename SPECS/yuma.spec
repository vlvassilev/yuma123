Name:           yuma
Version:        1.14
Release:        3%{?dist}
Summary:        YANG-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.org/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: ncurses
Requires: libxml2

%description
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.
The yangcli client supports single sessions over SSH with some
script support.  The yangdump and yangdiff development tools are also
included, to compile and process YANG modules.

%prep
%setup -q

%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
make STATIC=1 RELEASE=2 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id STATIC=1 RELEASE=2 DESTDIR=$RPM_BUILD_ROOT

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
echo "Yuma Tools installed."
echo "Check the user manuals in /usr/share/doc/yuma"

%changelog
* Sat Dec 18 2010 Andy Bierman <andy at netconfcentral.org> 1.14-3 [983]
  * yangcli: fixed bug in var display for boolean type
  * fixed bug in sprintf val to buffer for empty string
  * changed warn-linelen default to 0 to turn warning off by default
  * fix YANG parsing error for out-of-order nested uses expansion
    see test/pass/test9.yang
  * add -tree-identifiers option to yangcli to print in tree format
  * add some ifdef wrappers around debug statements
  * add more YANG parsing debug trace statements in debug4
  * started new branch for conversion to autoconf
  * yangcli: fix bug not checking if edit target is config=false
  * yangcli: fixed error message for setting config var
  * yangcli: fixed 'save' meta command for commit+copy-config
  * adding support for juniper capability encoding in hello
  * yangcli: added 'indent' system variable
  * yangcli: changed val_dump_ usage to use indent
    and display-mode settings
  * modules: changed yuma-types::IndentType default from
    3 to 2 to match implementation
  * added new error message for trying to write a read-only object
  * fixed netconfd bug where candidate not getting filled from
    running with clean editvars if with-startup=true
  * fixed netconfd bug where edit-config on running was not
    getting saved to NV-storage if with-startup=false
  * fixed bug in netconfd where continue-on-error would continue
    even if the config data was not schema valid.
    Now the validate phase must complete OK for the apply phase to exec
* Thu Nov 18 2010 Andy Bierman <andy at netconfcentral.org> 1.14-2 [969]
  * yangcli: fixed bug handling XML preamble in extern variables
  * XML: now using default namespace in XML output to reduce
    message size
  * yangdump: fixed bug generating refine-stmt sub-statements
  * yangcli: added support for Juniper incorrect base:1.0 namespace
  * added server_call_chain.txt docs to doc/extra
  * netconfd: added some error messages to agt_hello_dispatch to
    log error messages when bad <hello> found
  * modules: update some IETF YANG modules
* Sat Oct 09 2010 Andy Bierman <andy at netconfcentral.org> 1.14-1 [955]
  * added 'muntrace' call to clean up MEMTRACE=1 debug build mode
  * bumped library version to 1.14
  * improved yangcli capability processing:
    * all module namespace URIs are found in the module search path
    * capability URIs without 'module' parameter are checked against 
      the base namespace URI instead of declaring 'unknown'
  * fixed bugs in yangcli fill mode where get key was sometimes skipped
  * fixed bug in yangcli session startup not getting latest system vars
    in the session copy
  * fix bugs in compiler submodule list and XPath validation
  * added virtual node caching and made return value copy to netconfd,
    val_get_virtual_value() return not not live pointer anymore
  * fixed import module processing bug, some imports not getting
    cached, and instead parsed over again
  * fixed yangcli memory leak handling unknown (anyxml) reply content
  * added more error messages and checking to yangdump
  * updated ietf-netconf-monitoring to RFC 6022 version
  * updated ietf-yang-types and ietf-inet-types to RFC 6021 version
  * YANG compiler is now fixed to RFC 6020 version
* Sat Aug 28 2010 Andy Bierman <andy at netconfcentral.org> 1.13-6 [926]
  * fixed yangdump bugs in submodule processing
* Fri Aug 06 2010 Andy Bierman <andy at netconfcentral.org> 1.13-5
  * fix bug in make_sil_dir
* Sat Jul 17 2010 Andy Bierman <andy at netconfcentral.org> 1.13-4
  * prepare for initial sourceforge release
  * fix bugs in parsing and retention of sub-modules
    as the top-level file for yangdump and yangcli
* Fri Jul 16 2010 Andy Bierman <andy at netconfcentral.org> 1.13-3
  * Fixed yangdump bugs
    * yangdump refine-stmt validation
    * enumeration validation corrected
    * false config-mismatch error in groupings fixed
  * Added '$YUMA_INSTALL/lib' to SIL search path to support tarball install
  * Changed license from IWL to BSD
  * moved yuma project back to netconfcentral.org
  * combined yuma and yuma-dev packages
    * yangdump now generates SIL code (format=c,h,sqldb)
    * usr/include/yuma/* files installed
  * added code gen to yangdump; removed yangdumpcode
* Wed Jun 30 2010 Andy Bierman <andy at iwl.com> 1.13-2
  * Fixed bug in yangcli command prompt
  * Fixed bug in README file
  * Added support for tarball release and non-root install
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



