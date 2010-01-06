Name:           yuma-server
Version:        0.9.9
Release:        1%{?dist}
Summary:        Yang-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.org/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: libc
Requires: libdl
Requires: libxml2

%description
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.

%prep
%setup -q


%build
make FREE=1 SERVER=1 %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id FREE=1 SERVER=1 DESTDIR=$RPM_BUILD_ROOT

%post
ldconfig /usr/lib/libncx.so

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc /usr/share/doc/yuma/
%{_sbindir}/netconfd
%{_sbindir}/netconf-subsystem
%{_datadir}/yuma/
%{_libdir}/*


%changelog
* Wed Jan 6 2010 Andy Bierman <andy at netconfcentral.org> 0.9.8.601
- First RPM build
