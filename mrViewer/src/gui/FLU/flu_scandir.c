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


/* Copyright (C) 1992, 1993, 1994, 1995, 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA.  */

#if defined(WIN32) && !defined(__CYGWIN__)
#  include "flu_scandir_win32.c"
#else

#  include "flustring.h"

#  if !HAVE_SCANDIR
#    include <stdlib.h>
#    include <sys/types.h>
#    include <errno.h>

#    if HAVE_DIRENT_H
#      include <dirent.h>
#      define NAMLEN(dirent) strlen((dirent)->d_name)
#    else
#      define dirent direct
#      define NAMLEN(dirent) (dirent)->d_namlen
#      if HAVE_SYS_NDIR_H
#        include <sys/ndir.h>
#      endif
#      if HAVE_SYS_DIR_H
#        include <sys/dir.h>
#      endif
#      if HAVE_NDIR_H
#        include <ndir.h>
#      endif
#    endif

int
flu_scandir(const char *dir, struct dirent ***namelist,
	   int (*select)(struct dirent *),
	   int (*compar)(struct dirent **, struct dirent **))
{
  DIR *dp = opendir (dir);
  struct dirent **v = NULL;
  size_t vsize = 0, i;
  struct dirent *d;
  int save;

  if (dp == NULL)
    return -1;

  save = errno;
  errno = 0;

  i = 0;
  while ((d = readdir (dp)) != NULL)
    if (select == NULL || (*select) (d))
      {
      size_t dsize;

      if (i == vsize)
        {
          struct dirent **newv;
          if (vsize == 0)
            vsize = 10;
          else
            vsize *= 2;
          newv = (struct dirent **) realloc (v, vsize * sizeof (*v));
          if (newv == NULL)
            {
            lose:
              errno = ENOMEM;
              break;
            }
          v = newv;
        }

#    define _D_EXACT_NAMLEN(d) (strlen ((d)->d_name))
#    define _D_ALLOC_NAMLEN(d) (sizeof (d)->d_name > 1 ? sizeof (d)->d_name : \
                              _D_EXACT_NAMLEN (d) + 1)

      dsize = &d->d_name[_D_ALLOC_NAMLEN (d)] - (char *) d;
      v[i] = (struct dirent *) malloc (dsize);
      if (v[i] == NULL)
        goto lose;

      memcpy (v[i++], d, dsize);
      }

  if (errno != 0)
    {
      save = errno;
      (void) closedir (dp);
      while (i > 0)
      free (v[--i]);
      free (v);
      errno = save;
      return -1;
    }

  (void) closedir (dp);
  errno = save;

  /* Sort the list if we have a comparison function to sort with.  */
  if (compar) qsort (v, i, sizeof (*v), (int (*)(const void *, const void *))compar);
  *namelist = v;
  return i;
}

#  endif
#endif

/*
 * End of "$Id$".
 */
