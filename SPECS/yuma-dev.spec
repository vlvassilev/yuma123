Name:           yuma-dev
Version:        0.9.9
Release:        1%{?dist}
Summary:        Yuma Tools (development version)

Group:          Development/Tools
License:        BSD
URL:            http://www.netconfcentral.org/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: libc
Requires: libxml2

%description
Yuma Tools (development version) is a YANG-based 
NETCONF-over-SSH server and client application,
for any YANG modules.  The development header files
are included.  The yangdump development
tool is also included, to compile and process YANG modules.
Code generation is enabled in this version of yangdump.


%prep
%setup -q


%build
make DEVELOPER=1 %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id DEVELOPER=1 DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc /usr/share/doc/yuma/
%{_includedir}/yuma/

%changelog
* Wed Jan 6 2010 Andy Bierman <andy at netconfcentral.org> 0.9.8.601
- First RPM build
