//
// "$Id: Window_hotspot.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Move windows but keep them on-screen.
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

#include <fltk/events.h>
#include <fltk/Window.h>
#include <fltk/Monitor.h>
#if defined(_WIN32)
# include <fltk/x.h>
#endif

using namespace fltk;

/*! position() the window so that the mouse is pointing at the center
  of the \a widget, which may be the window itself. If \a offscreen is
  true then the window is allowed to extend off the Monitor (some X
  window managers do not allow this). */
void Window::hotspot(const Widget *widget, bool offscreen) {
  int X = widget->w()/2;
  int Y = widget->h()/2;
  while (widget != this) {
    X += widget->x(); Y += widget->y();
    widget = widget->parent();
  }
  hotspot(X, Y, offscreen);
}

/*! position() the window so that the mouse is pointing at the \a
  cx,cy position. If \a offscreen is true then the window is allowed
  to extend off the Monitor (some X window managers do not allow
  this). */
void Window::hotspot(int cx, int cy, bool offscreen) {
  int X,Y; get_mouse(X,Y); X -= cx; Y -= cy;
  if (!offscreen) {
    Rectangle r; borders(&r);
    const Monitor& monitor = Monitor::find(X,Y);
    if (X+w()+r.r() > monitor.work.r()) X = monitor.work.r()-r.r()-w();
    if (X+r.x() < monitor.work.x()) X = monitor.work.x()-r.x();
    if (X+w() > monitor.work.r()) X = monitor.work.r()-w();
    if (X < monitor.work.x()) X = monitor.work.x();
    if (Y+h()+r.b() > monitor.work.h()) Y = monitor.work.h()-r.b()-h();
    if (Y+r.y() < monitor.work.y()) Y = monitor.work.y()-r.y();
    if (Y+h() > monitor.work.h()) Y = monitor.work.h()-h();
    if (Y < monitor.work.y()) Y = monitor.work.y();
  }
  position(X,Y);
}

//
// End of "$Id: Window_hotspot.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
