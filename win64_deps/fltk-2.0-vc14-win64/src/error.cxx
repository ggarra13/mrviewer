//
// "$Id: error.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Warning/error message code for the Fast Light Tool Kit (FLTK).
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

// This method is in it's own source file so that stdlib and stdio
// do not need to be included in Fl.C:
// You can also override this by redefining all of these.

#include <fltk/error.h>
#include <fltk/string.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef _WIN32

static void warning(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
}

static void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
  // done by abort() fflush(stderr);
  exit(1);
}

#else

#include <fltk/x.h>
#include <fltk/utf.h>

static void warning(const char *format, ...) {
  va_list args;
  char buf[1024];
  wchar_t ucsbuf[1024];
  va_start(args, format);
  int buflen = vsnprintf(buf, 1024, format, args);
  va_end(args);
  utf8towc(buf, buflen, ucsbuf, 1024);
  MessageBoxW(0, ucsbuf, L"Warning", MB_ICONEXCLAMATION|MB_OK);
}

static void error(const char *format, ...) {
  va_list args;
  char buf[1024];
  wchar_t ucsbuf[1024+1];
  va_start(args, format);
  int buflen = vsnprintf(buf, 1024, format, args);
  va_end(args);
  utf8towc(buf, buflen, ucsbuf, 1024);
  MessageBoxW(0, ucsbuf, L"Error", MB_ICONSTOP|MB_SYSTEMMODAL);
  exit(1);
}

#endif

// Doxygen seems to have some trouble with these unless the comments
// are put first like this:

/*! \var void (*fltk::warning)(const char* format, ...)
  fltk will call this when it wants to report a recoverable problem.
  The display may be messed up but the user can probably keep
  working. (all X protocol errors call this). The default version on
  Unix prints a message to stderr, on Windows it pops up a MessageBox.
*/

/*! \var void (*fltk::error)(const char* format, ...)
  fltk will call this when it wants to report a recoverable problem.
  but in this case the display is so messed up it is unlikely the user
  can continue. Very little calls this now. The default version on
  Unix prints a message to stderr, on Windows it pops up a MessageBox,
  and then both versions call exit(1).

  You may be able to use longjmp or an exception to get back to your
  own code.
*/

/*! \var void (*fltk::fatal)(const char* format, ...)
  fltk will call this when it wants to report a problem that it cannot
  recover from. You must not make any fltk calls again. The default
  version is the same function as error().
*/

void (*fltk::warning)(const char* format, ...) = ::warning;
void (*fltk::error)(const char* format, ...) = ::error;
void (*fltk::fatal)(const char* format, ...) = ::error;

//
// End of "$Id: error.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
