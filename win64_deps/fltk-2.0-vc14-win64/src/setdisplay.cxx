//
// "$Id: setdisplay.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/run.h>
#include <stdlib.h>
#include <string.h>

/*! Startup method to set what X display to use. This uses setenv()
  to change the $DISPLAY environment variable, so it will affect
  programs that are exec'd by this one.

  This does some "uglification" required by X. If there is no colon
  in the string it appends ":0.0" to it. Thus a plain machine name
  may be used.

  On non-X systems this sets the environment variable anyway, even
  though it probably will not affect the display used. It
  appears that putenv is missing on some versions of Windows so I
  commented it all out there, sigh.
*/
void fltk::display(const char *d) {
#ifndef _WIN32
  char *e = new char[strlen(d)+13];
  strcpy(e,"DISPLAY=");
  strcpy(e+8,d);
  for (char *c = e+8; *c!=':'; c++) if (!*c) {strcpy(c,":0.0"); break;}
  putenv(e);
#endif
}

//
// End of "$Id: setdisplay.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
