Name:           yuma-server
Version:        0.9.9
Release:        1%{?dist}
Summary:        Yang-based Unified Modular Automation Tools

Group:          Development/Tools
License:        BSD
URL:            http://yuma.iwl.com/
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Requires: openssh
Requires: libxml2

%description
Yuma Tools is a YANG-based NETCONF-over-SSH client and server
development toolkit.  The netconfd server includes an automated
central NETCONF protocol stack, based directly on YANG modules.

%prep
%setup -q

%build
make FREE=1 SERVER=1 RELEASE=1 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install LDFLAGS+=--build-id FREE=1 SERVER=1 RELEASE=1 \
DESTDIR=$RPM_BUILD_ROOT

%post
ldconfig /usr/lib/libncx.so
if [ "`grep netconf-subsystem /etc/ssh/sshd_config -c`" == "0" ]; then \
    echo "Port 22" >> /etc/ssh/sshd_config;\
    echo "Port 830" >> /etc/ssh/sshd_config;\
    echo "Subsystem netconf /usr/sbin/netconf-subsystem" >> /etc/ssh/sshd_config;\
fi
echo "Yuma server: netconfd and netconf-subsystem installed"
echo "Check the user manuals in /etc/share/doc/yuma"
echo "or type 'man netconfd' for instructions on usage."

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc /usr/share/doc/yuma/
%{_sbindir}/netconfd
%{_sbindir}/netconf-subsystem
%{_datadir}/yuma/
/etc/yuma/*
%{_libdir}/*
%{_mandir}/*

%changelog
* Fri Jan 15 2010 Andy Bierman <andyb at iwl.com> 0.9.8.622
- First RPM build
