// "$Id: x.h 5140 2006-05-22 04:46:07Z spitzak $"
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

/** \file

Declarations of FLTK symbols and interfaces that only exist if FLTK
is compiled to use X11, as it is by default on Unix. It is recommended
you avoid using this header file, and that you segregate code requiring
it to it's own source file.

This header includes the Xlib.h, Xutil.h, Xatom.h, XInput.h, and XI.h
header files, with sufficent wrappers to avoid conflicts with FLTK.
In particular a macro is used so that "Window" is replaced by "XWindow"
in these headers. You should use this rather than including the X
header files directly.

*/

#ifndef fltk_x11_h
# define fltk_x11_h

////////////////////////////////////////////////////////////////
// Try to get the parts of Xlib.h included while avoiding warnings:

#  ifndef DOXYGEN
#   define Window XWindow

// pragmas are to fix the broken SGI Irix Xlib header files:
#   if !defined(__GNUC__) && (defined(_ABIN32) || defined(_ABI64))
#    pragma set woff 3322
#   endif
#   include <X11/Xlib.h>
#   include <X11/Xutil.h>
#   if !defined(__GNUC__) && (defined(_ABIN32) || defined(_ABI64))
#    pragma reset woff 3322
#   endif

#   include <X11/Xatom.h>

#   if USE_XFT && !MAKEDEPEND
#    include <X11/Xft/Xft.h>
#   else
     typedef struct _XftDraw XftDraw;
     typedef struct _XftFont XftFont;
#   endif

#   include <X11/extensions/XInput.h>
#   include <X11/extensions/XI.h>

#   undef Window

#   if defined(__FreeBSD__) || defined(__APPLE__) || defined(__CYGWIN__)
     typedef unsigned long	ulong;
     typedef unsigned int      	uint;
     typedef unsigned char	uchar;
#   endif

#  endif // !DOXYGEN

#include "draw.h"

extern FL_API Region	XRectangleRegion(int x, int y, int w, int h);
const int XSTATE_MASK = 0x1fff;

namespace fltk {

/// \name fltk/x11.h
//@{

////////////////////////////////////////////////////////////////
// constant info about the X server connection:

extern FL_API void	open_display();
extern FL_API void	open_display(Display*);
extern FL_API void	close_display();

extern FL_API Display*	xdisplay;
extern FL_API XWindow	message_window;
extern FL_API int	xscreen;
extern FL_API XVisualInfo* xvisual;
extern FL_API Colormap	xcolormap;

////////////////////////////////////////////////////////////////
// event handling:

// feed events into fltk by setting xevent and calling handle:
extern FL_API XEvent	xevent;
extern FL_API bool	handle();

// set by last xevent with a timestamp:
extern FL_API ulong	event_time;

////////////////////////////////////////////////////////////////
// DnD:

extern FL_API XWindow dnd_source_window;
extern FL_API Atom *dnd_source_types;
extern FL_API Atom dnd_type;
extern FL_API Atom dnd_source_action;
extern FL_API Atom dnd_action;

////////////////////////////////////////////////////////////////
// drawing functions:

extern FL_API XftDraw*	xftc;
extern FL_API GC	gc;
extern FL_API XWindow	xwindow;
extern FL_API ulong	current_xpixel;
extern FL_API ulong	xpixel(Color i);
extern FL_API void	clip_region(Region);
extern FL_API Region	clip_region();

extern FL_API void	draw_into(XWindow, int w, int h);
extern FL_API void	stop_drawing(XWindow);

extern FL_API XFontStruct* xfont();
extern FL_API XftFont*  xftfont();

////////////////////////////////////////////////////////////////
// only include this if <fltk/Window.h> was included:
#  if defined(fltk_Window_h) || defined(DOXYGEN)

/**
  When fltk tells X about a window, one of these objects is created.
  Warning: this object is highly subject to change!  It's definition
  is only here so that xid(Window) can be declared inline
*/
class FL_API CreatedWindow {
public:
  XWindow xid; //!< The CreatedWindow's XWindow IDentifier
  XWindow backbuffer; //!< The CreatedWindow's secondary buffer
  XWindow frontbuffer; //!< The CreatedWindow's primary buffer
  Window *window; //!< The Window (if any) this CreatedWindow refers to
  Region region; //!< The X Region this Window operates on
  void expose(const Rectangle&);
  CreatedWindow *next; //!< The next CreatedWindow in the linked list
  bool wait_for_expose; //!< Boolean flag determining whether or not the window must wait until it is exposed to receive eventsf
  bool backbuffer_bad; //!< used for XDBE
  bool overlay; //!< Whether or not to redraw the overlay on the next redraw loop
  ::Cursor cursor; //!< The XCursor this window uses
  const Widget* cursor_for;
  static CreatedWindow* first; //!< The first CreatedWindow of the linked list
  /** Finds a Window's corresponding CreatedWindow
      \param window the Window to find
      \return A pointer to the Window's CreatedWindow
  */
  static CreatedWindow* find(const Window* window) {return window->i;}
  void sendxjunk();
  static void create(Window*,
		     XVisualInfo*, Colormap,
		     int background = -1);
  static CreatedWindow* set_xid(Window*, XWindow);
  Rectangle current_size;
};

/** Converts a Window to its xid
    \param w The window to convert
    \return The value of the Window's xid
*/
inline XWindow xid(const Window*w) {return CreatedWindow::find(w)->xid;}
/** Converts a xid to its equivalent Window
    \param xid The XWindow ID of the window
    \return The Window matching this xid, or NULL if it doesn't exist
*/
Window* find(XWindow xid);

#  endif // Window_h

//@}

} // namespace fltk

#   if USE_CAIRO
#    include <fltk/fltk_cairo.h>
#    include <cairo-xlib.h>
#   else
     typedef struct _cairo cairo_t;
#   endif
#endif
