//
// "$Id: gl_overlay.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// OpenGL overlay test program for the Fast Light Tool Kit (FLTK).
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
#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Slider.h>
#include <fltk/ToggleButton.h>
#include <fltk/math.h>

#if !HAVE_GL

class shape_window : public fltk::Widget {
public:	
  int sides;
  shape_window(int x,int y,int w,int h,const char *l=0)
    :fltk::Widget(x,y,w,h,l){
      label("This demo does\nnot work without GL");
  }
};
#else
#include <fltk/gl.h>
#include <fltk/GlWindow.h>

class shape_window : public fltk::GlWindow {
  void draw();
  void draw_overlay();
public:
  int sides;
  int overlay_sides;
  shape_window(int x,int y,int w,int h,const char *l=0);
};

shape_window::shape_window(int x,int y,int w,int h,const char *l)
  : fltk::GlWindow(x,y,w,h,l)
{
  sides = overlay_sides = 3;
}

#include <stdio.h>

void shape_window::draw() {
// the valid() property may be used to avoid reinitializing your
// GL transformation for each redraw:
  if (!valid()) {
    static int r;
    printf("Invalid %d\n", ++r);
    valid(1);
    glLoadIdentity();
    glViewport(0,0,w(),h());
  }
  glClear(GL_COLOR_BUFFER_BIT);
  glBegin(GL_POLYGON);
  for (int i=0; i<sides; i++) {
    double ang = i*2*M_PI/sides;
    glColor3f(float(i)/sides,float(i)/sides,float(i)/sides);
    glVertex3f(cos(ang),sin(ang),0);
  }
  glEnd();
  fltk::glsetcolor(fltk::WHITE);
  fltk::glsetfont(labelfont(), labelsize());
  static int count = 0;
  count++;
  char buffer[40];
  sprintf(buffer, "normal draw %d", count);
  fltk::gldrawtext(buffer, .1, .5);
}

void shape_window::draw_overlay() {
// the valid() property may be used to avoid reinitializing your
// GL transformation for each redraw:
  if (!valid()) {
    valid(1);
    glLoadIdentity();
    glViewport(0,0,w(),h());
  }
// draw an amazing graphic:
  fltk::glsetcolor(fltk::RED);
  glBegin(GL_LINE_LOOP);
  for (int i=0; i<overlay_sides; i++) {
    double ang = i*2*M_PI/overlay_sides;
    glVertex3f(cos(ang),sin(ang),0);
  }
  glEnd();
  fltk::glsetfont(labelfont(), labelsize());
  static int count = 0;
  count++;
  char buffer[40];
  sprintf(buffer, "overlay draw %d", count);
  fltk::gldrawtext(buffer, .1, .6);
}
#endif

// when you change the data, as in this callback, you must call redraw():
void sides_cb(fltk::Widget *o, void *p) {
  shape_window *sw = (shape_window *)p;
  sw->sides = int(((fltk::Slider *)o)->value());
  sw->redraw();
}

#if HAVE_GL
void overlay_sides_cb(fltk::Widget *o, void *p) {
  shape_window *sw = (shape_window *)p;
  sw->overlay_sides = int(((fltk::Slider *)o)->value());
  sw->redraw_overlay();
}
#endif

int main(int argc, char **argv) {

  fltk::Window window(300, 370);
  window.begin();

  shape_window sw(10, 75, window.w()-20, window.h()-90);
//sw.mode(FLTK::RGB);
  window.resizable(&sw);

  fltk::Slider oslider(60, 5, window.w()-70, 30, "Overlay:");
  oslider.clear_flag(fltk::ALIGN_MASK);
  oslider.set_flag(fltk::ALIGN_LEFT);
#if HAVE_GL
  oslider.callback(overlay_sides_cb,&sw);
  oslider.value(sw.overlay_sides);
#endif
  oslider.step(1);
  oslider.range(3,40);
  oslider.tooltip("Move this slider to make overlay redraw");

  fltk::Slider slider(60, 40, window.w()-70, 30, "Normal:");
  slider.clear_flag(fltk::ALIGN_MASK);
  slider.set_flag(fltk::ALIGN_LEFT);
  slider.callback(sides_cb,&sw);
  slider.value(sw.sides);
  slider.step(1);
  slider.range(3,40);
  oslider.tooltip("Move this slider to make background image redraw");

  window.end();
  window.show(argc,argv);

#if HAVE_GL
  sw.tooltip(sw.can_do_overlay() ?

"The red image is drawn by implementing fltk::GlWindow::draw_overlay().\n"
"\n"
"Hardware overlays may be unable to draw some "
"colors or may exhibit bugs. If you see the red outline but no red text "
"this is a known bug on Windows."
:
"The red image is drawn by implementing fltk::GlWindow::draw_overlay().\n"
"\n"
"You do not have OpenGL overlay hardware, so fltk "
"simulates it. To work on all OpenGl hardware the simulation is very "
"simple by default. You can reduce redraws of the background by changing the "
"environment variable $GL_SWAP_TYPE:\n"
"\n"
"setenv GL_SWAP_TYPE USE_COPY (almost always works)\n"
"setenv GL_SWAP_TYPE COPY (works on some cards)\n"
"setenv GL_SWAP_TYPE NODAMAGE (works only with software OpenGl)"
	     );
  sw.show();
  sw.redraw_overlay();
#else
  sw.show();
#endif

  return fltk::run();
}

//
// End of "$Id: gl_overlay.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
