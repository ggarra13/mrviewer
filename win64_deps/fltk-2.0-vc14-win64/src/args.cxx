//
// "$Id: args.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// OPTIONAL initialization code for a program using fltk.
// You do not need to call this!  Feel free to make up your own switches.
// Optional argument initialization code for the Fast Light Tool Kit (FLTK).
//

#include <config.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/filename.h>
#include <fltk/error.h>
#include <fltk/Monitor.h>

#include <fltk/x.h>

#if !USE_X11
int XParseGeometry(const char*, int*, int*, unsigned int*, unsigned int*);
#define NoValue		0x0000
#define XValue  	0x0001
#define YValue		0x0002
#define WidthValue  	0x0004
#define HeightValue  	0x0008
#define AllValues 	0x000F
#define XNegative 	0x0010
#define YNegative 	0x0020
#endif

using namespace fltk;

// This fixes linkage problem in VC++ 6.0
#if defined(_MSC_VER)
  //extern FL_API const char *filename_name(const char *);
#endif

static int match(const char *a, const char *match, int atleast = 1) {
  const char *b = match;
  while (*a && (*a == *b || tolower(*a) == *b)) {a++; b++;}
  return !*a && b >= match+atleast;
}

// flags set by previously parsed arguments:
extern bool fl_show_iconic;
static bool arg_called;
static bool return_i;
static const char* name;
static const char* geometry;
extern Color fl_bg_switch;	// in Style.cxx

/*!
  Consume a single switch from \a argv, starting at word \a i. Returns the
  number of words eaten (1 or 2, or 0 if it is not recognized) and
  adds the same value to \a i. You can use this function if you prefer to
  control the incrementing through the arguments yourself. The arguments
  recognized are listed under args().

  The args() argument parser is <i>entirely optional</i>. It was written
  to make demo programs easy to write, although some minor work was done
  to make it usable by more complex programs. But there is no requirement
  that you call it or even acknoledge it's existence, if you prefer to
  use your own code to parse switches.
*/
int fltk::arg(int argc, char **argv, int &i) {
  arg_called = true;

  const char *s = argv[i];

  if (!s) {i++; return 1;}	// something removed by calling program?

  // a word that does not start with '-', or a word after a '--', or
  // the word '-' by itself all start the "non-switch arguments" to
  // a program.  Return 0 to indicate that we don't understand the
  // word, but set a flag (return_i) so that args() will return at
  // that point:
  if (s[0] != '-' || s[1] == '-' || !s[1]) {return_i = true; return 0;}
  s++; // point after the dash

  if (match(s, "iconic")) {
    fl_show_iconic = true;
    i++;
    return 1;
  }

  const char *v = argv[i+1];
  if (i >= argc-1 || !v)
    return 0;	// all the rest need an argument, so if missing it is an error

  if (match(s, "geometry")) {

    int flags, gx, gy; unsigned int gw, gh;
    flags = XParseGeometry(v, &gx, &gy, &gw, &gh);
    if (!flags) return 0;
    geometry = v;

  } else if (match(s, "display")) {
    display(v);

  } else if (match(s, "name")) {
    name = v;

  } else if (match(s, "bg") || match(s, "background")) {
    fl_bg_switch = color(v);
    if (!fl_bg_switch) error("Unknown color \"%s\"", v);

  } else return 0; // unrecognized

  i += 2;
  return 2;
}

/*! Consume all switches from argv.

  To use the switch parser, call fltk::args(argc,argv) near the start of
  your program. This does not open the display, instead switches that
  need the display open are stashed into static variables. Then you
  must display your first window by calling window->show(argc,argv),
  which will do anything stored in the static variables.

  \a callback lets you define your own switches. It is called with the
  same argc and argv, and with i the index of each word. The callback
  should return zero if the switch is unrecognized, and not change
  i. It should return non-zero if the switch is recognized, and add at
  least 1 to i (it can add more to consume words after the
  switch). This function is called before any other tests, so you can
  override any FLTK switch (this is why fltk can use very short
  switches instead of the long ones all other toolkits force you to
  use).

  On return \a i is set to the index of the first non-switch. This is either:
  - The first word that does not start with '-'.
  - The word "-" (used by many programs to name stdin as a file)
  - The first word after "--" (GNU standard end-of-switches switch)
  - The first unrecognized switch (return value is 0).
  - \a argc
 
  The return value is i unless an unrecognized switch is found, in
  which case it is zero. If your program takes no arguments other than
  switches you should produce an error if the return value is less
  than argc.

  All switches may be abbreviated one letter and case is ignored: 
  - \c -iconic Window::iconize() will be done to the window.
  - \c -geometry \e WxH Window is resized to this width & height
  - \c -geometry \e +X+Y Initial window position
  - \c -geometry \e WxH+X+Y Window is resized and positioned.
  - \c -display \e host or \c -display \e host:n.n The X display to use (ignored under WIN32).
  - \c -name \e string will set the Window::label()
  - \c -bg \e color Call set_background() with the named color.
     Use "#rrggbb" to set it in hex.
  - \c -background \e color is the same as -bg color

*/
int fltk::args(int argc, char** argv, int& i, int (*cb)(int,char**,int&)) {
  arg_called = true;
  i = 1; // skip argv[0]
  while (i < argc) {
    if (cb && cb(argc,argv,i)) continue;
    if (!arg(argc,argv,i)) {if (!return_i) i = 0; break;}
  }
  return i;
}

