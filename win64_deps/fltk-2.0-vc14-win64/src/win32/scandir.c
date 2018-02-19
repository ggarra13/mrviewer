// "$Id: scandir.c 1339 2006-04-03 22:47:29Z spitzak $"
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

// Emulation of posix scandir() call
// This source file is #include'd by scandir.c
// THIS IS A C FILE! DO NOT CHANGE TO C++!!!
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <fltk/utf.h>

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
#else
struct dirent { char d_name[1]; };
#endif

int scandir(const char *dirname, struct dirent ***namelist,
    int (*select)(struct dirent *),
    int (*compar)(struct dirent **, struct dirent **)) {
  WIN32_FIND_DATAW find;
  HANDLE h;
  int nDir = 0, NDir = 0, len;
  struct dirent **dir = 0, *selectDir;
  unsigned long ret;
  char *temp;
  unsigned widelen;
  unsigned short* widebuff = NULL, *d;
  
  len = widelen = strlen(dirname);

  // If the filename is already encoded into utf8,
  // We needn't (and shouldn't) bother about converting it into utf8 again!
  if (!utf8test(dirname, len)) {
    widelen = utf8frommb(NULL, 0, dirname, len);
    temp = (char*)malloc(sizeof(char)*widelen+11);
    utf8frommb(temp, widelen+1, dirname, len+1);
  } else {
    temp = (char*)malloc(sizeof(char)*widelen+11);
    strcpy(temp, dirname);
  }
  // Change the filename to a wchar_t* representation
  // so we can read filenames in any charset (if they exist).
  // Windows needs this to be a wchar_t to deal with its stupid UTF16 format.
  widelen = utf8towc(temp, widelen, NULL, 0);
  widebuff = (unsigned short*)calloc(widelen+15, sizeof(unsigned short));
  utf8towc(temp, strlen(temp), widebuff, widelen+5);
  
  d = widebuff+wcslen(widebuff);
  if (d==widebuff) *d++ = '.';
  if (*(d-1)!='/' && *(d-1)!='\\') *d++ = '/';
  *d++ = '*';

  if ((h=FindFirstFileW(widebuff, &find))==INVALID_HANDLE_VALUE) {
    ret = GetLastError();
    if (ret != ERROR_NO_MORE_FILES) {
      // TODO: return some error code
    }
    *namelist = dir;
    return nDir;
  }
  do {
    selectDir=(struct dirent*)malloc(sizeof(struct dirent)+wcslen(find.cFileName)*5+1);
	// convert the filename back to UTF-8, as this is what FLTK uses internally
	utf8fromwc(selectDir->d_name, sizeof(struct dirent)+wcslen(find.cFileName)*5+1, find.cFileName, wcslen(find.cFileName));
    if (!select || (*select)(selectDir)) {
      if (nDir==NDir) {
	struct dirent **tempDir = (struct dirent **)calloc(sizeof(struct dirent*), NDir+33);
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
  } while (FindNextFileW(h, &find));
  ret = GetLastError();
  if (ret != ERROR_NO_MORE_FILES) {
    // TODO: return some error code
  }
  FindClose(h);
  free(temp);
  free(widebuff);

  if (compar) qsort (dir, nDir, sizeof(*dir),
		     (int(*)(const void*, const void*))compar);

  *namelist = dir;
  return nDir;
}

#ifdef __cplusplus
}
#endif

//
// End of "$Id: scandir.c 1339 2006-04-03 22:47:29Z spitzak $".
//
