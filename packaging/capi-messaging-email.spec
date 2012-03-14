Name:       capi-messaging-email
Summary:    Email library in Tizen Native API
Version:    0.1.0
Release:    1.1
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(email-service)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-appfw-application)
Requires(post): /sbin/ldconfig  
Requires(postun): /sbin/ldconfig

%description


%package devel
Summary:  Email library in Tizen Native API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel



%prep
%setup -q


%build
FULLVER=%{version}
MAJORVER=`echo ${FULLVER} | cut -d '.' -f 1`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=${FULLVER} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/libcapi-messaging-email.so*

%files devel
%{_includedir}/messaging/*.h
%{_libdir}/pkgconfig/*.pc

