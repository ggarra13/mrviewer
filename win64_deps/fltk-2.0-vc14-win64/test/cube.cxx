//
// "$Id: cube.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// OpenGL test program for the Fast Light Tool Kit (FLTK).
//
// Modified to have 2 cubes to test multiple OpenGL contexts
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
#include <fltk/draw.h>
#include <fltk/Window.h>
#include <fltk/Box.h>
#include <fltk/Button.h>
#include <fltk/RadioLightButton.h>
#include <fltk/Slider.h>
#include <stdlib.h>

using namespace fltk;

#if !HAVE_GL
#include <fltk/ask.h>

int main(int, char**) {
  fltk::alert("This demo does not work without OpenGL");
  return 1;
}
#else
#include <fltk/GlWindow.h>
#include <fltk/gl.h>

class cube_box : public GlWindow {
  void draw();
public:
  double lasttime;
  int wire;
  double size;
  double speed;
  cube_box(int x,int y,int w,int h,const char *l=0)
    : GlWindow(x,y,w,h,l) {lasttime = 0.0;}
};

/* The cube definition */
float v0[3] = {0.0, 0.0, 0.0};
float v1[3] = {1.0, 0.0, 0.0};
float v2[3] = {1.0, 1.0, 0.0};
float v3[3] = {0.0, 1.0, 0.0};
float v4[3] = {0.0, 0.0, 1.0};
float v5[3] = {1.0, 0.0, 1.0};
float v6[3] = {1.0, 1.0, 1.0};
float v7[3] = {0.0, 1.0, 1.0};

#define v3f(x) glVertex3fv(x)

void drawcube(int wire) {
/* Draw a colored cube */
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(0,0,255);
  v3f(v0); v3f(v1); v3f(v2); v3f(v3);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(0,255,255); v3f(v4); v3f(v5); v3f(v6); v3f(v7);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(255,0,255); v3f(v0); v3f(v1); v3f(v5); v3f(v4);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(255,255,0); v3f(v2); v3f(v3); v3f(v7); v3f(v6);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(0,255,0); v3f(v0); v3f(v4); v3f(v7); v3f(v3);
  glEnd();
  glBegin(wire ? GL_LINE_LOOP : GL_POLYGON);
  glColor3ub(255,0,0); v3f(v1); v3f(v2); v3f(v6); v3f(v5);
  glEnd();
}

void cube_box::draw() {
  lasttime = lasttime+speed;
  if (!valid()) {
    glLoadIdentity();
    glViewport(0,0,w(),h());
    glEnable(GL_DEPTH_TEST);
    glFrustum(-1,1,-1,1,2,10000);
    glTranslatef(0,0,-10);
    glsetfont(HELVETICA_BOLD, 16 );
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
  glRotatef(float(lasttime*1.6),0,0,1);
  glRotatef(float(lasttime*4.2),1,0,0);
  glRotatef(float(lasttime*2.3),0,1,0);
  glTranslatef(-1.0, 1.2f, -1.5);
  glScalef(float(size),float(size),float(size));
  drawcube(wire);
  glPopMatrix();
  color(GRAY75);
  glDisable(GL_DEPTH_TEST);
  gldrawtext(wire ? "Cube: wire" : "Cube: flat", -4.5f, -4.5f );
  glEnable(GL_DEPTH_TEST);
}

Window *form;
Slider *speed, *size;
Button *button, *wire, *flat;
cube_box *cube, *cube2;

static void exit_cb(Widget*, void*) {
  form->hide();
}

void makeform(const char *name) {
  form = new Window(510+390,390,name);
  form->begin();
  Widget *box = new Widget(20,20,350,350,"");
  box->box(DOWN_BOX);
  box = new Widget(510,20,350,350,"");
  box->box(DOWN_BOX);
  speed = new Slider(390,90,40,220,"Speed");
  speed->set_vertical();
  size = new Slider(450,90,40,220,"Size");
  size->set_vertical();
  wire = new RadioLightButton(390,20,100,30,"Wire");
  flat = new RadioLightButton(390,50,100,30,"Flat");
  button = new Button(390,340,100,30,"Exit");
  button->callback(exit_cb);
  cube = new cube_box(23,23,344,344, 0);
  cube2 = new cube_box(513,23,344,344, 0);
  Widget *b = new Widget(cube->x(),size->y(), cube->w(),size->h(),0);
  b->box(NO_BOX);
  form->resizable(b);
  b->hide();
  form->end();
}

/* 
 * It should be noted that it is not a good practice to have an idle 
 * callback in place at all times because it will eat all available 
 * processor time. A timout callback would be a better solution.
 * However, "idle" is used in this particular test program to show
 * maximum OpenGL performance.
 */

void idle_cb(void*) {
  cube->wire = wire->value();
  cube2->wire = !wire->value();
  cube->size = cube2->size = size->value();
  cube->speed = cube2->speed = speed->value();
  cube->redraw();
  cube2->redraw();
}

int main(int argc, char **argv) {
  makeform(argv[0]);
  speed->range(0,4);
  speed->value(cube->speed = cube2->speed = 1.0);
  size->range(0.01,4);
  size->value(cube->size = cube2->size = 1.0);
  flat->value(1); cube->wire = 0; cube2->wire = 1;
  form->label("cube");
  form->show(argc,argv);
  cube->show();
  cube2->show();
#if 0
  // This demonstrates how to manipulate OpenGL contexts.
  // In this case the same context is used by multiple windows (I'm not
  // sure if this is allowed on Win32, can somebody check?).
  // This fixes a bug on the XFree86 3.0 OpenGL where only one context
  // per program seems to work, but there are probably better uses for
  // this!
  cube->make_current(); // causes context to be created
  cube2->context(cube->context()); // share the contexts
#endif
  fltk::add_idle(idle_cb, 0);
  fltk::run();
  return 0;
}

#endif

//
// End of "$Id: cube.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
