//
// "$Id: GlWindow.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// OpenGL window. You must subclass this and implement draw() if
// you want this to work.
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

#ifndef fltk_GlWindow_h
#define fltk_GlWindow_h

#include "Window.h"

namespace fltk {

#ifndef GLContext // you can define this to the correct type if wanted
typedef void* GLContext; //!< Actually a GLXContext or HGLDC
#endif

class GlChoice; // structure to hold result of glXChooseVisual
class GlOverlay; // used by X version for the overlay

enum {
  NO_AUTO_SWAP = 1024,
  NO_ERASE_OVERLAY = 2048
};

class FL_GL_API GlWindow : public Window {

public:

  void create();
  void flush();
  void destroy();
  void layout();

  char valid() const {return valid_;}
  void valid(char i) {valid_ = i;}
  void invalidate();

  int mode() const {return mode_;}
  bool mode(int a);
  static bool can_do(int);
  bool can_do() const {return can_do(mode_);}

  GLContext context() const {return context_;}
  // this wrapper is so c++mangled name does not depend on GLContext type:
  void context(GLContext v, bool destroy_flag = false) {_context(v,destroy_flag);}
  void make_current();
  void swap_buffers();
  void ortho();

  bool can_do_overlay();
  void redraw_overlay();
  void hide_overlay();
  void make_overlay_current();

  ~GlWindow();
  GlWindow(int W, int H, const char *l=0) : Window(W,H,l) {init();}
  GlWindow(int X, int Y, int W, int H, const char *l=0)
    : Window(X,Y,W,H,l) {init();}

  virtual void draw() = 0;
  virtual void draw_overlay();

  virtual int handle( int event );

private:

  int mode_;
  GlChoice *gl_choice;
  GLContext context_;
  void _context(void*, bool destroy_flag);
  char valid_;
  char damage1_; // damage() of back buffer
  void init();

  void *overlay;
  void make_overlay();
  friend class GlOverlay;

  void draw_swap();
};

}

#endif

//
// End of "$Id: GlWindow.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
