%define lib_major 0

Name:		libcroco
Summary:	CSS2 parser library
Version: 	0.6.8
Release: 	1
License: 	LGPL
Group:		System/Libraries
Source0: 	ftp://ftp.gnome.org/pub/GNOME/sources/%{name}-%{version}.tar.gz
URL: 		http://savannah.nongnu.org/projects/libcroco
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
BuildRequires:	libxml2-devel
BuildRequires:  glib2-devel

%description
libcroco is a standalone css2 parsing library.
It provides a low level event driven SAC like api
and a css object model like api.

%package devel
Summary:	Libraries and include files for developing with librsvg.
Group:		Development/C
Requires:	%{name} = %{version}-%{release}
Provides:   %{name}-devel = %{version}-%{release}
Requires:	libxml2-devel
Requires:	glib2-devel

%description devel
This package provides the necessary development libraries and include
files to allow you to develop with libcroco?

%prep
%setup -q

%build

%configure

make

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
%makeinstall

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT


%post -p /sbin/ldconfig 

%postun -p /sbin/ldconfig

%files 
%defattr(-, root, root)
%doc AUTHORS COPYING COPYING.LIB ChangeLog NEWS README
%{_bindir}/csslint
%{_mandir}/man1/csslint*
%{_libdir}/*.so.*

%files  devel
%defattr(-,root,root)
%{_bindir}/croco-config
%{_mandir}/man1/croco-config*
%{_libdir}/*.la
%{_libdir}/*.a
%{_libdir}/*.so
%{_includedir}/*
%{_libdir}/pkgconfig/*

%changelog
* Tue Jul 8 2003 Christian Schaller <Uraeus@gnome.org>
- Udate for 0.3 and Red Hat

* Mon Jul  7 2003 Frederic Crozat <fcrozat@mandrakesoft.com> - 0.2.0-1mdk
- Release 0.2.0
- WARNING : binary compat is broken, you need to recompile apps built with libcroco 0.1

* Fri Jun 13 2003 G�tz Waschk <waschk@linux-mandrake.com> 0.1.0-2mdk
- rebuild for new rpm

* Fri Apr 18 2003 Frederic Crozat <fcrozat@mandrakesoft.com> - 0.1.0-1mdk
- First Mdk package.
