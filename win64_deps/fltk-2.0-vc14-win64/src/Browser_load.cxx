//
// "$Id: Browser_load.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
//
// File loading routines for the Fast Light Tool Kit (FLTK).
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

#include <fltk/events.h>
#include <fltk/Browser.h>
#include <stdio.h>

using namespace fltk;

/** Adds the contents of a file to a browser, splitting at newlines.
    This is useful if the browser was storing items that should be saved
    on program exit and reloaded next time the program is started

    \param filename The name of the file to load
    \return 0 if the file couldn't be opened, -1 if filename is NULL or
    contains no text and 1 otherwise
*/
int Browser::load(const char *filename) {
#define MAXBLINE 1024
  char newtext[MAXBLINE];
  int c;
  int i;
  clear();
  if (!filename || !(filename[0])) return -1;
  FILE *fl = fopen(filename,"r");
  if (!fl) return 0;
  i = 0;
  do {
    c = getc(fl);
    if (c == '\n' || c <= 0 || i>=(MAXBLINE-1)) {
      newtext[i] = 0;
      add(newtext);
      i = 0;
    } else newtext[i++] = c;
  } while (c >= 0);
  fclose(fl);
  return 1;
}

//
// End of "$Id: Browser_load.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
