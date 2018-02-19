//
// "$Id: filename_list.cxx 8507 2011-03-06 10:18:05Z bgbnbigben $"
//
// Filename list routines for the Fast Light Tool Kit (FLTK).
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

// Wrapper for scandir with a prototype that matches on all systems and
// a "user friendly" sorting function that puts numbered files into the
// order the user expects.

#include <config.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <fltk/string.h>
#include <fltk/utf.h>
#include <fltk/filename.h>

#if ! HAVE_SCANDIR
extern "C" {
int scandir (const char *dir, dirent ***namelist,
	 int (*select)(dirent *),
	 int (*compar)(const dirent*const*, const dirent*const*));
}
#endif

int fltk::alphasort(const dirent*const*a, const dirent*const*b) {
  return strcmp((*a)->d_name, (*b)->d_name);
}

int fltk::casealphasort(const dirent*const*a, const dirent*const*b) {
  return strcasecmp((*a)->d_name, (*b)->d_name);
}

int fltk::filename_list(const char *d, dirent ***list,
                     FileSortF *sort) {
  // Nobody defines the comparison function prototype correctly!
  // It should be "const dirent* const*". I don't seem to be able to
  // do this even for our own internal version because some compilers
  // will not cast it to the non-const version! Egad. So we have to
  // use if's to go to what the various systems use:
#if !HAVE_SCANDIR
  // This version is when we define our own scandir (WIN32 and perhaps
  // some Unix systems):
  int n = scandir(d, list, 0, sort);
#elif defined(HAVE_SCANDIR_POSIX)
  // POSIX (2008) defines the comparison function like this:
  int n = scandir(d, list, 0, (int(*)(const dirent **, const dirent **))sort);
#elif defined(__linux) || defined (__FreeBSD__) || defined (__NetBSD__)
  int n = scandir(d, list, 0, (int(*)(const void*,const void*))sort);
#elif defined(__osf__) || defined(__sgi)
  // OSF, DU 4.0x
  int n = scandir(d, list, 0, (int(*)(dirent **, dirent **))sort);
#elif defined(_AIX)
  // AIX is almost standard...
  int n = scandir(d, list, 0, (int(*)(void*, void*))sort);
#else
  // The vast majority of UNIX systems want the sort function to have this
  // prototype, most likely so that it can be passed to qsort without any
  // changes:
  int n = scandir(d, list, 0, (int(*)(const void*,const void*))sort);
#endif

  return n;
}

//
// End of "$Id: filename_list.cxx 8507 2011-03-06 10:18:05Z bgbnbigben $".
//
