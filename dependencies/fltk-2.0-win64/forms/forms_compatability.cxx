//
// "$Id: forms_compatability.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Forms compatibility functions for the Fast Light Tool Kit (FLTK).
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

// Functions needed to emulate XForms/Forms using fltk.
// Many more functions are defined as inlines in forms.h!

#include <fltk/forms.h>
#include <stdlib.h>

void fl_set_object_lstyle(Fl_Widget* o,int a) {
  o->label_font(fl_fonts+(a&15));
  switch (a >> 8) {
  case 0:
    o->label_type(FL_NORMAL_LABEL);
    break;
  case 1:
    o->label_type(FL_SHADOW_LABEL);
    break;
  case 2:
    o->label_type(FL_ENGRAVED_LABEL);
    break;
  case 3:
    o->label_type(FL_EMBOSSED_LABEL);
    break;
  }
}

// Emulate old forms (not XForms) inverted coordinate system.  Like
// XForms, this emulation is shut off if fl_initialize() is called
// before the widgets are created:

char fl_flip = 2;

void fl_end_form() {
  while (Fl_Group::current()) fl_end_group();
}

void fl_end_group() {
  Fl_Group* group = Fl_Group::current();
  // set the dimensions of a group to surround contents
  if (group->children() && !group->w()) {
    Fl_Widget* o = group->child(0);
    int rx = o->x();
    int ry = o->y();
    int rw = rx+o->w();
    int rh = ry+o->h();
    for (int i = 1; i < group->children(); i++) {
      o = group->child(i);
      if (o->x() < rx) rx = o->x();
      if (o->y() < ry) ry = o->y();
      if (o->x()+o->w() > rw) rw = o->x()+o->w();
      if (o->y()+o->h() > rh) rh = o->y()+o->h();
    }
    group->x(rx);
    group->y(ry);
    group->w(rw-rx);
    group->h(rh-ry);
  }
  // flip all the children's coordinate systems:
  if (fl_flip) {
    Fl_Widget* o = group->is_window() ? group : group->window();
    int Y = o->h();
    for (int i = 0; i < group->children(); i++) {
      Fl_Widget* o = group->child(i);
      o->y(Y-o->y()-o->h());
    }
//    group->oy_ = Y-group->oy_-group->h();
  }
  group->fix_old_positions();
  group->end();
}

static int initargc;
static char **initargv;

void fl_initialize(int *argc, char **argv, const char *, FL_CMD_OPT *, int) {
  initargc = *argc;
  initargv = new char*[*argc+1];
  int i,j;
  for (i=0; i<=*argc; i++) initargv[i] = argv[i];
  for (i=j=1; i<*argc; ) {
    if (Fl::arg(*argc,argv,i));
    else argv[j++] = argv[i++];
  }
  argv[j] = 0;
  *argc = j;
  if (fl_flip==2) fl_flip = 0;
}

// Emulate fl_show_form().  The "placement" bitflag causes all kinds of
// different fltk functions to be called:

char fl_modal_next; // set by fl_freeze_forms()

void fl_show_form(Fl_Window *f,int place,int b,const char *n) {

  Fl_Widget::default_style->label_type = FL_SYMBOL_LABEL;

  f->label(n);
  if (!b) f->clear_border();
  if (fl_modal_next || b==FL_TRANSIENT) {f->set_modal(); fl_modal_next = 0;}

  if (place & FL_PLACE_MOUSE) f->hotspot(f);

  if (place & FL_PLACE_CENTER)
    f->position((Fl::w()-f->w())/2, (Fl::h()-f->h())/2);

  if (place & FL_PLACE_FULLSCREEN)
    f->fullscreen();

  if (place & (FL_PLACE_POSITION | FL_PLACE_GEOMETRY))
    f->position(
      (f->x() < 0) ? Fl::w()-f->w()+f->x()-1 : f->x(),
      (f->y() < 0) ? Fl::h()-f->h()+f->y()-1 : f->y());

// if (place & FL_PLACE_ASPECT) {
// this is not yet implemented
// it can be done by setting size_range().

  if (place == FL_PLACE_FREE || place == FL_PLACE_SIZE) {
    f->x(FL_USEDEFAULT); f->y(FL_USEDEFAULT);
  }

  if (place == FL_PLACE_FREE || place & FL_FREE_SIZE)
    if (!f->resizable()) f->resizable(f);

  if (initargc) {f->show(initargc,initargv); initargc = 0;}
  else f->show();
}

