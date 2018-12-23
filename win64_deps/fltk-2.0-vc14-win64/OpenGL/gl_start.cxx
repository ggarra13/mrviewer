// "$Id: gl_start.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <config.h>
#if HAVE_GL

#include <fltk/Window.h>
#include "GlChoice.h"
#include <fltk/draw.h>
#include <fltk/visual.h>
#include <fltk/gl.h>
using namespace fltk;

#ifdef _WIN32
static GlChoice* gl_choice;
#endif

#ifdef __APPLE__
static GlChoice *gl_choice;
#endif

/**
  Same as fltk::visual(int) except choose a visual that is also capable
  of drawing OpenGL. On modern X servers this is true by default, but
  on older ones OpenGL will crash if the visual is not selected with
  this.

  \a mode is the same bitflags accepted by GlWindow::mode(). This
  causes all windows (and thus glstart()) to have these capabilities.
*/
bool fltk::glVisual(int mode) {
  GlChoice *c = GlChoice::find(mode);
  if (!c) return false;
#ifdef _WIN32
  gl_choice = c;
#elif defined(__APPLE__)
  gl_choice = c;
#else
  xvisual = c->vis;
  xcolormap = c->colormap;
#endif
  return true;
}

static GLContext context;
//static int clip_state_number=-1;
static int pw, ph;

/**
  Set up an OpenGL context to draw into the current window being
  drawn by fltk. This will allow you to use OpenGL to update a
  normal window. The current transformation is reproduced, and
  the current clipping is simulated with glScissor() calls (which
  can only do a rectangle).

  You must use glfinish() to exit this mode before any normal fltk
  drawing calls are done.

  You should call glvisual() at program startup if you intend to
  use this. This may be used to change how windows are created so
  this call works.

  I would like this to work reliably, but it is not real good
  now on any platforms. In particular it does not cooperate with
  the double-buffering schemes. It does appear to work on X
  when you turn off double buffering, it also works if OpenGL
  is entirely software, such as MESA.

  Do \e not call glstart()/glfinish() when drawing into a GlWindow!
*/
void fltk::glstart() {
  if (!context) {
#ifdef _WIN32
    if (!gl_choice) glVisual(0);
    context = create_gl_context(Window::drawing_window(), gl_choice);
#elif defined(__APPLE__)
    context = create_gl_context(Window::drawing_window(), gl_choice);
#else
    context = create_gl_context(xvisual);
#endif
  }
  set_gl_context(Window::drawing_window(), context);
#ifdef _WIN32
  ;
#elif defined(__APPLE__)
  ;
#else
  glXWaitX();
#endif
  if (pw != Window::drawing_window()->w() || ph != Window::drawing_window()->h()) {
    pw = Window::drawing_window()->w();
    ph = Window::drawing_window()->h();
    glLoadIdentity();
    glViewport(0, 0, pw, ph);
    glOrtho(0, pw, 0, ph, -1, 1);
    glDrawBuffer(GL_FRONT);
  }
  // obey the clipping. Only rectangles work:
  Rectangle r(Window::drawing_window()->w(), Window::drawing_window()->h());
  // 0 = all clipped, 1 = no change, 2 = partial clip:
  if (intersect_with_clip(r) != 1) {
    glScissor(r.x(), Window::drawing_window()->h()-r.b(), r.w(), r.h());
    glEnable(GL_SCISSOR_TEST);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }
}

/**
  Turn off the effects of a previous glstart(). You must call this before
  using normal fltk drawing methods.
*/
void fltk::glfinish() {
  glFlush();
#ifdef _WIN32
  ;
#elif defined(__APPLE__)
  ;
#else
  glXWaitGL();
#endif
}

#endif

// End of "$Id: gl_start.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
