Name:       capi-messaging-email
Summary:    Email library in Tizen Native API
Version:    0.1.14
Release:    3
Group:      Messaging/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(email-service)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-appfw-application)

%description
Email library in Tizen Native API.


%package devel
Summary:  Email library in Tizen Native API (Development)
Requires: %{name} = %{version}-%{release}

%description devel
%devel_desc



%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`  
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER} 

make %{?jobs:-j%jobs}

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%license LICENSE
%{_libdir}/libcapi-messaging-email.so.*

%files devel
%{_includedir}/messaging/*.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-messaging-email.so


