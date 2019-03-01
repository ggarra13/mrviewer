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
 * Common string header file for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2007 by Bill Spitzak and others.
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

#ifndef flustring_h
#  define flustring_h

#  include <FL/Fl_Export.H>
// OTB Modifications
//#include <config.h>
#include "flu-config.h"

#  include <stdio.h>
#  include <stdarg.h>
#  include <string.h>
#  ifdef HAVE_STRINGS_H
#    include <strings.h>
#  endif /* HAVE_STRINGS_H */
#  include <ctype.h>

/*
 * Apparently Unixware defines "index" to strchr (!) rather than
 * providing a proper entry point or not providing the (obsolete)
 * BSD function.  Make sure index is not defined...
 */

#  ifdef index
#    undef index
#  endif /* index */

#  if defined(WIN32) && !defined(__CYGWIN__)
#    define strcasecmp(s,t)	_stricmp((s), (t))
#    define strncasecmp(s,t,n)	_strnicmp((s), (t), (n))
// Visual C++ 2005 incorrectly displays a warning about the use of POSIX APIs
// on Windows, which is supposed to be POSIX compliant...  Some of these
// functions are also defined in ISO C99...
#    ifndef __WATCOMC__
#      define strdup _strdup
#      define unlink _unlink
#    endif // !__WATCOMC__
#  elif defined(__EMX__)
#    define strcasecmp(s,t)	stricmp((s), (t))
#    define strncasecmp(s,t,n)	strnicmp((s), (t), (n))
#  endif /* WIN32 */

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

/*
 * MetroWerks' CodeWarrior put thes "non-standard" functions in
 * <extras.h> which unfortunatly does not play well otherwise
 * when included - to be resolved...
 */

#  if defined(__APPLE__) && defined(__MWERKS__) && defined(_MSL_USING_MW_C_HEADERS)
int strcasecmp(const char*,const char*);
int strncasecmp(const char*,const char*,int);
char *strdup(const char*);
#  endif

FL_EXPORT extern int flu_snprintf(char *, size_t, const char *, ...);
#  if !HAVE_SNPRINTF
#    define snprintf flu_snprintf
#  endif /* !HAVE_SNPRINTF */

FL_EXPORT extern int flu_vsnprintf(char *, size_t, const char *, va_list ap);
#  if !HAVE_VSNPRINTF
#    define vsnprintf flu_vsnprintf
#  endif /* !HAVE_VSNPRINTF */

/*
 * strlcpy() and strlcat() are some really useful BSD string functions
 * that work the way strncpy() and strncat() *should* have worked.
 */

FL_EXPORT extern size_t flu_strlcat(char *, const char *, size_t);
#  if !HAVE_STRLCAT
#    define strlcat flu_strlcat
#  endif /* !HAVE_STRLCAT */

FL_EXPORT extern size_t flu_strlcpy(char *, const char *, size_t);
#  if !HAVE_STRLCPY
#    define strlcpy flu_strlcpy
#  endif /* !HAVE_STRLCPY */

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !flustring_h */


/*
 * End of "$Id$".
 */
