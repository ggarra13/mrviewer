//
// "$Id: KDE_colors.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Theme plugin file for FLTK
//
// Copyright 1999 Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

// Just colors and fonts for KDE -- no boxtypes
// This is done by passing an argument to the KDE plugin that causes
// it to skip that stuff.

// This theme is designed to be used with other themes to allow them to
// use KDE's colors.

#include <fltk/Fl.h>
#include <fltk/fl_theme.h>

extern "C" int fltk_plugin() {
  return fl_kde(1);
}

//
// End of "$Id: KDE_colors.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