/*!
  This must be called after fltk::args(argc,argv) to show the "main"
  window, this indicates which window should be affected by any
  -geometry switch. In addition if fltk::args() has not been called
  yet this does so, this is a useful shortcut for the main window in a
  small program.
*/
void Window::show(int argc, char **argv) {
  if (argc < 1) {show(); return;}
  if (!arg_called) args(argc,argv);

  if (geometry) {
    sizes();//to allocate the array if this window was not end()ed...
    int flags = 0, gx = x(), gy = y(); unsigned int gw = w(), gh = h();
    flags = XParseGeometry(geometry, &gx, &gy, &gw, &gh);
    if (flags & XNegative) gx = Monitor::all().w()-w()+gx;
    if (flags & YNegative) gy = Monitor::all().h()-h()+gy;
    //  int mw,mh; minsize(mw,mh);
    //  if (mw > gw) gw = mw;
    //  if (mh > gh) gh = mh;
    Widget *r = resizable();
    if (!r) resizable(this);
    // for _WIN32 we assume window is not mapped yet:
    if (flags & (XValue | YValue))
      x(-1), resize(gx,gy,gw,gh);
    else
      Widget::resize(gw,gh);
    // layout();// not needed?
    resizable(r);
    geometry = 0;
  }

  // can't just assume argv[0], since you don't always *have* one on win32...
  if (argc && !name)  name = filename_name(argv[0]);
  // make sure name exists and isn't empty...
  // Windows doesn't like empty window class names
  if (name && *name) xclass(name);
  if (!label()) label(name);

  show();

#if USE_X11
  // set the command string, used by state-saving window managers:
  int i;
  int n=0; for (i=0; i<argc; i++) n += strlen(argv[i])+1;
  char *buffer = new char[n];
  char *p = buffer;
  for (i=0; i<argc; i++) for (const char *q = argv[i]; (*p++ = *q++););
  XChangeProperty(xdisplay, xid(this), XA_WM_COMMAND, XA_STRING, 8, 0,
		  (unsigned char *)buffer, p-buffer-1);
  delete[] buffer;
#endif
}

// Calls useful for simple demo programs, with automatic help message:

static const char * const helpmsg =
"options are:\n"
#if USE_X11
" -d[isplay] host:n.n\n"
#endif
" -g[eometry] WxH+X+Y\n"
" -n[ame] windowname\n"
" -i[conic]\n"
" -bg color";

/*!
  This is a portion of the string printed by fltk::args() detects
  an invalid argument on the command-line. You can add this to your
  own error or help message to show the fltk switches. It's value is
  (no newline at start or the end):
\code
 -d[isplay] host:n.n
 -g[eometry] WxH+X+Y
 -n[ame] windowname
 -i[conic]
 -bg color
\endcode
*/
const char * const fltk::help = helpmsg+13;

/*!
  The second form of fltk::args() is useful if your program does not
  have command line switches of its own. It parses all the switches,
  and if any are not recognized it calls fltk::fatal(fltk::help).
*/
void fltk::args(int argc, char **argv) {
  int i; if (args(argc,argv,i) < argc) error(helpmsg);
}

#if !USE_X11

/* the following function was stolen from the X sources as indicated. */

/* Copyright 	Massachusetts Institute of Technology  1985, 1986, 1987 */
/* $XConsortium: XParseGeom.c,v 11.18 91/02/21 17:23:05 rws Exp $ */

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */

static int ReadInteger(char* string, char** NextString)
{
  register int Result = 0;
  int Sign = 1;
    
  if (*string == '+')
    string++;
  else if (*string == '-') {
    string++;
    Sign = -1;
  }
  for (; (*string >= '0') && (*string <= '9'); string++) {
    Result = (Result * 10) + (*string - '0');
  }
  *NextString = string;
  if (Sign >= 0)
    return (Result);
  else
    return (-Result);
}

int XParseGeometry(const char* string, int* x, int* y,
		   unsigned int* width, unsigned int* height)
{
  int mask = NoValue;
  register char *strind;
  unsigned int tempWidth=0, tempHeight=0;
  int tempX=0, tempY=0;
  char *nextCharacter;

  if ( (string == NULL) || (*string == '\0')) return(mask);
  if (*string == '=')
    string++;  /* ignore possible '=' at beg of geometry spec */

  strind = (char *)string;
  if (*strind != '+' && *strind != '-' && *strind != 'x') {
    tempWidth = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter) 
      return (0);
    strind = nextCharacter;
    mask |= WidthValue;
  }

  if (*strind == 'x' || *strind == 'X') {	
    strind++;
    tempHeight = ReadInteger(strind, &nextCharacter);
    if (strind == nextCharacter)
      return (0);
    strind = nextCharacter;
    mask |= HeightValue;
  }

  if ((*strind == '+') || (*strind == '-')) {
    if (*strind == '-') {
      strind++;
      tempX = -ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
	return (0);
      strind = nextCharacter;
      mask |= XNegative;

    } else {
      strind++;
      tempX = ReadInteger(strind, &nextCharacter);
      if (strind == nextCharacter)
	return(0);
      strind = nextCharacter;
      }
    mask |= XValue;
    if ((*strind == '+') || (*strind == '-')) {
      if (*strind == '-') {
	strind++;
	tempY = -ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	  return(0);
	strind = nextCharacter;
	mask |= YNegative;

      } else {
	strind++;
	tempY = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	  return(0);
	strind = nextCharacter;
      }
      mask |= YValue;
    }
  }
	
  /* If strind isn't at the end of the string the it's an invalid
     geometry specification. */

  if (*strind != '\0') return (0);

  if (mask & XValue)
    *x = tempX;
  if (mask & YValue)
    *y = tempY;
  if (mask & WidthValue)
    *width = tempWidth;
  if (mask & HeightValue)
    *height = tempHeight;
  return (mask);
}

#endif // if !USE_X11

//
// End of "$Id: args.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
