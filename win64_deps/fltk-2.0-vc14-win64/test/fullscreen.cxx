//
// "$Id: fullscreen.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Fullscreen test program for the Fast Light Tool Kit (FLTK).
//
// This demo shows how to do many of the window manipulations that
// are popular on SGI programs, even though X does not really like
// them.  You can toggle the border on/off, change the visual to
// switch between single/double buffer, and make the window take
// over the screen.
//
// Normally the program makes a single window with a child GL window.
// This simulates a program where the 3D display is surrounded by
// control knobs.  Running the program with an argument will
// make it make a seperate GL window from the controls window.  This
// simulates a (older?) style program where the graphics display is
// a different window than the controls.
//
// This program reports how many times it redraws the window to
// stdout, so you can see how much time it is wasting.  It appears
// to be impossible to prevent X from sending redundant resize
// events, so there are extra redraws.  But the way I have the
// code arranged here seems to be keeping that to a minimu.
//
// Apparently unavoidable bugs:
//
// Turning the border on causes an unnecessary redraw.
//
// Turning off full screen when the border is on causes an unnecessary
// resize and redraw when the program turns the border on.
//
// If it is a seperate window, turning double buffering on and off
// will cause the window to raise, deiconize, and possibly move.  You
// can avoid this by making the Fl_Gl_Window a child of a normal
// window.
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
#include <fltk/error.h>
#include <fltk/Window.h>
#include <fltk/Slider.h>
#include <fltk/LightButton.h>
#include <fltk/math.h>

#include <stdio.h>
#include <stdlib.h>

#if HAVE_GL

#include <fltk/gl.h>
#include <fltk/visual.h>
#include <fltk/GlWindow.h>

class shape_window : public fltk::GlWindow {
  void draw();
public:
  int sides;
  shape_window(int x,int y,int w,int h,const char *l=0);
};

shape_window::shape_window(int x,int y,int w,int h,const char *l) :
fltk::GlWindow(x,y,w,h,l) {
  sides = 3;
}

void shape_window::draw() {
  printf("drawing size %d %d\n",w(),h());
  if (!valid()) {
    valid(1);
//  printf("init\n");
    glLoadIdentity();
    glViewport(0,0,w(),h());
  }
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(.5,.6,.7);
  glBegin(GL_POLYGON);
  for (int i=0; i<sides; i++) {
    double ang = i*2*M_PI/sides;
    glVertex3f(cos(ang),sin(ang),0);
  }
  glEnd();

  fltk::glsetcolor(fltk::WHITE);
  fltk::glsetfont(fltk::HELVETICA, 12); 
  fltk::gldrawtext("Using OpenGL", -.9, -.9);
}

#else

#include <fltk/draw.h>

class shape_window : public fltk::Window {
  void draw();
public:
  int sides;
  shape_window(int x,int y,int w,int h,const char *l=0);
};

shape_window::shape_window(int x,int y,int w,int h,const char *l)
: fltk::Window(x,y,w,h,l) {
  sides = 3;
}

void shape_window::draw() {
  printf("drawing size %d %d\n",w(),h());
  fltk::setcolor(fltk::BLACK);
  fltk::fillrect(0,0,w(),h());

  fltk::setcolor(fltk::BLUE);
  fltk::push_matrix();
  fltk::translate(w()/2, h()/2);
  fltk::scale(w()/2, h()/2);    
  for (int i=0; i<sides; i++) {
    double ang = i*2*M_PI/sides;
    fltk::addvertex((float)cos(ang), (float)sin(ang));
  }
  fltk::closepath();
  fltk::pop_matrix();
  fltk::fillstrokepath(fltk::WHITE);

  fltk::setcolor(fltk::WHITE);
  fltk::setfont(fltk::HELVETICA, 12); 
  fltk::drawtext("Not using OpenGL",1,fltk::getascent());
}

#endif

void sides_cb(fltk::Widget *o, void *p) {
  shape_window *sw = (shape_window *)p;
  sw->sides = int(((fltk::Slider *)o)->value());
  sw->redraw();
}

fltk::Window *control_win = 0;

