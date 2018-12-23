//
// "$Id"
//
// fluid_img declaration: all fluid xpm's into one table
//
// Type classes for most of the fltk widgets.  Most of the work
// is done by code in WidgetType.C.  Also a factory instance
// of each of these type classes.
//
// This file also contains the "new" menu, which has a pointer
// to a factory instance for every class (both the ones defined
// here and ones in other files)
//
// Copyright 1998-2006 by Bill Spitzak and others.
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
#ifndef fltk_fluid_img_h
#define fltk_fluid_img_h

#include <fltk/xpmImage.h>

extern fltk::xpmImage*	fluid_pixmap[];
extern fltk::xpmImage	open_pixmap;
extern fltk::xpmImage	new_pixmap;
extern fltk::xpmImage	save_pixmap;
extern fltk::xpmImage	saveas_pixmap;
extern fltk::xpmImage	merge_pixmap;
extern fltk::xpmImage	write_pixmap;
extern fltk::xpmImage	exit_pixmap;

#endif

// End of "$Id"
