#
# "$Id: fltk.spec 8500 2011-03-03 09:20:46Z bgbnbigben $"
#
# RPM spec file for FLTK.
#
# Copyright 1998-2003 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to the following page:
#
#    http://www.fltk.org/str.php
#

%define version 2.0a0
%define release 0
%define prefix /usr

Summary: Fast Light Tool Kit (FLTK)
Name: fltk
Version: %{version}
Release: %{release}
Copyright: LGPL
Group: System Environment/Libraries
Source: ftp://ftp.fltk.org/pub/fltk/%{version}/fltk-%{version}-source.tar.gz
URL: http://www.fltk.org
Packager: Michael Sweet <mike@easysw.com>
# use BuildRoot so as not to disturb the version already installed
BuildRoot: /var/tmp/fltk-%{PACKAGE_VERSION}

%description
The Fast Light Tool Kit ("FLTK", pronounced "fulltick") is a
cross-platform C++ GUI toolkit for UNIX(r)/Linux(r) (X11),
Microsoft(r) Windows(r), and MacOS(r) X.  FLTK provides modern
GUI functionality without the bloat and supports 3D graphics via
OpenGL(r) and its built-in GLUT emulation.

%package devel
Summary: FLTK - development environment
Group: Development/Libraries

%description devel
Install fltk-devel if you need to develop FLTK applications. 
You'll need to install the fltk package if you plan to run
dynamically linked applications.

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" LDFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix} --mandir=%{_mandir} --enable-shared --enable-xft --enable-xdbe

# If we got this far, all prerequisite libraries must be here.
make

%install
# these lines just make sure the directory structure in the
# RPM_BUILD_ROOT exists
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT

make -e prefix=$RPM_BUILD_ROOT/%{prefix} mandir=$RPM_BUILD_ROOT/%{_mandir} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%dir %{prefix}/lib
%{prefix}/lib/libfltk2*.so.*

%files devel
%defattr(-,root,root)
%dir %{prefix}/bin
%{prefix}/bin/*
%dir %{prefix}/include/fltk
%{prefix}/include/fltk/*
%dir %{prefix}/lib
%{prefix}/lib/libfltk2*.so
%{prefix}/lib/libfltk2*.a
%dir %{_mandir}
%{_mandir}/*
%dir %{prefix}/share/doc/fltk
%{prefix}/share/doc/fltk/*

#
# End of "$Id: fltk.spec 8500 2011-03-03 09:20:46Z bgbnbigben $".
#
