// "$Id: Fl_Gl_Window.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include "GlChoice.h"
#include <fltk/GlWindow.h>
#include <fltk/damage.h>
#include <fltk/error.h>
#include <fltk/visual.h>
#include <fltk/layout.h>
#include <fltk/run.h>
#include <fltk/events.h>
#include <stdlib.h>
#include <string.h>
using namespace fltk;

/** \class fltk::GlWindow

Provides an area in which the draw() method can use OpenGL to draw.
This widget sets things up so OpenGL works, and also keeps an OpenGL
"context" for that window, so that changes to the lighting and
projection may be reused between redraws. GlWindow also flushes
the OpenGL streams and swaps buffers after draw() returns.

draw() is a pure virtual method.  You must subclass GlWindow and
provide an implementation for draw(). You can avoid reinitializing the
viewport and lights and other things by checking valid() at the start
of draw() and only doing the initialization if it is false.

draw() can \e only use OpenGL calls.  Do not attempt to call any of
the functions in &lt;fltk/draw.h&gt;, or X or GDI32 or any other
drawing api.  Do not call glstart() or glfinish().

<h2>Double Buffering</h2>

Normally double-buffering is enabled. You can disable it by chaning
the mode() to turn off the DOUBLE_BUFFER bit.

If double-buffering is enabled, the back buffer is made current before
draw() is called, and the back and front buffers are \e automatically
swapped after draw() is completed.

Some tricks using the front buffer require you to control the
swapping.  You can call swap_buffers() to swap them (OpenGL does not
provide a portable function for this, so we provide it). But you will
need to turn off the auto-swap, you do this by adding the NO_AUTO_SWAP
bit to the mode().

<h2>Overlays</h2>

The method draw_overlay() is a second drawing operation that is put
atop the main image. You can implement this, and call redraw_overlay()
to indicate that the image in this overlay has changed and that
draw_overlay() must be called.

Originally this was written to support hardware overlays, but FLTK
emulated it if the hardware was missing so programs were
portable. FLTK 2.0 is not normally compiled to support hardware
overlays, but the emulation still remains, so you can use these
functions. (Modern hardware typically has no overlays, and besides it
is fast enough that the original purpose of them is moot)

By default the emulation is to call draw_overlay() after draw() and
before swapping the buffers, so the overlay is just part of the normal
image and does not blink. You can get some of the advantages of
overlay hardware by setting the GL_SWAP_TYPE environment variable,
which will cause the front buffer to be used for the draw_overlay()
method, and not call draw() each time the overlay changes. This
will be faster if draw() is very complex, but the overlay will
blink. GL_SWAP_TYPE can be set to:
- "USE_COPY" use glCopyPixels to copy the back buffer to the front.
This should always work.
- "COPY" indicates that the swap_buffers() function actually copies the
back to the front buffer, rather than swapping them. If your card
does this (most do) then this is best.
- "NODAMAGE" indicates that behavior is like "COPY" but \e nothing
changes the back buffer, including overlapping it with another OpenGL
window. This is true of software OpenGL emulation, and may be true
of some modern cards with lots of memory.

*/

////////////////////////////////////////////////////////////////

// The symbol SWAP_TYPE defines what is in the back buffer after doing
// a glXSwapBuffers().

// The OpenGl documentation says that the contents of the backbuffer
// are "undefined" after glXSwapBuffers().  However, if we know what
// is in the backbuffers then we can save a good deal of time.  For
// this reason you can define some symbols to describe what is left in
// the back buffer.

// Having not found any way to determine this from glx (or wgl) I have
// resorted to letting the user specify it with an environment variable,
// GL_SWAP_TYPE, it should be equal to one of these symbols:

// contents of back buffer after glXSwapBuffers():
#define SWAP 1		// assumme garbage is in back buffer (default)
#define USE_COPY 2	// use glCopyPixels to imitate COPY behavior
#define COPY 3		// unchanged
#define NODAMAGE 4	// unchanged even by X expose() events

static char SWAP_TYPE = 0; // 0 = determine it from environment variable

////////////////////////////////////////////////////////////////

/** \fn bool GlWindow::can_do() const
  Returns true if the hardware supports the current value of mode().
  If false, attempts to show or draw this window will cause an fltk::error().
*/

/**
Returns true if the hardware supports \a mode, see mode() for the
meaning of the bits.
*/
bool GlWindow::can_do(int mode) {
  return GlChoice::find(mode) != 0;
}

