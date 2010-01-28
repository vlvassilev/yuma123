Name:           yuma
Version:        0.10
Release:        1%{?dist}
Summary:        YANG-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://yuma.iwl.com/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.
The yangdump and yangdiff development tools are also
included, to compile and process YANG modules.

%package client

Summary: YANG-based Unified Modular Automation Tools (client-side)
Group:          Development/Tools
License:        BSD

Requires: ncurses
Requires: libssh2
Requires: libxml2

%description client
Yuma Tools (client only) is a YANG-based NETCONF-over-SSH 
client application, which provides a CLI-like interface
for any NETCONF server that supports YANG modules.
The yangdump and yangdiff development tools are also
included, to compile and process YANG modules.

%prep
%setup -q

%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
make FREE=1 RELEASE=1 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id FREE=1 RELEASE=1 \
DESTDIR=$RPM_BUILD_ROOT

%post
ldconfig
echo "Yuma client: yangcli, yangdump, and yangdiff installed"
echo "Check the user manuals in /etc/share/doc/yuma"
echo "or type 'man <program name>' for instructions on usage."

%clean
rm -rf $RPM_BUILD_ROOT

%files client
%defattr(-,root,root,-)
%doc /usr/share/doc/yuma/
%{_bindir}/yangcli
%{_bindir}/yangdump
%{_bindir}/yangdiff
%{_datadir}/yuma/
/etc/yuma/yangcli-sample.conf
/etc/yuma/yangdiff-sample.conf
/etc/yuma/yangdump-sample.conf
%{_libdir}/libncx.so.1.0
%{_mandir}/man1/yangcli.1.gz
%{_mandir}/man1/yangdiff.1.gz
%{_mandir}/man1/yangdump.1.gz


%changelog
* Fri Jan 29 2010 Andy Bierman <andyb at iwl.com> 0.9.8.646
- Align with yang-draft-10; add some bugfixes
* Sun Jan 17 2010 Andy Bierman <andyb at iwl.com> 0.9.8.636
- First RPM build

%package server

Summary:  YANG-based Unified Modular Automation Tools (server-side)
Requires: openssh
Requires: libxml2
Requires: client

%description server
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.

%files server
%defattr(-,root,root,-)
%{_sbindir}/netconfd
%{_sbindir}/netconf-subsystem
/etc/yuma/netconfd-sample.conf
%{_libdir}/yuma/
%{_mandir}/man1/netconfd.1.gz
%{_mandir}/man1/netconf-subsystem.1.gz





