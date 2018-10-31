Name:           %{project_name}
Version:        %{our_version}
Release:        %{?our_release}%{!?our_release:1}%{?dist}
Summary:        FRED - ORBit2 connection apache module
Group:          Applications/Utils
License:        GPL
URL:            http://fred.nic.cz
Source:         %{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  git, gcc, apr-devel, httpd-devel, ORBit2-devel, doxygen
%if 0%{?centos}
BuildRequires: centos-release-scl, llvm-toolset-7-cmake, llvm-toolset-7-build
%else
BuildRequires: cmake
%endif
Requires: httpd, ORBit2

%description
FRED (Free Registry for Enum and Domain) is free registry system for 
managing domain registrations. Part of FRED (see fred-mod-eppd and 
fred-mod-whoisd) is consists of apache modules. These modules communicate 
through CORBA technology with backend servers. Common part of this modules 
which deals with ORBit2 background id hidden in this fred-mod-corba module.

%prep
%setup

%build
%if 0%{?centos}
%{?scl:scl enable llvm-toolset-7 - << \EOF}
%global __cmake /opt/rh/llvm-toolset-7/root/usr/bin/cmake
%endif
%cmake -DCMAKE_INSTALL_PREFIX=/ -DDISTRO_PREFIX=/usr/ -DVERSION=%{version} -DIDL_DIR=%{_topdir}/BUILD/idl-%{idl_branch}/idl .
%make_build
%if 0%{?centos}
%{?scl:EOF}
%endif

%install
rm -rf ${RPM_BUILD_ROOT}
%make_install
find ${RPM_BUILD_ROOT}/usr/share/doc/ | cut -c$(echo -n "${RPM_BUILD_ROOT} " | wc -c)- > INSTALLED_FILES

%post
test -f /etc/httpd/conf.d/01-fred-mod-corba-apache.conf || ln -s /usr/share/fred-mod-corba/01-fred-mod-corba-apache.conf /etc/httpd/conf.d/
/usr/sbin/sestatus | grep -q "SELinux status:.*disabled" || setsebool -P httpd_can_network_connect=on

%preun
test ! -f /etc/httpd/conf.d/01-fred-mod-corba-apache.conf || rm /etc/httpd/conf.d/01-fred-mod-corba-apache.conf

%clean
rm -rf ${RPM_BUILD_ROOT}

%files -f INSTALLED_FILES
%defattr(-,root,root,-)
%{_libdir}/httpd/modules/mod_corba.so
/usr/share/fred-mod-corba/01-fred-mod-corba-apache.conf

%changelog
* Sat Jan 12 2008 Jaromir Talir <jaromir.talir@nic.cz>
- initial spec file

