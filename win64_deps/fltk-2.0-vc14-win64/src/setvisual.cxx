//
// "$Id: setvisual.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <config.h>
#include <fltk/visual.h>
#include <fltk/x.h>

/*! \fn bool fltk::visual(int);

  X-specific crap to allow you to force the "visual" used by
  fltk to one you like, rather than the "default visual" which
  in many cases has less capabilities than your machine really
  has! For instance fltk::visual(fltk::RGB_COLOR) will get you a full
  color display instead of an 8-bit colormap, if possible.

  You must call this before you show() any windows. The integer
  argument is an 'or' of the following:
  - fltk::INDEXED_COLOR indicates that a colormapped visual is ok. This call
    will normally fail if a TrueColor visual cannot be found.
  - fltk::RGB_COLOR this value is zero and may be passed to indicate that
    fltk::INDEXED_COLOR is \e not wanted.
  - fltk::RGB24_COLOR indicates that the visual must have at least
    8 bits of red, green, and blue (Windows calls this "millions of
    colors").
  - fltk::DOUBLE_BUFFER indicates that hardware accelerated double buffering
    is wanted.
  - Other bits used by glVisual() and GlWindow::mode() are ignored by this.

  This returns true if the system has the capabilities by default or
  FLTK suceeded in turing them on. Your program will still work even
  if this returns false (it just won't look as good).

  On non-X systems this just returns true or false indicating if the
  system supports the passed values.
*/

#if USE_X11

#if USE_XDBE
#define Window XWindow
#include <X11/extensions/Xdbe.h>
#undef Window
#endif

using namespace fltk;

static int test_visual(XVisualInfo& v, int flags) {
  if (v.screen != xscreen) return 0;
#if USE_COLORMAP
  if (!(flags & INDEXED_COLOR)) {
    if (v.c_class != StaticColor && v.c_class != TrueColor) return 0;
    if (v.depth <= 8) return 0; // fltk will work better in colormap mode
  }
  if (flags & RGB24_COLOR) {
    if (v.depth < 24) return 0;
  }
  // for now, fltk does not like colormaps of more than 8 bits:
  if ((v.c_class&1) && v.depth > 8) return 0;
#else
  // simpler if we can't use colormapped visuals at all:
  if (v.c_class != StaticColor && v.c_class != TrueColor) return 0;
#endif
#if USE_XDBE
  if (flags & DOUBLE_BUFFER) {
    static XdbeScreenVisualInfo *xdbejunk = 0;
    if (!xdbejunk) {
      int event_base, error_base;
      if (!XdbeQueryExtension(xdisplay, &event_base, &error_base)) return 0;
      XWindow root = RootWindow(xdisplay, xscreen);
      int numscreens = 1;
      xdbejunk = XdbeGetVisualInfo(xdisplay,&root,&numscreens);
      if (!xdbejunk) return 0;
    }
    for (int j = 0; ; j++) {
      if (j >= xdbejunk->count) return 0;
      if (xdbejunk->visinfo[j].visual == v.visualid) break;
    }
  }
#endif
  return 1;
}

bool fltk::visual(int flags) {
  open_display();
  // always use default if possible:
  if (test_visual(*xvisual, flags)) return true;
  // get all the visuals:
  XVisualInfo vTemplate;
  int num;
  XVisualInfo *visualList = XGetVisualInfo(xdisplay, 0, &vTemplate, &num);
  // find all matches, use the one with greatest depth:
  XVisualInfo *found = 0;
  for (int i=0; i<num; i++) if (test_visual(visualList[i], flags)) {
    if (!found || found->depth < visualList[i].depth)
      found = &visualList[i];
  }
  if (!found) {XFree((void*)visualList); return false;}
  xvisual = found;
  xcolormap = XCreateColormap(xdisplay, RootWindow(xdisplay,xscreen),
			      xvisual->visual, AllocNone);
  return true;
}

#elif defined(_WIN32)

bool fltk::visual(int flags) {
  if (flags & DOUBLE_BUFFER) return false;
  bool ret = true;
  HDC screen = GetDC(0);
  if ((!(flags & INDEXED_COLOR) && GetDeviceCaps(screen, BITSPIXEL) <= 8) ||
     ((flags & RGB24_COLOR) && GetDeviceCaps(screen, BITSPIXEL)<24)) ret = false;
  ReleaseDC(0, screen);
  return ret;
}

#else
// all other systems we assumme are rgb-always:

bool fltk::visual(int flags) {
  // INDEXED_COLOR probably should return false, but who really cares...
  return true;
}

#endif

//
// End of "$Id: setvisual.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
