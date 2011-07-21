Name:           yuma
Version:        2.0
Release:        0%{?dist}
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
make STATIC=1 RELEASE=0 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id STATIC=1 RELEASE=0 DESTDIR=$RPM_BUILD_ROOT

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
* Thu July 21 2011 Andy Bierman <andy at netconfcentral.org> 2.0-0 [1245]
  * initial 2.0 release
    * contains all yuma 1.15 features, plus major features
	* NETCONF base:1.1 support (RFC 6241 and RFC 6242)
	* with-defaults 'report-all-tagged' mode (RFC 6243)
	* --urltarget path selection mechanism (UrlPath)

