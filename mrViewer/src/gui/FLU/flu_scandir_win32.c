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
 * WIN32 scandir function for the Fast Light Tool Kit (FLTK).
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

#ifndef __CYGWIN__
/* Emulation of posix scandir() call */

#include <FL/filename.H>
#include "flustring.h"
#include <windows.h>
#include <stdlib.h>

int flu_scandir(const char *dirname, struct dirent ***namelist,
	       int (*select)(struct dirent *),
	       int (*compar)(struct dirent **, struct dirent **)) {
  int len;
  char *findIn, *d, is_dir = 0;
  WIN32_FIND_DATA find;
  HANDLE h;
  int nDir = 0, NDir = 0;
  struct dirent **dir = 0, *selectDir;
  unsigned long ret;

  len    = strlen(dirname);
  findIn = (char *)malloc((size_t)(len+5));

  if (!findIn) return -1;

  strcpy(findIn, dirname);
  for (d = findIn; *d; d++) if (*d=='/') *d='\\';
  if ((len==0)) { strcpy(findIn, ".\\*"); }
  if ((len==2)&&findIn[1]==':'&&isalpha(findIn[0])) { *d++ = '\\'; *d = 0; }
  if ((len==1)&& (d[-1]=='.')) { strcpy(findIn, ".\\*"); is_dir = 1; }
  if ((len>0) && (d[-1]=='\\')) { *d++ = '*'; *d = 0; is_dir = 1; }
  if ((len>1) && (d[-1]=='.') && (d[-2]=='\\')) { d[-1] = '*'; is_dir = 1; }
  if (!is_dir) { /* this file may still be a directory that we need to list */
    DWORD attr = GetFileAttributes(findIn);
    if (attr&FILE_ATTRIBUTE_DIRECTORY) 
      strcpy(d, "\\*");
  }
  if ((h=FindFirstFile(findIn, &find))==INVALID_HANDLE_VALUE) {
    free(findIn);
    ret = GetLastError();
    if (ret != ERROR_NO_MORE_FILES) {
      nDir = -1;
    }
    *namelist = dir;
    return nDir;
  }
  do {
    selectDir=(struct dirent*)malloc(sizeof(struct dirent)+strlen(find.cFileName)+2);
    strcpy(selectDir->d_name, find.cFileName);
    if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      /* Append a trailing slash to directory names... */
      strcat(selectDir->d_name, "/");
    }
    if (!select || (*select)(selectDir)) {
      if (nDir==NDir) {
	struct dirent **tempDir = (struct dirent **)calloc(sizeof(struct dirent*), (size_t)(NDir+33));
	if (NDir) memcpy(tempDir, dir, sizeof(struct dirent*)*NDir);
	if (dir) free(dir);
	dir = tempDir;
	NDir += 32;
      }
      dir[nDir] = selectDir;
      nDir++;
      dir[nDir] = 0;
    } else {
      free(selectDir);
    }
  } while (FindNextFile(h, &find));
  ret = GetLastError();
  if (ret != ERROR_NO_MORE_FILES) {
    /* don't return an error code, because the dir list may still be valid
       up to this point */ 
  }
  FindClose(h);

  free (findIn);

  if (compar) qsort(dir, (size_t)nDir, sizeof(*dir),
		    (int(*)(const void*, const void*))compar);

  *namelist = dir;
  return nDir;
}

#endif

/*
 * End of "$Id$".
 */