void double_cb(fltk::Widget *o, void *p) {
  shape_window *sw = (shape_window *)p;
  int d = ((fltk::Button *)o)->value();
#if HAVE_GL
  sw->mode(d ? (fltk::DOUBLE_BUFFER|fltk::RGB_COLOR) : fltk::RGB_COLOR);
  // We must show "control_win" if running in 2 window mode.
  // GlWindow::mode will destroy GL window and also control window, 
  // cause it's "child_of()" GL window. 
  if (control_win) control_win->show(); 
#else
  if(d) sw->set_double_buffer();
  else  sw->clear_double_buffer();
#endif
}

// This is useless in current FLTK2...
void border_cb(fltk::Widget *o, void *p) {
  //fltk::Window *w = (fltk::Window *)p;
  //int d = ((fltk::Button *)o)->value();
  //w->setborder(d);
}

int px,py,pw,ph;
fltk::Button *border_button;
fltk::Window *fullscreen = 0;

void fullscreen_cb(fltk::Widget *o, void *p) {
  fltk::Window *w = (fltk::Window *)p;
  int d = ((fltk::Button *)o)->value();
  if (d) {
    // Store current geometry of the window
    px = w->x(); py = w->y();
    pw = w->w(); ph = w->h();
    w->fullscreen();
    fullscreen = w;
  } else {
    // restore geometry to the window and turn fullscreen off
    w->fullscreen_off(px,py,pw,ph);
    fullscreen = 0;
  }
}

void maximize_cb(fltk::Widget *o, void *p) {
  fltk::Window *w = (fltk::Window *)p;
  w->maximize();
}

void exit_cb(fltk::Widget *, void *) {
  // Turn fullscreen off when exit
  if(fullscreen)
    fullscreen->fullscreen_off(px,py,pw,ph);
  exit(0);
}

#define NUMB 5

int twowindow = 0;
int initfull = 0;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == '2') {twowindow = 1; i++; return 1;}
  if (argv[i][1] == 'f') {initfull = 1; i++; return 1;}
  return 0;
}

int main(int argc, char **argv) {

  int i=0;
  if (fltk::args(argc,argv,i,arg) < argc)
    fltk::fatal("Options are:\n -2 = 2 windows\n -f = startup fullscreen\n%s", fltk::help);

  fltk::Window window(300, (twowindow?10:300)+30*NUMB); window.end();
  window.callback(exit_cb);
  
  if(twowindow)
    control_win = &window;

  shape_window sw(10, 10, window.w()-20, (twowindow?400:window.h())-30*NUMB-20);
#if HAVE_GL
  sw.mode(fltk::RGB_COLOR);
#endif

  fltk::Window *w;
  if (twowindow) {	// make it's own window
    sw.resizable(&sw);
    w = (fltk::Window*)(&sw); // cast added for fltk2.0
    window.child_of(w); // makes controls stay on top when fullscreen pushed
    argc--;
    sw.show();
  } else {		// otherwise make a subwindow
    window.add(sw);
    window.resizable(&sw);
    w = &window;
  }

  window.begin();

  int y = window.h()-30*NUMB-5;
  fltk::Slider slider(50,y,window.w()-60,30,"Sides:");
  slider.align(fltk::ALIGN_LEFT);
  slider.callback(sides_cb,&sw);
  slider.value(sw.sides);
  slider.step(1);
  slider.range(3,40);
  y+=30;

  fltk::LightButton b1(50,y,window.w()-60,30,"Double Buffered");
  b1.callback(double_cb,&sw);
  y+=30;

  /*fltk::LightButton b2(50,y,window.w()-60,30,"Border");
  b2.callback(border_cb,w);
  b2.set();
  border_button = &b2;
  y+=30;*/

  fltk::LightButton b3(50,y,window.w()-60,30,"FullScreen");
  b3.callback(fullscreen_cb,w);
  y+=30;

  fltk::Button btnMaximize(50,y,window.w()-60,30,"Maximize me!");
  btnMaximize.callback(maximize_cb,&window);
  y+=30;

  fltk::Button eb(50,y,window.w()-60,30,"Exit");
  eb.callback(exit_cb);
  y+=30;

  if (initfull) {b3.set(); b3.do_callback();}

  window.end();
  window.show(argc,argv);

  return fltk::run();
}

//
// End of "$Id: fullscreen.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
