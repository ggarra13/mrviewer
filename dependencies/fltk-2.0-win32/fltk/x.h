// "$Id: x.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/** \file
  "Portably" include either x11.h, win32.h, or osx.h header files,
  depending on your system. Since the system-specific interfaces
  resemble each other somewhat, use of this header and some macros
  may allow you to merge system-specific code for different systems
  together.
*/

#ifndef fltk_x_h
# define fltk_x_h
# if defined(_WIN32) && !USE_X11
#  include "win32.h"
# elif defined(__APPLE__) && !USE_X11
#  include "osx.h"
# else
#  define USE_X11 1
#  include "x11.h"
# endif
#endif

#if USE_CAIRO
# include "fltk_cairo.h"
#endif 
//
// End of "$Id: x.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