void GlWindow::create() {
  if (!gl_choice) {
    gl_choice = GlChoice::find(mode_);
    if (!gl_choice) {error("Insufficient GL support"); return;}
  }
#ifdef WIN32
  Window::create();
#elif USE_QUARTZ
  Window::create();
#else
  CreatedWindow::create(this, gl_choice->vis, gl_choice->colormap, -1);
  //if (overlay && overlay != this) ((GlWindow*)overlay)->show();
  //fltk::flush(); glXWaitGL(); glXWaitX();
#endif
}

/** \fn char GlWindow::valid() const;
  This flag is turned off on a new window or if the window is ever
  resized or the context is changed. It is turned on after draw()
  is called. draw() can use this to skip initializing the viewport,
  lights, or other pieces of the context.

\code
void My_GlWindow_Subclass::draw() {
  if (!valid()) {
    glViewport(0,0,w(),h());
    glFrustum(...);
    glLight(...);
    glEnable(...);
    ...other initialization...
  }
  ... draw your geometry here ...
}
\endcode
*/

/** Turn off valid(). */
void GlWindow::invalidate() {
  valid(0);
#if USE_X11
  if (overlay) ((GlWindow*)overlay)->valid(0);
#endif
}

/**
Set or change the OpenGL capabilites of the window.  The value can be 
any of the symbols from \link visual.h <fltk/visual.h> \endlink OR'd together:

- fltk::INDEXED_COLOR indicates that a colormapped visual is ok. This call
  will normally fail if a TrueColor visual cannot be found.
- fltk::RGB_COLOR this value is zero and may be passed to indicate that
  fltk::INDEXED_COLOR is \e not wanted.
- fltk::RGB24_COLOR indicates that the visual must have at least
  8 bits of red, green, and blue (Windows calls this "millions of
  colors").
- fltk::DOUBLE_BUFFER indicates that double buffering is wanted.
- fltk::SINGLE_BUFFER is zero and can be used to indicate that double
  buffering is \a not wanted.
- fltk::ACCUM_BUFFER makes the accumulation buffer work
- fltk::ALPHA_BUFFER makes an alpha buffer
- fltk::DEPTH_BUFFER makes a depth/Z buffer
- fltk::STENCIL_BUFFER makes a stencil buffer
- fltk::MULTISAMPLE makes it multi-sample antialias if possible (X only)
- fltk::STEREO stereo if possible
- NO_AUTO_SWAP disables the automatic call to swap_buffers() after draw().
- NO_ERASE_OVERLAY if overlay hardware is used, don't call glClear before
  calling draw_overlay().

If the desired combination cannot be done, FLTK will try turning
off MULTISAMPLE and STERERO.  If this also fails then attempts to create
the context will cause fltk::error() to be called, aborting the program.
Use can_do() to check for this and try other combinations.

You can change the mode while the window is displayed.  This is most 
useful for turning double-buffering on and off.  Under X this will 
cause the old X window to be destroyed and a new one to be created.  If 
this is a top-level window this will unfortunately also cause the 
window to blink, raise to the top, and be de-iconized, and the ID
will change, possibly breaking other code.  It is best to make the GL 
window a child of another window if you wish to do this!
*/
bool GlWindow::mode(int m) {
  if (m == mode_) return false;
  mode_ = m;
  // destroy context and g:
  if (shown()) {
    bool reshow = visible_r();
    destroy();
    gl_choice = 0;
    if (reshow) {create(); show();}
  }
  return true;
}

#define NON_LOCAL_CONTEXT 0x80000000

/**
Selects the OpenGL context for the widget, creating it if necessary.
It is called automatically prior to the draw() method being
called. You can call it in handle() to set things up to do OpenGL
hit detection, or call it other times to do incremental update
of the window.
*/
void GlWindow::make_current() {
  Window::make_current(); // so gc is correct for non-OpenGL calls
#if USE_QUARTZ
  if (!gl_choice) {
    gl_choice = GlChoice::find(mode_);
    if (!gl_choice) {error("Insufficient GL support"); return;}
  }
#endif

  if (!context_) {
    mode_ &= ~NON_LOCAL_CONTEXT;
    context_ = create_gl_context(this, gl_choice);
    set_gl_context(this, context_);
    valid(0);
  } else {
    set_gl_context(this, context_);
  }
}

/**
Set the projection so 0,0 is in the lower left of the window and each
pixel is 1 unit wide/tall.  If you are drawing 2D images, your
draw() method may want to call this when valid() is
false.
*/
void GlWindow::ortho() {
#if 1
  // simple version
  glLoadIdentity();
  glViewport(0, 0, w(), h());
  glOrtho(0, w(), 0, h(), -1, 1);
#else
  // this makes glRasterPos off the edge much less likely to clip,
  // but a lot of OpenGL drivers do not like it.
  GLint v[2];
  glGetIntegerv(GL_MAX_VIEWPORT_DIMS, v);
  glLoadIdentity();
  glViewport(w()-v[0], h()-v[1], v[0], v[1]);
  glOrtho(w()-v[0], w(), h()-v[1], h(), -1, 1);
#endif
}

