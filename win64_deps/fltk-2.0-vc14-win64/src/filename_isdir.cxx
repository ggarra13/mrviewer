// "$Id: filename_isdir.cxx 8507 2011-03-06 10:18:05Z bgbnbigben $"
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
// I would like to make this file PUBLIC DOMAIN. However I am not the
// original author. If the original contributor agrees, simply change this
// comment to match the filename_ext.cxx one.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php

#include <config.h>
#include <fltk/filename.h>
#include <fltk/string.h>
#include <fltk/utf.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
# include <io.h>
# include <windows.h>
#else
# include <unistd.h>
#endif

// This function makes sure that 'stat' is only called once if the users
// queries different aspects on the same file. 'stat' can be relatively slow.

static struct stat last_stat;
static const char *last_statname = 0;
static bool last_result = false;

/** Portably calls the system's stat() function, to deal with
    native Unicode filenames.
    Has the same return values and use as the system's stat.
    The string passed to fltk_stat \b must be UTF8 (note that ASCII is
    a subset of UTF8)
*/
int fltk::fltk_stat(const char* name, struct stat *buffer) {
#if defined(_WIN32) && !defined (__CYGWIN__)
  wchar_t * nativeFilename = NULL;
  int length = utf8towc(name, strlen(name), NULL, 0);
  nativeFilename = new wchar_t[length+2];
  utf8towc(name, strlen(name), nativeFilename, length+1);
  int ret = _wstat(nativeFilename, buffer);
  delete [] nativeFilename;
#else
  int ret = stat(name, buffer);
#endif
  return ret;
}


static bool fill_stat(const char *name) {
  if (last_statname && strcmp(last_statname, name)==0) return last_result;
  delete[] const_cast<char *>( last_statname ); // otherwize VC++ will scream
  last_statname = newstring(name);
#if defined(_WIN32) || defined(__EMX__)
  // _WIN32 apparently thinks A: is not a directory, but A:/ is!
  char buffer[4];
  if (name[0] && name[1]==':' && name[2] == 0) {
    buffer[0] = name[0];
    buffer[1] = ':';
    buffer[2] = '/';
    buffer[3] = 0;
    name = buffer;
  }
  // on _WIN32 && !__CYGWIN__, all strings into this file will be UTF-8

#endif// _WIN32 || __EMX__
  last_result = (fltk::fltk_stat(name, &last_stat)==0);
  return last_result;
}

/** Returns true if the file exists . */
bool fltk::filename_exist(const char* name) {
  return name && *name && fill_stat(name);
}

/** Returns true if the file exists and is a directory. */
bool fltk::filename_isdir(const char* name) {
  if (!fill_stat(name)) return false;
  return ((last_stat.st_mode & 0170000) == 0040000);
}

/** Returns true if the file exists and is a regular file. */
bool fltk::filename_isfile(const char* name) {
  if (!fill_stat(name)) return false;
  return (last_stat.st_mode&0170000)==0100000;
}

/** Returns the size of the file in bytes. Returns zero if it does not exist.*/
FL_FILESIZE_T fltk::filename_size(const char* name) {
  if (!fill_stat(name)) return 0;
  return (FL_FILESIZE_T)last_stat.st_size;
}

/**
  Returns the modification time of the file as a Unix timestamp
  (number of seconds since the start of 1970 in GMT). Returns 0
  if the file does not exist.
*/
long int fltk::filename_mtime(const char *name) {
  if (!fill_stat(name)) return 0L;
  if (last_stat.st_mtime) return (long) last_stat.st_mtime;
  if (last_stat.st_atime) return (long) last_stat.st_atime;
  return (long) last_stat.st_ctime;
}

/**
  Sorts two files based on their modification date.
*/
int fltk::modificationsort(const dirent*const* a, const dirent*const* b) {
  return fltk::filename_mtime((*a)->d_name) < fltk::filename_mtime((*b)->d_name);
}
//
// End of "$Id: filename_isdir.cxx 8507 2011-03-06 10:18:05Z bgbnbigben $".
//
