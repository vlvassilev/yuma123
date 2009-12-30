Name:           yuma
Version:        0.9.9
Release:        1%{?dist}
Summary:        Yang-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.org/
Source0:        yuma-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: libc
Requires: libdl
Requires: libncurses
Requires: libssh2
Requires: libxml2

%description
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.
The yangcli client provides a CLI-like interface
for any NETCONF server that supports YANG modules.
The yangdump and yangdiff development tools are also
included, to compile and process YANG modules.

%prep
%setup -q


%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
make FREE=1 %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id FREE=1 DESTDIR=$RPM_BUILD_ROOT

%post
ldconfig /usr/lib/yuma/libncx.so

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
%{_libdir}/yuma/*


%changelog
* Fri Nov 27 2009 Andy Bierman <andy at netconfcentral.org> 0.9.8.571
- First RPM build
