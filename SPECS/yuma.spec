Name:           yuma
Version:        0.9.8
Release:        1%{?dist}
Summary:        Yang-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.com/
Source0:        yuma-0.9.8.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: libc
Requires: libdl
Requires: libncurses
Requires: libssh2
Requires: libxml2

%description
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central stack, based directly on YANG statements.
The yangcli client provides a CLI-like interface
for any NETCONF server that supports YANG modules.
The yangdump and yangdiff development tools are also
included, to process YANG modules offline.

%prep
%setup -q


%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
mkdir -p $RPM_BUILD_ROOT/usr/local/lib
make FREE=1 %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id FREE=1 DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc /usr/share/doc/yuma/
%{_bindir}/*
%{_sbindir}/*
/usr/local/sbin/netconf-subsystem
%{_datadir}/%{name}/
%{_libdir}/libncx.so


%changelog
* Fri Nov 27 2009 Andy Bierman <andy at netconfcentral.com> 0.9.8.549
- First RPM build
