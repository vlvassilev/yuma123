Name:           yuma-source
Version:        0.9.9
Release:        1%{?dist}
Summary:        Yuma Tools (development version)

Group:          Development/Tools
License:        BSD
URL:            http://yuma.iwl.com/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: glibc
Requires: ncurses
Requires: libssh2
Requires: libxml2

%description
Yuma Tools (source version) is a YANG-based 
NETCONF-over-SSH server and client application,
for any YANG modules.  The complete source files
are included.  The yangdump and yangdiff development
tools are also included, to compile and process YANG modules.
Code generation is enabled in this version of yangdump.


%prep
%setup -q


%build
cd libtecla
./configure --prefix=$RPM_BUILD_ROOT 
cd ..
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id DESTDIR=$RPM_BUILD_ROOT

%post
ldconfig /usr/lib/yuma/libncx.so

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc /usr/share/doc/yuma/
%{_bindir}/*
%{_sbindir}/*
%{_datadir}/yuma/
%{_libdir}/*
%{_includedir}/yuma/
%{_mandir}/*

%changelog
* Fri Jan 15 2010 Andy Bierman <andyb at iwl.com> 0.9.8.622
- First RPM build
