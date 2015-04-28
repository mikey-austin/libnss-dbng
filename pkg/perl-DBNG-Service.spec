Name:           perl-DBNG-Service
Version:        0.01
Release:        1%{?dist}
Summary:        DBNG::Service Perl module
License:        CHECK(Distributable)
Group:          Development/Libraries
URL:            http://search.cpan.org/dist/DBNG-Service/
Source0:        http://www.cpan.org/modules/by-module/DBNG/DBNG-Service-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      x86_64
BuildRequires:  perl(ExtUtils::MakeMaker) libnss-dbng
Requires:       perl(:MODULE_COMPAT_%(eval "`%{__perl} -V:version`"; echo $version)) libnss-dbng

%description
DBNG::Service Perl module providing PERL OO interface for passwd, group & shadow services:

  - DBNG::Service::Passwd
  - DBNG::Service::Group
  - DBNG::Service::Shadow

%prep
%setup -q -n DBNG-Service-%{version}

%build
%{__perl} Makefile.PL INSTALLDIRS=vendor
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make pure_install PERL_INSTALL_ROOT=$RPM_BUILD_ROOT
find $RPM_BUILD_ROOT -type f -name .packlist -exec rm -f {} \;
find $RPM_BUILD_ROOT -depth -type d -exec rmdir {} 2>/dev/null \;

%{_fixperms} $RPM_BUILD_ROOT/*

%check
rm -rf /tmp/dbng-test
mkdir -p /tmp/dbng-test
TEST_BASE="/tmp/dbng-test" make test

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc META.json
%{perl_vendorarch}/*