/**
  Swap the front and back buffers of this window (or copy the
  back buffer to the front, possibly clearing or garbaging the back one,
  depending on your OpenGL implementation.

  This is called automatically after draw() unless the NO_AUTO_SWAP
  flag is set in the mode().
*/
void GlWindow::swap_buffers() {
#ifdef _WIN32
#if USE_GL_OVERLAY
  // Do not swap the overlay, to match GLX:
  wglSwapLayerBuffers(CreatedWindow::find(this)->dc, WGL_SWAP_MAIN_PLANE);
#else
  SwapBuffers(CreatedWindow::find(this)->dc);
#endif
#elif USE_QUARTZ
  aglSwapBuffers((AGLContext)context_);
#else
  glXSwapBuffers(xdisplay, xid(this));
#endif
}

#if USE_GL_OVERLAY && defined(_WIN32)
int fl_overlay_depth = 0;
bool fl_overlay;
#endif

void GlWindow::flush() {
  uchar save_valid = valid_;
  uchar save_damage = damage();

  CreatedWindow* i = CreatedWindow::find(this);
#if USE_GL_OVERLAY && defined(_WIN32)
  if (save_damage == DAMAGE_OVERLAY && overlay && overlay != this && !i->region) goto OVERLAY_ONLY;
#endif

  make_current();

  if (mode_ & DOUBLE_BUFFER) {

    glDrawBuffer(GL_BACK);

    if (!SWAP_TYPE) {
      SWAP_TYPE = SWAP;
      const char* c = getenv("GL_SWAP_TYPE");
      if (c) switch (c[0]) {
      case 'U' : SWAP_TYPE = USE_COPY; break;
      case 'C' : SWAP_TYPE = COPY; break;
      case 'N' : SWAP_TYPE = NODAMAGE; break;
      default  : SWAP_TYPE = SWAP; break;
      }
    }

    if (SWAP_TYPE == NODAMAGE) {

      // don't draw if only overlay damage or expose events:
      if (save_damage != DAMAGE_OVERLAY || !save_valid) {
	draw();
	if (!(mode_ & NO_AUTO_SWAP)) swap_buffers();
      } else {
	swap_buffers();
      }

    } else if (SWAP_TYPE == COPY) {

      // don't draw if only the overlay is damaged:
      if (save_damage != DAMAGE_OVERLAY || i->region || !save_valid) {
	draw();
	if (!(mode_ & NO_AUTO_SWAP)) swap_buffers();
      } else {
	swap_buffers();
      }

    } else if (SWAP_TYPE == USE_COPY && overlay == this) {
      // If we are faking the overlay, use CopyPixels to act like
      // SWAP_TYPE == COPY.  Otherwise overlay redraw is way too slow.
      // don't draw if only the overlay is damaged:
      if (damage1_ || save_damage != DAMAGE_OVERLAY || i->region || !save_valid) draw();
      // we use a seperate context for the copy because rasterpos must be 0
      // and depth test needs to be off:
      static GLContext ortho_context = 0;
      static GlWindow* ortho_window = 0;
      if (!ortho_context) {
        ortho_context = create_gl_context(this, gl_choice);
        save_valid = 0;
      }
      set_gl_context(this, ortho_context);
      if (!save_valid || ortho_window != this) {
	ortho_window = this;
	glDisable(GL_DEPTH_TEST);
	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_FRONT);
	glLoadIdentity();
	glViewport(0, 0, w(), h());
	glOrtho(0, w(), 0, h(), -1, 1);
	glRasterPos2i(0,0);
      }
      glCopyPixels(0,0,w(),h(),GL_COLOR);
      make_current(); // set current context back to draw overlay
      damage1_ = 0;

    } else {

      damage1_ = save_damage;
      set_damage(DAMAGE_ALL);
      draw();
      if (overlay == this) draw_overlay();
      if (!(mode_ & NO_AUTO_SWAP)) swap_buffers();
      goto NO_OVERLAY;

    }

    if (overlay==this) { // fake overlay in front buffer
      glDrawBuffer(GL_FRONT);
      draw_overlay();
      glDrawBuffer(GL_BACK);
      glFlush();
    }
  NO_OVERLAY:;

  } else {	// single-buffered context is simpler:

    draw();
    if (overlay == this) draw_overlay();
    glFlush();
  }

