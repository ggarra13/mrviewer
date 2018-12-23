// This emulation header was designed to get flwm to compile. It
// probably does not work for any other x fltk program.

#ifndef Fl_X_H
#define Fl_X_H

#include "Enumerations.H"
#include <fltk/x.h>

#define Fl_X		fltk::CreatedWindow
#define fl_display	fltk::xdisplay
#define fl_screen	fltk::xscreen
#define fl_xevent	(&fltk::xevent)
#define fl_colormap	fltk::xcolormap
#define fl_open_display	fltk::open_display
#define fl_visual	fltk::xvisual
#define fl_xid(w)	fltk::xid(w)
#define fl_event_time	fltk::event_time
#define fl_xpixel(x)	fltk::xpixel(x)
#define fl_window	fltk::xwindow
#define fl_gc		fltk::gc
#define fl_xfont	fltk::xfont()

#endif