// Emulate the event-getting routines.
// XForms returns any widget without a callback that is activated, or
// returns zero if the last window is closed.
// Since all widgets in fltk have a callback, this would never return.
// However most programs use this like Fl::wait(), they rely on this
// returning quite often. So I copy that behavior. To allow programs
// that exit when this returns zero to work, I return the first window
// each time, this is zero only when all windows are closed.

Fl_Widget *fl_do_forms(void) {
  Fl::wait();	
  return Fl::first_window();
}

Fl_Widget *fl_check_forms() {
  Fl::check();
  return Fl::first_window();
}

// Subclass to simulate the XForms text object.  This is the same as
// a Fl_Box except that the label is drawn inside it:

void Fl_FormsText::draw() {
  draw_box();
  set_flag(flags()|FL_ALIGN_INSIDE); // questionable method of compatability
  draw_inside_label();
}

// Create an XForms button by selecting correct fltk subclass:

#include <fltk/Fl_Return_Button.h>
#include <fltk/Fl_Repeat_Button.h>

Fl_Button *fl_add_button(uchar t,int x,int y,int w,int h,const char *l) {
  Fl_Button *b;
  switch (t) {
  case FL_RETURN_BUTTON:
  case FL_HIDDEN_RET_BUTTON:
    b = new Fl_Return_Button(x,y,w,h,l);
    break;
  case FL_TOUCH_BUTTON:
    b = new Fl_Repeat_Button(x,y,w,h,l);
    break;
  default:
    b = new Fl_Button(x,y,w,h,l);
  }
  switch (t) {
  case FL_TOGGLE_BUTTON:
  case FL_RADIO_BUTTON:
    b->type(t);
    break;
#if 0
  case FL_HIDDEN_BUTTON:
  case FL_HIDDEN_RET_BUTTON:
    b->type(FL_HIDDEN_BUTTON);
    break;
#endif
  case FL_INOUT_BUTTON:
    b->when(FL_WHEN_CHANGED);
    break;
  }
  return b;
}

// Convert an XForms shortcut string name to an fltk integer shortcut:

#include <stdlib.h>
int fl_old_shortcut(const char* s) {
  if (!s || !*s) return 0;
  int n = 0;
  if (*s == '#') {n |= FL_ALT; s++;}
  if (*s == '+') {n |= FL_SHIFT; s++;}
  if (*s == '^') {n |= FL_CTRL; s++;}
  if (s[1]) return n | (int)strtol(s,0,0); // allow 0xf00 to get any key
  return n | *s;
}

// Wrappers for the popup message utilities to convert from the XForms
// 3-argument versions to the sprintf versions used by fltk:

void fl_show_message(const char *q1,const char *q2,const char *q3) {
  fl_message("%s\n%s\n%s", q1?q1:"", q2?q2:"", q3?q3:"");
}

void fl_show_alert(const char *q1,const char *q2,const char *q3,int) {
  fl_alert("%s\n%s\n%s", q1?q1:"", q2?q2:"", q3?q3:"");
}

int fl_show_question(const char *q1,const char *q2,const char *q3) {
  return fl_ask("%s\n%s\n%s", q1?q1:"", q2?q2:"", q3?q3:"");
}

int fl_show_choice(
  const char *q1,
  const char *q2,
  const char *q3,
  int, // number of buttons, ignored
  const char *b0,
  const char *b1,
  const char *b2) {
  return fl_choice("%s\n%s\n%s", q1?q1:"", q2?q2:"", q3?q3:"", b0,b1,b2)+1;
}

char *fl_show_simple_input(const char *str1, const char *defstr) {
  const char *r = fl_input(str1, defstr);
  return (char *)(r ? r : defstr);
}

//
// End of "$Id: forms_compatability.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
