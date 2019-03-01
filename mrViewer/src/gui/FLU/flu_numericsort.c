/*
 * Taken explicitly from FLTK to avoid transferring dirent between FLU and FLTK,
 * causing filename mess when FLTK is not compiled with Large File Support
 *
 * The bugfix involves :
 * - copy/paste of fl_filename_list.c, fl_numericsort.c, fl_scandir.c, fl_scandir_win32.c,
 *   flstring.h and flstring.c
 * - creation of a flu-config.h configuration file to define some compile-time macros
 * - modification of CMakeLists.txt to generate the configuration file the same way
 *   fltk-config.h is generated
 * - renaming the function with 'flu' prefix instead of 'fl'
 * - using the renamed flu_filename_list function in Flu_File_Chooser.cpp
 *
 */

/*
 * "$Id$"
 *
 * Numeric sorting routine for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2005 by Bill Spitzak and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems on the following page:
 *
 *     http://www.fltk.org/str.php
 */

/* My own scandir sorting function, useful for the film industry where
   we have many files with numbers in their names: */

// OTB Modifications
//#include <config.h>
#include "flu-config.h"

#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>

#include <FL/filename.H>

#if !defined(WIN32) || defined(__CYGWIN__)
#  ifdef HAVE_DIRENT_H
#    include <dirent.h>
#  else
#    define dirent direct
#    if HAVE_SYS_NDIR_H
#      include <sys/ndir.h>
#    endif /* HAVE_SYS_NDIR_H */
#    if HAVE_SYS_DIR_H
#      include <sys/dir.h>
#    endif /* HAVE_SYS_DIR_H */
#    if HAVE_NDIR_H
#      include <ndir.h>
#    endif /* HAVE_NDIR_H */
#  endif /* HAVE_DIRENT_H */
#endif /* !WIN32 || __CYGWIN__ */

/*
 * 'flu_numericsort()' - Compare two directory entries, possibly with
 *                   a case-insensitive comparison...
 */

static int numericsort(struct dirent **A, struct dirent **B, int cs) {
  const char* a = (*A)->d_name;
  const char* b = (*B)->d_name;
  int ret = 0;
  for (;;) {
    if (isdigit(*a & 255) && isdigit(*b & 255)) {
      int diff,magdiff;
      while (*a == '0') a++;
      while (*b == '0') b++;
      while (isdigit(*a & 255) && *a == *b) {a++; b++;}
      diff = (isdigit(*a & 255) && isdigit(*b & 255)) ? *a - *b : 0;
      magdiff = 0;
      while (isdigit(*a & 255)) {magdiff++; a++;}
      while (isdigit(*b & 255)) {magdiff--; b++;}
      if (magdiff) {ret = magdiff; break;} /* compare # of significant digits*/
      if (diff) {ret = diff; break;}	/* compare first non-zero digit */
    } else {
      if (cs) {
      	/* compare case-sensitive */
	if ((ret = *a-*b)) break;
      } else {
	/* compare case-insensitve */
	if ((ret = tolower(*a & 255)-tolower(*b & 255))) break;
      }

      if (!*a) break;
      a++; b++;
    }
  }
  if (!ret) return 0;
  else return (ret < 0) ? -1 : 1;
}

/*
 * 'flu_casenumericsort()' - Compare directory entries with case-sensitivity.
 */

int flu_casenumericsort(struct dirent **A, struct dirent **B) {
  return numericsort(A, B, 0);
}

/*
 * 'flu_numericsort()' - Compare directory entries with case-sensitivity.
 */

int flu_numericsort(struct dirent **A, struct dirent **B) {
  return numericsort(A, B, 1);
}

/*
 * End of "$Id$".
 */