#if USE_GL_OVERLAY && defined(_WIN32)
 OVERLAY_ONLY:
  // Draw into hardware overlay planes if they are damaged:
  if (overlay && overlay != this
      && (i->region || save_damage&DAMAGE_OVERLAY || !save_valid)) {
#if SGI320_BUG
    // SGI 320 messes up overlay with user-defined cursors:
    bool fixcursor = false;
    if (i->cursor && i->cursor != fl_default_cursor) {
      fixcursor = true; // make it restore cursor later
      SetCursor(0);
    }
#endif
    set_gl_context(this, (GLContext)overlay);
    if (fl_overlay_depth) wglRealizeLayerPalette(i->dc, 1, TRUE);
    glDisable(GL_SCISSOR_TEST);
    if (!(mode_&NO_ERASE_OVERLAY)) glClear(GL_COLOR_BUFFER_BIT);
    fl_overlay = true;
    valid(save_valid);
    draw_overlay();
    fl_overlay = false;
    wglSwapLayerBuffers(i->dc, WGL_SWAP_OVERLAY1);
#if SGI320_BUG
    if (fixcursor) SetCursor(i->cursor);
#endif
  }
#endif
  valid(1);
}

void GlWindow::layout() {
  if (layout_damage() & LAYOUT_WH) {
    valid(0);
#if USE_QUARTZ
    no_gl_context(); // because the BUFFER_RECT may change
#endif
#if USE_X11
    if (!resizable() && overlay && overlay != this)
      ((GlWindow*)overlay)->resize(0,0,w(),h());
#endif
  }
  Window::layout();
#if USE_X11
  glXWaitX();
#endif
}

/** \fn GLContext GlWindow::context() const;
  Return the current OpenGL context object being used by this window,
  or 0 if there is none.
*/
/** \fn void GlWindow::context(GLContext v, bool destroy_flag);
  Set the OpenGL context object to use to draw this window.

  This is a system-dependent structure (HGLRC on Windows,GLXContext on
  X, and AGLContext (may change) on OS/X), but it is portable to copy
  the context from one window to another. You can also set it to NULL,
  which will force FLTK to recreate the context the next time
  make_current() is called, this is useful for getting around bugs in
  OpenGL implementations.

  \a destroy_flag indicates that the context belongs to this window
  and should be destroyed by it when no longer needed. It will be
  destroyed when the window is destroyed, or when the mode() is
  changed, or if the context is changed to a new value with this call.
*/
void GlWindow::_context(void* v, bool destroy_flag) {
  if (context_ && context_ != v && !(mode_&NON_LOCAL_CONTEXT))
    delete_gl_context(context_);
  context_ = (GLContext)v;
  if (destroy_flag) mode_ &= ~NON_LOCAL_CONTEXT;
  else mode_ |= NON_LOCAL_CONTEXT;
}

/** Besides getting rid of the window, this will destroy the context
    if it belongs to the window. */
void GlWindow::destroy() {
  context(0);
#if USE_GL_OVERLAY
  if (overlay && overlay != this) {
#ifdef _WIN32
    delete_gl_context((GLContext)overlay);
#endif
    overlay = 0;
  }
#endif
  Window::destroy();
}

/** The destructor will destroy the context() if it belongs to the window. */
GlWindow::~GlWindow() {
  destroy();
}

/** \fn GlWindow::GlWindow(int x, int y, int w, int h, const char *label=0);
The constructor sets the mode() to RGB_COLOR|DEPTH_BUFFER|DOUBLE_BUFFER
which is probably all that is needed for most 3D OpenGL graphics.
*/

void GlWindow::init() {
  mode_ = DEPTH_BUFFER | DOUBLE_BUFFER;
  context_ = 0;
  gl_choice = 0;
  overlay = 0;
  damage1_ = 0;
}

/**
You must implement this virtual function if you want to draw into the
overlay.  The overlay is cleared before this is called (unless the
NO_ERASE_OVERLAY bit is set in the mode \e and hardware overlay is
supported). You should draw anything that is not clear using OpenGL.

If the hardware overlay is being used it will probably be color
indexed. You must use glsetcolor() to choose colors (it allocates them
from the colormap using system-specific calls), and remember that you
are in an indexed OpenGL mode and drawing anything other than
flat-shaded will probably not work.

Depending on the OS and whether or not the overlay is being simulated,
the context may be shared with the main window. This means if you
check valid() in draw() to avoid initialization, you must do so here
and initialize to exactly the same setting.
*/
void GlWindow::draw_overlay() {}

int GlWindow::handle( int event )
{
#if USE_QUARTZ
    switch ( event ) {
    case HIDE:
        aglSetDrawable( context(), NULL );
        break;
    }
#endif
    return Window::handle( event );
}

#endif

//
// End of "$Id: Fl_Gl_Window.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
