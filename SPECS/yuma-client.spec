Name:           yuma-client
Version:        0.9.9
Release:        1%{?dist}
Summary:        Yuma Tools (client side only)

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.org/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: libc
Requires: libncurses
Requires: libssh2
Requires: libxml2

%description
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
make FREE=1 CLIENT=1 %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id FREE=1 CLIENT=1 DESTDIR=$RPM_BUILD_ROOT

%post
ldconfig /usr/lib/libncx.so

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc /usr/share/doc/yuma/
%{_bindir}/yangcli
%{_bindir}/yangdump
%{_bindir}/yangdiff
%{_datadir}/yuma/
%{_libdir}/*
%{_libdir}/yuma/*


%changelog
* Wed Jan 6 2010 Andy Bierman <andy at netconfcentral.org> 0.9.8.601
- First RPM build
