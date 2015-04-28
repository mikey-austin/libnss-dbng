Name:      libnss-dbng
Summary:   libnss-dbng
Version:   0.5.0
Release:   1
License:   GPL v3
Group:     Applications/Internet
URL:       https://github.com/mikey-austin/libnss-dbng 

Source0:   %{name}-%{version}.tar.gz

Buildroot: ${RPM_BUILD_DIR}/%{name}-%{version}

Requires: db4
BuildRequires: db4-devel

%description
libnss-dbng - the next generation of libnss-db

%prep
%setup -q

%build
# Customize as needed.
#./configure --prefix=/usr MIN_UID=1010 MIN_GID=1010 DEFAULT_BASE=/var/lib/dbng

./configure --prefix=/usr DEFAULT_BASE=/var/lib/dbng
make

%install
rm -fr %{buildroot}
make install DESTDIR=%{buildroot}
mkdir -p %{buildroot}/var/lib/dbng
chmod 0755 %{buildroot}/var/lib/dbng
mkdir -p %{buildroot}/usr/include/dbng
mv %{buildroot}/usr/include/{,dbng/}service.h
mv %{buildroot}/usr/include/{,dbng/}service-passwd.h
mv %{buildroot}/usr/include/{,dbng/}service-group.h
mv %{buildroot}/usr/include/{,dbng/}service-shadow.h
mv %{buildroot}/usr/include/{,dbng/}dbng.h

%clean
#rm -fr %{buildroot}

%files
%defattr(-,root,root)
%define _unpackaged_files_terminate_build 0
%doc README COPYING AUTHORS
%{_mandir}/man8/*.8*
%dir %attr(0755,root,root) %{_var}/lib/dbng
/usr/include/dbng/service.h
/usr/include/dbng/service-passwd.h
/usr/include/dbng/service-group.h
/usr/include/dbng/service-shadow.h
/usr/include/dbng/dbng.h
/usr/lib/libnss_dbng.so
/usr/lib/libnss_dbng.la
/usr/lib/libnss_dbng.so.2
/usr/lib/libnss_dbng.so.2.0.0
/usr/lib/libdbng.a
/usr/lib/libnss_dbng.a
/usr/lib/libdbng.la
/usr/lib/libdbng.so
/usr/lib/libdbng.so.0.0.0
/usr/lib/libdbng.so.0
/usr/sbin/dbngctl
