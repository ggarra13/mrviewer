//
// "$Id: WidgetType.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Widget type code for the Fast Light Tool Kit (FLTK).
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

#include "Enumeration.h"
#include "WidgetType.h"

#include <fltk/Window.h>
#include <fltk/run.h>
#include <fltk/Group.h>
#include <fltk/ask.h>
#include <fltk/Slider.h>
#include <fltk/DoubleBufferWindow.h>
#include <fltk/InvisibleBox.h>
#include <fltk/Font.h>
#include <fltk/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "PrefsData.h"
#include "coding_style.h"
#include "undo.h"

using namespace fltk;

int WidgetType::is_widget() const {return 1;}

const char *WidgetType::type_name() const {return "fltk::Widget";}

Widget *WidgetType::widget(int x,int y,int w, int h) {
  return new Widget(x,y,w,h,"label");
}

WidgetType *WidgetType::_make() {return new WidgetType();}

const char* WidgetType::subclass() const {
  // first return any text class name the user put in:
  if (user_class_) return user_class_;
  // then return any class based on the "type()", this is any
  // entry in the type table where the code starts with "Fl":
  const Enumeration* table = subtypes();
  if (table) {
    const Enumeration* e = from_value(o->type(), table);
    if (e && e->subclass) return e->subclass;
  }
  return type_name();
}

// Make an WidgetType subclass instance.
// It figures out the automatic size and parent of the new widget,
// creates the fltk::Widget (by calling the virtual function _make),
// adds it to the fltk::Widget hierarchy, creates a new FluidType
// instance, sets the widget pointers, and makes all the display
// update correctly...

extern int reading_file;
int force_parent;

#include <fltk/StyleSet.h>
extern fltk::StyleSet* fluid_style_set;
extern fltk::StyleSet* style_set;

FluidType *WidgetType::make() {
  // Find the current widget, or widget to copy:
  FluidType *qq = FluidType::current;
  while (qq && !qq->is_widget()) qq = qq->parent;
  if (!qq) {
    fltk::message("Please select a widget");
    return 0;
  }
  WidgetType* q = (WidgetType*)qq;
  // find the parent widget:
  WidgetType* p = q;
  if ((force_parent || !p->o->is_group()) && p->parent->is_widget())
    p = (WidgetType*)(p->parent);
  force_parent = 0;

  // Figure out a border between widget and window:
  int B = p->o->w()/2; if (p->o->h()/2 < B) B = p->o->h()/2; if (B>25) B = 25;

  // Figure out a position and size for the widget
  int X,Y,W,H;
  if (is_group()) {	// fill the parent with the widget
    X = B;
    W = p->o->w()-B;
    Y = B;
    H = p->o->h()-B;
  } else if (q != p) {	// copy position and size of current widget
    W = q->o->w();
    H = q->o->h();
    X = q->o->x()+W;
    Y = q->o->y();
    if (X+W > p->o->w()) {
      X = q->o->x();
      Y = q->o->y()+H;
      if (Y+H > p->o->h()) Y = B;
    }
  } else {	// just make it small and square...
    X = B;
    Y = B;
    W = H = B;
  }

  // satisfy the grid requirements (otherwise it edits really strangely):
  if (prefs.gridx()>1) {
    X = (X/prefs.gridx())*prefs.gridx();
    W = ((W-1)/prefs.gridx()+1)*prefs.gridx();
  }
  if (prefs.gridy()>1) {
    Y = (Y/prefs.gridy())*prefs.gridy();
    H = ((H-1)/prefs.gridy()+1)*prefs.gridy();
  }

  // Construct the FluidType:
  style_set->make_current();
  WidgetType *t = _make();
  if (!o) o = widget(0,0,100,100); // create template widget
  t->factory = this;
  // Construct the fltk::Widget:
  t->o = widget(X,Y,W,H);
  if (H>W) t->o->set_vertical();
  fluid_style_set->make_current();
  if (reading_file) t->o->label(0);
  else if (t->o->label()) t->label(t->o->label()); // allow editing
  t->o->user_data((void*)t);
  // Put it in the parent:
  //  ((fltk::Group *)(p->o))->add(t->o); (done by FluidType::add())
  // add to browser:
  t->add(p);
  t->redraw();
  return t;
}

#include "Fluid_Image.h"

float WidgetType::default_size = 14.0;

void WidgetType::set_image(Fluid_Image *i) {
  if (i == image) return;
  if (image) image->decrement();
  if (i) i->increment();
  image = i;
  o->image(i ? i->symbol() : 0);
  redraw();
}

void WidgetType::setlabel(const char *n) {
  o->label(n);
  redraw();
}

WidgetType::WidgetType() {
  extra_code_ = 0;
  user_class_ = 0;
  hotspot_ = 0;
  o = 0;
  default_size = 0.0;
  image = 0;
  public_ = 1;
  set_xy = true;
}

WidgetType::~WidgetType() {
  if (o) {
    o->hide();
    if (o->parent()) ((fltk::Group*)o->parent())->remove(*o);
    delete o;
  }
}

void WidgetType::extra_code(const char *n) {
  storestring(n,extra_code_);
}

extern void redraw_browser();
void WidgetType::user_class(const char *n) {
  if (storestring(n,user_class_)) redraw_browser();
}

void WidgetType::redraw() {
  FluidType *t = this;
  if (is_menu_item()) {
    // find the menu button that parents this menu:
    do t = t->parent; while (t && t->is_menu_item());
  } else {
    while (t->parent && t->parent->is_widget()) t = t->parent;
  }
  if (t) ((WidgetType*)t)->o->redraw();
}

// sorts all selected children by position:
void sort(FluidType *parent) {
  for (FluidType* f = parent->first_child; f;) {
    sort(f);
    FluidType* next = f->next_brother;
    if (f->selected() && (f->is_widget() && !f->is_menu_item())) {
      fltk::Widget* fw = ((WidgetType*)f)->o;
      FluidType *g; // we will insert before this
      for (g = parent->first_child; g != f; g = g->next_brother) {
	if (!g->selected()) continue;
	fltk::Widget* gw = ((WidgetType*)g)->o;
	if (gw->y() >= fw->y()+fw->h()) break;
	if (gw->y()+gw->h() > fw->y() && gw->x() >= fw->x()+fw->w()) break;
	if (gw->y() >= fw->y() && gw->x() > fw->x()) break;
      }
      if (g != f) f->move_before(g);
    }
    f = next;
  }
}

////////////////////////////////////////////////////////////////
// The control panels!

#include "widget_panel.h"
#include <fltk/ColorChooser.h>

// All the callbacks use the argument to indicate whether to load or store.
// This avoids the need for pointers to all the widgets, and keeps the
// code localized in the callbacks.
// A value of LOAD means to load.  The hope is that this will not collide
// with any actual useful values for the argument.  I also use this to
// initialized parts of the widget that are nyi by fluid.

WidgetType *current_widget; // one of the selected ones
static int numselected; // number selected
static int haderror;
static fltk::Window *the_panel;

void name_cb(fltk::Input* o, void *v) {
  if (v == LOAD) {
    if (numselected != 1) {
      static char buf[16];
      sprintf(buf,"(%d widgets)",numselected);
      o->window()->label(buf);
      o->hide();
    } else {
      o->static_value(current_widget->name());
      o->show();
      o->window()->label(current_widget->title());
    }
  } else {
    if (numselected == 1) {
      current_widget->name(o->value());
      // I don't update window title, as it probably is being closed
      // and wm2 (a window manager) barfs if you retitle and then
      // hide a window:
      // o->window()->label(current_widget->title());
    }
  }
  fltk::Color c = fltk::BLACK;
  if (current_widget->name() && *(current_widget->name())) c = fltk::RED;
  if (o->labelcolor() != c)
    { o->labelcolor(c); o->redraw_label(); }
}

#define for_all_selected_widgets() \
for (FluidType *o = FluidType::first; o; o = o->walk()) \
  if (o->selected() && o->is_widget())

void name_public_cb(fltk::CheckButton* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->public_);
  } else {
    for_all_selected_widgets() {
      modflag = 1;
      ((WidgetType*)o)->public_ = i->value();
    } 	
  }
  if (!i->value()) i->labelcolor(fltk::RED);
  else i->labelcolor(fltk::BLACK);
  i->redraw();
}    

static char* oldlabel;
static unsigned oldlabellen;

void label_cb(fltk::Input* i, void *v) {
  if (v == LOAD) {
    i->static_value(current_widget->label());
    if (strlen(i->value()) >= oldlabellen) {
      oldlabellen = strlen(i->value())+128;
      oldlabel = (char*)realloc(oldlabel,oldlabellen);
    }
    strcpy(oldlabel,i->value());
  } else {
    for_all_selected_widgets() o->label(i->value());
  }
  fltk::Color c = fltk::BLACK;
  if (current_widget->label() && *(current_widget->label())) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

static char* oldtooltip;
static unsigned oldtooltiplen;

void tooltip_cb(fltk::WordwrapInput* i, void *v) {
  if (v == LOAD) {
    //if (current_widget->o->is_window()) { i->hide(); return; }
    i->show();
    i->static_value(current_widget->tooltip());
    if (strlen(i->value()) >= oldtooltiplen) {
      oldtooltiplen = strlen(i->value())+128;
      oldtooltip = (char*)realloc(oldtooltip,oldtooltiplen);
    }
    strcpy(oldtooltip,i->value());
  } else {
    for_all_selected_widgets() o->tooltip(i->value());
  }
  fltk::Color c = fltk::BLACK;
  if (current_widget->tooltip() && *(current_widget->tooltip())) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void x_cb(fltk::ValueInput* i, void *v) {
  int x;
  if (v != LOAD) {
    x = int(i->value());
    if (x <= -1) x = -1;
    for_all_selected_widgets() {
      WidgetType* q = (WidgetType*)o;
      q->o->position(x, q->o->y());
      q->redraw();
    }
  } else {
    x = current_widget ? current_widget->o->x() : 0;
  }
  i->value(x);
}

void y_cb(fltk::ValueInput* i, void *v) {
  int y;
  if (v != LOAD) {
    y = int(i->value());
    if (y <= -1) y = -1;
    for_all_selected_widgets() {
      WidgetType* q = (WidgetType*)o;
      q->o->position(q->o->x(), y);
      q->redraw();
    }
  } else {
    y = current_widget ? current_widget->o->y() : 0;    
  }
  i->value (y);
}

void width_cb(fltk::ValueInput* i, void *v) {
  int width;
  if (v != LOAD) {
    width = int(i->value());
    if (width <= 0) width = 0;
    for_all_selected_widgets() {
      WidgetType* q = (WidgetType*)o;
      q->o->resize(width, q->o->h());
      if (!o->is_menu_item()){
	if (q->o->w() > q->o->h()) q->o->set_horizontal();
	else if (q->o->w() < q->o->h()) q->o->set_vertical();
      }
      q->redraw();
    }
  } else {
    width = current_widget ? current_widget->o->w() : 0;
  }
  i->value (width);
}

void height_cb(fltk::ValueInput* i, void *v) {
  int height;
  if (v != LOAD) {
    height = int(i->value());
    if (height <= 0) height = 0;
    for_all_selected_widgets() {
      WidgetType* q = (WidgetType*)o;
      q->o->resize(q->o->w(), height);
      if (!o->is_menu_item()){
	if (q->o->w() > q->o->h()) q->o->set_horizontal();
	else if (q->o->w() < q->o->h()) q->o->set_vertical();
      }
      q->redraw();
    }  
  } else {
    height = current_widget ? current_widget->o->h() : 0;  
  }
  i->value (height);
}

void set_xy_cb(fltk::CheckButton* i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_window()) i->show(); else i->hide();
    i->value(current_widget->set_xy);
  } else {
    modflag = 1;
    current_widget->set_xy = i->value();  
  }
  if (!current_widget->set_xy) {
    widget_x->deactivate ();
    widget_y->deactivate ();  
  } else {
    widget_x->activate ();
    widget_y->activate ();  
  }
  if (i->value()) i->labelcolor(fltk::RED);
  else i->labelcolor(fltk::BLACK);  
  i->redraw();
}

////////////////////////////////////////////////////////////////

static const Enumeration boxmenu[] = {
{"None",	"NO_BOX",	(void *)fltk::NO_BOX},
{"Up",		"UP_BOX",	(void *)fltk::UP_BOX},
{"Down",	"DOWN_BOX",	(void *)fltk::DOWN_BOX},
{"Thin Up",	"THIN_UP_BOX",	(void *)fltk::THIN_UP_BOX},
{"Thin Down",	"THIN_DOWN_BOX",(void *)fltk::THIN_DOWN_BOX},
{"Flat",	"FLAT_BOX",	(void *)fltk::FLAT_BOX},
{"Border",	"BORDER_BOX",	(void *)fltk::BORDER_BOX},
{"Frame",	"BORDER_FRAME",	(void *)fltk::BORDER_FRAME},
{"Engraved",	"ENGRAVED_BOX",	(void *)fltk::ENGRAVED_BOX},
{"Embossed",	"EMBOSSED_BOX",	(void *)fltk::EMBOSSED_BOX},
{"Highlight",	"HIGHLIGHT_UP_BOX",(void *)fltk::HIGHLIGHT_UP_BOX},
{"Highlight Down","HIGHLIGHT_DOWN_BOX",(void *)fltk::HIGHLIGHT_DOWN_BOX},
{"Round",	"ROUND_UP_BOX",	(void *)fltk::ROUND_UP_BOX},
{"Round Down",	"ROUND_DOWN_BOX",(void *)fltk::ROUND_DOWN_BOX},
{"Diamond",	"DIAMOND_UP_BOX",(void *)fltk::DIAMOND_UP_BOX},
{"Diamond Down","DIAMOND_DOWN_BOX",(void *)fltk::DIAMOND_DOWN_BOX},
{"Shadow",	"SHADOW_BOX",	(void *)fltk::SHADOW_BOX},
{"Rounded",	"ROUNDED_BOX",	(void *)fltk::ROUNDED_BOX},
{"Rounded Shadow","RSHADOW_BOX",(void *)fltk::RSHADOW_BOX},
{"Rounded Flat","RFLAT_BOX",	(void *)fltk::RFLAT_BOX},
{"Oval",	"OVAL_BOX",	(void *)fltk::OVAL_BOX},
{"Oval Shadow",	"OSHADOW_BOX",	(void *)fltk::OSHADOW_BOX},
{"Oval Flat",	"OFLAT_BOX",	(void *)fltk::OFLAT_BOX},
{"Plastic Up",	"PLASTIC_UP_BOX",	(void *)fltk::PLASTIC_UP_BOX},
{"Plastic Down",	"PLASTIC_DOWN_BOX",	(void *)fltk::PLASTIC_DOWN_BOX},
{0}};

#define NOT_DEFAULT(W, what) (W->o->what() != ((WidgetType*)(W->factory))->o->what())

void box_cb(fltk::Choice* i, void *v) {
  if (v == LOAD) {
    set_menu(i, boxmenu);
    const Enumeration* e = from_value((void*)(current_widget->o->box()), boxmenu);
    i->value(e ? e-boxmenu : 0);
  } else {
    int m = i->value();
    fltk::Box* n = (fltk::Box*)(boxmenu[m].compiled);
    if (!n) return; // should not happen
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->box(n);
      q->redraw();
    }
  }
  fltk::Color c = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, box)) c = fltk::RED;
  if (i->labelcolor() != c) {
    i->labelcolor(c);
    i->redraw_label();
  }
}

void button_box_cb(fltk::Choice* i, void *v) {
  if (v == LOAD) {
    set_menu(i, boxmenu);
    const Enumeration* e = from_value((void*)(current_widget->o->buttonbox()), boxmenu);
    i->value(e ? e-boxmenu : 0);
  } else {
    int m = i->value();
    fltk::Box* n = (fltk::Box*)(boxmenu[m].compiled);
    if (!n) return; // should not happen
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->buttonbox(n);
      q->redraw();
    }
  }
  fltk::Color c = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, buttonbox)) c = fltk::RED;
  if (i->labelcolor() != c) {
    i->labelcolor(c);
    i->redraw_label();
  }
}

////////////////////////////////////////////////////////////////

static const Enumeration whenmenu[] = {
  {"never",	"NEVER",	(void*)fltk::WHEN_NEVER},
  {"Changed",	"CHANGED",	(void*)fltk::WHEN_CHANGED},
  {"Release",	"RELEASE",	(void*)fltk::WHEN_RELEASE},
  {"Release always","RELEASE_ALWAYS",(void*)fltk::WHEN_RELEASE_ALWAYS},
  {"Enter key",	"ENTER_KEY",	(void*)fltk::WHEN_ENTER_KEY},
  {"Enter key always","ENTER_KEY_ALWAYS",(void*)fltk::WHEN_ENTER_KEY_ALWAYS},
  {0}
};

void when_cb(fltk::Choice* i, void *v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->hide(); return;}
    set_menu(i, whenmenu);
    i->show();
    const Enumeration* e =
      from_value(current_widget->o->when(), whenmenu);
    i->value(e ? e-whenmenu : 0);
  } else {
    int m = i->value();
    int n = int((long)(whenmenu[m].compiled));
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->when(n);
    }
  }
  i->redraw();
}

void when_button_cb(fltk::CheckButton*, void *) {} // delete this!

bool WidgetType::resizable() const {
  if (is_window()) return ((Window*)o)->resizable() != 0;
  Group* p = (Group*)o->parent();
  return p ? p->resizable() == o : false;
}

void WidgetType::resizable(bool v) {
  if (v) {
    if (resizable()) return;
    if (is_window()) ((Window*)o)->resizable(o);
    else {
      Group* p = (Group*)o->parent();
      if (p) p->resizable(o);
    }
  } else {
    if (!resizable()) return;
    if (is_window()) {
      ((Window*)o)->resizable(0);
    } else {
      Group* p = (Group*)o->parent();
      if (p) p->resizable(0);
    }
  }
}

void resizable_cb(fltk::CheckButton* i,void* v) {
  if (v == LOAD) {
    if (current_widget->is_menu_item()) {i->deactivate(); return;}
    if (numselected > 1) {i->deactivate(); return;}
    i->activate();
    i->value(current_widget->resizable());
  } else {
    current_widget->resizable(i->value());
    modflag=1;
  }
  if (current_widget->resizable()) i->labelcolor(fltk::RED);
  else i->labelcolor(fltk::BLACK);
  i->redraw();
}

void hotspot_cb(fltk::CheckButton* i,void* v) {
  if (v == LOAD) {
    if (numselected > 1 || current_widget->is_menu_item()) {i->hide(); return;}
    i->show();
    i->value(current_widget->hotspot());
  } else {
    modflag = 1;
    current_widget->hotspot(i->value());
    if (i->value()) {
      // turn off hotspot in all siblings
      for (FluidType *p = current_widget->parent; p; p = p->parent) {
	if (!p->is_widget()) continue;
	for (FluidType* o = p->first_child; o; o = o->walk())
	  if (o != current_widget && o->is_widget())
	    ((WidgetType*)o)->hotspot(0);
      }
    }
  }
  if (current_widget->hotspot()) i->labelcolor(fltk::RED);
  else i->labelcolor(fltk::BLACK);  
  i->redraw();
}

void visible_cb(fltk::CheckButton* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->o->visible());
  } else {
    int n = i->value();
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      n ? q->o->show() : q->o->hide();
      q->redraw();
    }
  }
  if (!i->value()) i->labelcolor(fltk::RED);
  else i->labelcolor(fltk::BLACK);
  i->redraw();
}

void active_cb(fltk::CheckButton* i, void* v) {
  if (v == LOAD) {
    i->value(current_widget->o->active());
  } else {
    int n = i->value();
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      n ? q->o->activate() : q->o->deactivate();
      q->redraw();
    }
  }
  if (!i->value()) i->labelcolor(fltk::RED);
  else i->labelcolor(fltk::BLACK);  
  i->redraw();
}

////////////////////////////////////////////////////////////////

static const Enumeration fontmenu[] = {
  {"Helvetica",		"HELVETICA",		(void*)fltk::HELVETICA},
  {"Helvetica bold",	"HELVETICA_BOLD",	(void*)fltk::HELVETICA_BOLD},
  {"Helvetica italic",	"HELVETICA_ITALIC",	(void*)fltk::HELVETICA_ITALIC},
  {"Helvetica bold italic","HELVETICA_BOLD_ITALIC",(void*)fltk::HELVETICA_BOLD_ITALIC},
  {"Courier",		"COURIER",		(void*)fltk::COURIER},
  {"Courier bold",	"COURIER_BOLD",		(void*)fltk::COURIER_BOLD},
  {"Courier italic",	"COURIER_ITALIC",	(void*)fltk::COURIER_ITALIC},
  {"Courier bold italic","COURIER_BOLD_ITALIC",	(void*)fltk::COURIER_BOLD_ITALIC},
  {"Times",		"TIMES",		(void*)fltk::TIMES},
  {"Times bold",	"TIMES_BOLD",		(void*)fltk::TIMES_BOLD},
  {"Times italic",	"TIMES_ITALIC",		(void*)fltk::TIMES_ITALIC},
  {"Times bold italic",	"TIMES_BOLD_ITALIC",	(void*)fltk::TIMES_BOLD_ITALIC},
  {"Symbol",		"SYMBOL_FONT",		(void*)fltk::SYMBOL_FONT},
  {"Terminal",		"SCREEN_FONT",		(void*)fltk::SCREEN_FONT},
  {"Terminal Bold",	"SCREEN_BOLD_FONT",	(void*)fltk::SCREEN_BOLD_FONT},
  {"Zapf Dingbats",	"ZAPF_DINGBATS",	(void*)fltk::ZAPF_DINGBATS},
  {0}
};

int fontnumber(fltk::Font* f) {
  for (int n = 0; ; n++) {
    if (n > 15) return 0;
    if (f == fontmenu[n].compiled) return n;
  }
}

void label_font_cb(fltk::Choice* i, void *v) {
  if (v == LOAD) {
    set_menu(i, fontmenu);
    i->value(fontnumber(current_widget->o->labelfont()));
  } else {
    int n = i->value();
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->labelfont(fltk::font(n));
      q->redraw();
    }
  }
  fltk::Color c = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, labelfont)) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void label_size_cb(fltk::ValueInput* i, void *v) {
  float n;
  if (v == LOAD) {
    n = current_widget->o->labelsize();
  } else {
    n = float(i->value());
    if (n <= 0) n = WidgetType::default_size;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->labelsize(n);
      q->redraw();
    }
  }
  i->value(n);
  fltk::Color c = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, labelsize)) c = fltk::RED;
  if (i->textcolor() != c)
    { i->textcolor(c); i->redraw();}
}

void image_cb(fltk::Button *a, void * v) {
  if (v != LOAD) {
    Fluid_Image *i = ui_find_image(current_widget->image);
    if (i == current_widget->image) return; // user hit "Cancel"
    for_all_selected_widgets() {
      WidgetType* p = (WidgetType*)o;
      p->set_image(i);
    }
    image_inlined_cb(include_image_button, LOAD); // update the button state
  }
  const char* s = current_widget->image ? 
    current_widget->image->name() : 0;
  if (s != image_button->label()) 
    {image_button->label(s); image_button->redraw();}
  fltk::Color c = fltk::BLACK;
  if (current_widget->image) c = fltk::RED;
  if (image_button->labelcolor() != c)
    { image_button->labelcolor(c); image_button->redraw();}
}

static const Enumeration labelstylemenu[] = {
  {"Normal",	"NORMAL_LABEL",	(void*)fltk::NORMAL_LABEL},
  {"Symbol",	"SYMBOL_LABEL",	(void*)fltk::SYMBOL_LABEL},
  {"Shadow",	"SHADOW_LABEL",	(void*)fltk::SHADOW_LABEL},
  {"Engraved",	"ENGRAVED_LABEL",(void*)fltk::ENGRAVED_LABEL},
  {"Embossed",	"EMBOSSED_LABEL",(void*)fltk::EMBOSSED_LABEL},
  {"None",	"NO_LABEL",	(void*)(fltk::NO_LABEL)},
{0}};

void label_style_cb(fltk::Choice* i, void *v) {
  if (v == LOAD) {
    set_menu(i, labelstylemenu);
    const Enumeration* e = from_value((void*)(current_widget->o->labeltype()), labelstylemenu);
    i->value(e ? e-labelstylemenu : 0);
  } else {
    int m = i->value();
    fltk::LabelType* n = (fltk::LabelType*)(labelstylemenu[m].compiled);
    if (!n) return; // should not happen
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* p = (WidgetType*)o;
      p->o->labeltype(n);
      p->redraw();
    }
  }
  fltk::Color c = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, labeltype)) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

////////////////////////////////////////////////////////////////

void color_cb(fltk::LightButton* i, void *v) {
  fltk::Color c = current_widget->o->color();
  if (v == LOAD) {
    i->show();
  } else {
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->color(c); q->redraw();
    }
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, color) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

void selection_color_cb(fltk::LightButton* i, void *v) {
  fltk::Color c = current_widget->o->selection_color();
  if (v == LOAD) {
    i->label("Selection Color");
    i->show();
  } else {
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->selection_color(c); q->redraw();
    }
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, selection_color) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

void button_color_cb(fltk::LightButton* i, void *v) {
  fltk::Color c = current_widget->o->buttoncolor();
  if (v == LOAD) {
    i->show();
  } else {
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->buttoncolor(c); q->redraw();
    }
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, buttoncolor) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

void label_color_cb(fltk::LightButton* i, void *v) {
  fltk::Color c = current_widget->o->labelcolor();
  if (v != LOAD) {
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->labelcolor(c); q->redraw();
    }
  } else {
    i->show();
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, labelcolor) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

void text_font_cb(fltk::Choice* i, void* v) {
  fltk::Font* n;
  if (v == LOAD) {
    set_menu(i, fontmenu);
    if (current_widget->is_menu_item()) i->label("Shortcut Font:");
    else i->label("Text Font:");
    i->show();
    i->value(fontnumber(current_widget->o->textfont()));
  } else {
    n = fltk::font(i->value());
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->textfont(n);
      q->redraw();
    }
  }
  fltk::Color c = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, textfont)) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void text_size_cb(fltk::ValueInput* i, void* v) {
  float n;
  if (v == LOAD) {
    n = current_widget->o->textsize();
    i->show();
  } else {
    n = float(i->value());
    if (n <= 0) n = WidgetType::default_size;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->textsize(n);
      q->redraw();
    }
  }
  i->value(n);
  fltk::Color c = fltk::BLACK;
  if (NOT_DEFAULT(current_widget, textsize)) c = fltk::RED;
  if (i->textcolor() != c)
    { i->textcolor(c); i->redraw(); }
}

void text_color_cb(fltk::LightButton* i, void* v) {
  fltk::Color c;
  if (v == LOAD) {
    c = current_widget->o->textcolor();
    i->show();
  } else {
    c = i->selection_color();
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->textcolor(c);
      q->redraw();
    }
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, textcolor) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

void selected_text_color_cb(fltk::LightButton* i, void* v) {
  fltk::Color c;
  if (v == LOAD) {
    i->label("Selected Text Color");
    c = current_widget->o->selection_textcolor();
    i->show();
  } else {
    c = i->selection_color();
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->selection_textcolor(c);
      q->redraw();
    }
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, selection_textcolor) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

void highlight_color_cb(fltk::LightButton* i, void *v) {
  fltk::Color c = fltk::BLACK;
  if (v == LOAD) {
    c = current_widget->o->highlight_color();
    if (!c) c = current_widget->o->buttoncolor();
    i->show();
  } else {
    c = i->selection_color();
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->highlight_color(c); q->redraw();
    }
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, highlight_color) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

void highlight_label_color_cb(fltk::LightButton* i, void *v) {
  fltk::Color c = fltk::BLACK;
  if (v == LOAD) {
    i->label("Highlight Label Color");
    c = current_widget->o->highlight_textcolor();
    if (!c) c = current_widget->o->labelcolor();
    i->show();
  } else {
    c = i->selection_color();
    if (!fltk::color_chooser(i->label(), c)) return;
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      q->o->highlight_textcolor(c); q->redraw();
    }
  }
  i->color(c);
  i->selection_color(c);
  i->labelcolor(NOT_DEFAULT(current_widget, highlight_textcolor) ? fltk::RED : fltk::BLACK);
  i->redraw();
}

static const Enumeration alignmenu[] = {
  {"x", "fltk::ALIGN_CENTER",	(void*)(fltk::ALIGN_CENTER)},
  {"x", "fltk::ALIGN_TOP",	(void*)(fltk::ALIGN_TOP)},
  {"x", "fltk::ALIGN_BOTTOM",	(void*)(fltk::ALIGN_BOTTOM)},
  {"x", "fltk::ALIGN_LEFT",	(void*)(fltk::ALIGN_LEFT)},
  {"x", "fltk::ALIGN_RIGHT",	(void*)(fltk::ALIGN_RIGHT)},
  {"x", "fltk::ALIGN_INSIDE",	(void*)(fltk::ALIGN_INSIDE)},
  {"x", "fltk::ALIGN_CLIP",	(void*)(fltk::ALIGN_CLIP)},
  {"x", "fltk::ALIGN_WRAP",	(void*)(fltk::ALIGN_WRAP)},
{0}};

void align_cb(fltk::Button* i, void *v) {
  fltk::Flags b = fltk::Flags(i->argument());
  // fabien: I don't understand how colors codification
  // should work here i get strange results once i propagate the load events
  // bill if you read these lines please specify how you intented it to work.

  if (v == LOAD) {
    i->value((current_widget->o->flags() & b) != 0);
    fltk::Flags tplate = ((WidgetType*)(current_widget->factory))->o->flags();
    fltk::Color c = fltk::BLACK;
    fltk::Color d = fltk::RED;
    if (tplate & b) {c = fltk::RED; d = fltk::BLACK;}
    if (i->labelcolor() != c) {i->labelcolor(c); i->redraw();}
    i->selection_textcolor(d);
    i->selection_color(i->color());
  } else {
      Undo::checkpoint();

    for_all_selected_widgets() {
      WidgetType* q = (WidgetType*)o;
      int x = q->o->flags() & fltk::ALIGN_MASK;
      int y;
      if (i->value()) {
	y = x | b;
      } else {
	y = x & ~b;
      }
      if (x != y) {q->o->align(y); q->redraw(); modflag = 1;}
    }
  }
}

void image_inlined_cb(fltk::CheckButton* i, void *v) {
  if (v==LOAD) { // for now all images are inlined or none is.
    if (current_widget->image) {
      i->activate();
      i->value(current_widget->image->inlined);
    } else {
      i->deactivate();
      i->value(false);
    }
  } else {
    for_all_selected_widgets() {
      WidgetType* q = (WidgetType*)o;
      if (q->image) q->image->inlined = i->value();
    }
  }
}

////////////////////////////////////////////////////////////////

void callback_cb(fltk::MultiLineInput* i, void *v) {
  if (v == LOAD) {
    i->static_value(current_widget->callback());
  } else {
    const char *c = i->value();
    const char *d = c_check(c);
    if (d) {fltk::message("Error in callback: %s",d); haderror = 1; return;}
    for_all_selected_widgets() o->callback(c);
  }
  fltk::Color c = fltk::BLACK;
  if (current_widget->callback()) c = fltk::RED;
  if (i->labelcolor() != c) {
    i->labelcolor(c);
    i->redraw_label();
  }
}

void user_data_cb(fltk::Input *i, void *v) {
  if (v == LOAD) {
    i->static_value(current_widget->user_data());
  } else {
    const char *c = i->value();
    const char *d = c_check(c);
    if (d) {fltk::message("Error in user_data: %s",d); haderror = 1; return;}
    for_all_selected_widgets() o->user_data(c);
  }
  fltk::Color c = fltk::BLACK;
  if (current_widget->user_data()) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void user_data_type_cb(fltk::Input *i, void *v) {
  static const char *dflt = "void*";
  if (v == LOAD) {
    const char *c = current_widget->user_data_type();
    if (!c) c = dflt;
    i->static_value(c);
  } else {
    const char *c = i->value();
    const char *d = c_check(c);
    if (!*c) i->value(dflt);
    else if (!strcmp(c,dflt)) c = 0;
    if (!d) {
      if (c && *c && c[strlen(c)-1] != '*' && strcmp(c,"long"))
	d = "must be pointer or long";
    }
    if (d) {fltk::message("Error in type: %s",d); haderror = 1; return;}
    for_all_selected_widgets() o->user_data_type(c);
  }
  fltk::Color c = fltk::BLACK;
  if (strcmp(i->value(), "void*")) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void extra_code_input_cb(fltk::MultiLineInput* i, void* v) {
  if (v == LOAD) {
    i->static_value(current_widget->extra_code());
  } else {
    const char *c = i->value();
    const char *d = c_check(c&&c[0]=='#' ? c+1 : c);
    if (d) {fltk::message("Error in %s: %s",i->label(),d); haderror = 1; return;}
    for_all_selected_widgets() {
      WidgetType *t = (WidgetType*)o;
      t->extra_code(c);
    }
  }
  fltk::Color c = fltk::BLACK;
  if (i->value() && *i->value()) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void user_class_cb(fltk::Input* i, void* v) {
  if (v == LOAD) {
    const char* s = 0;
    for_all_selected_widgets() {
      WidgetType *t = (WidgetType*)o;
      const char* c = t->subclass();
      if (!s || !*s) s = c;
      else if (strcmp(s,c)) {s = 0; break;}
    }
    i->static_value(s);
  } else {
    const char *c = i->value();
    while (isspace(*c)) c++;
    for_all_selected_widgets() {
      WidgetType *t = (WidgetType*)o;
      t->user_class(0);
      if (*c && strcmp(c, t->subclass())) t->user_class(c);
    }
  }
  fltk::Color c = fltk::BLACK;
  if (current_widget->user_class()) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

////////////////////////////////////////////////////////////////
// Kludges to the panel for subclasses:

void slider_size_cb(fltk::ValueInput* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_valuator()!=2) {i->hide(); return;}
    i->show();
    i->value(((fltk::Slider*)(current_widget->o))->slider_size());
  } else {
    int n = int(i->value());
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      if (q->is_valuator()==2) {
	((fltk::Slider*)(q->o))->slider_size(n);
	q->redraw();
      }
    }
  }
}

void min_cb(fltk::ValueInput* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_valuator()) {i->hide(); return;}
    i->show();
    i->value(((fltk::Valuator*)(current_widget->o))->minimum());
  } else {
    double n = i->value();
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      if (q->is_valuator()) {
	((fltk::Valuator*)(q->o))->minimum(n);
	q->redraw();
      }
    }
  }
  fltk::Color c = fltk::BLACK;
  if (i->value() != 0.0) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void max_cb(fltk::ValueInput* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_valuator()) {i->hide(); return;}
    i->show();
    i->value(((fltk::Valuator*)(current_widget->o))->maximum());
  } else {
    double n = i->value();
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      if (q->is_valuator()) {
	((fltk::Valuator*)(q->o))->maximum(n);
	q->redraw();
      }
    }
  }
  fltk::Color c = fltk::BLACK;
  if (i->value() != 1.0) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void step_cb(fltk::ValueInput* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_valuator()) {i->hide(); return;}
    i->show();
    i->value(((fltk::Valuator*)(current_widget->o))->step());
  } else {
    double n = i->value();
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      if (q->is_valuator()) {
	((fltk::Valuator*)(q->o))->step(n);
	q->redraw();
      }
    }
  }
  fltk::Color c = fltk::BLACK;
  if (i->value() != 0.0) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void line_cb(fltk::ValueInput* i, void* v) {
  if (v == LOAD) {
    if (!current_widget->is_valuator()) {i->hide(); return;}
    i->show();
    i->value(((fltk::Valuator*)(current_widget->o))->linesize());
  } else {
    int n = int(i->value());
    if (n >= 0) for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      if (q->is_valuator()) {
	((fltk::Valuator*)(q->o))->linesize(n);
	q->redraw();
      }
    }
  }
  fltk::Color c = fltk::BLACK;
  if (i->value() != 1.0) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

void value_cb(fltk::ValueInput* i, void* v) {
  if (v == LOAD) {
    if (current_widget->is_valuator()) {
      i->show();
      i->value(((fltk::Valuator*)(current_widget->o))->value());
    } else if (current_widget->is_button()) {
      i->show();
      i->value(current_widget->o->state());
    } else 
      i->hide();
  } else {
    double n = i->value();
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      if (q->is_valuator()) {
	((fltk::Valuator*)(q->o))->value(n);
      } else {
	q->o->state(n != 0);
	if (q->is_menu_item()) q->redraw();
      }
    }
  }
  fltk::Color c = fltk::BLACK;
  if (i->value() != 0.0) c = fltk::RED;
  if (i->labelcolor() != c)
    { i->labelcolor(c); i->redraw_label(); }
}

////////////////////////////////////////////////////////////////

// subtypes:

const Enumeration *WidgetType::subtypes() const {return 0;}

void subtype_cb(fltk::Choice* i, void* v) {
  if (v == LOAD) {
    const Enumeration* table = current_widget->subtypes();
    if (!table) {i->hide(); return;}
    i->show();
    set_menu(i, table);
    const Enumeration* e = from_value(current_widget->o->type(), table);
    i->value(e ? e-table : 0);
    i->redraw();
  } else {
    const Enumeration* table = current_widget->subtypes();
    int n = int((long)(table[i->value()].compiled));
    for_all_selected_widgets() {
      modflag = 1;
      WidgetType* q = (WidgetType*)o;
      if (q->subtypes()==table) {
        q->o->type(n);
        q->redraw();
      }
    }
  }
}

/**
 * Groups in the panel send LOAD to all widgets, send any other
 * callback to only the changed widgets, and recursively call
 * themselves:
 */
void propagate_group(fltk::Group* g, void* v) {
  if (v == LOAD) {
    for (int i=g->children(); i--;) {
      fltk::Widget* o = g->child(i);
      o->do_callback(o,LOAD);
    }
  } else {
    for (int i=g->children(); i--;) {
      fltk::Widget* o = g->child(i);
      if (o->changed() || o->callback()==(fltk::Callback*)propagate_group ||
          o->callback()==(fltk::Callback*)propagate_tabs)
      {
	o->do_callback();
	if (haderror) return;
	o->clear_changed();
      }
    }
  }    
}

/**
 * Fluid insists on giving the correct type to arguments so we need
 * a different call for the tabs:
 */
void propagate_tabs(fltk::TabGroup* g, void* v) {
  propagate_group(g, v);
}

inline Fluid_Plugin** next_panel(Fluid_Plugin** pp, Fluid_Plugin* &p)
{
  while(pp < plugins+nbplugins && !(*pp)->make_panel) pp++;
  if(pp < plugins+nbplugins) p = *pp;
  return pp;
}

void set_cb(fltk::Button*, void*) {
  haderror = 0;
  propagate_group(the_panel, 0);
  Fluid_Plugin *p = 0, **pp;
  for(pp = next_panel(plugins, p); pp-plugins<nbplugins; pp = next_panel(pp+1, p))
    if(p->panel_is_orphan) propagate_group(p->panel, 0);
}

void ok_cb(fltk::ReturnButton* o, void* v) {
  set_cb(o,v);
  if (!haderror)
    the_panel->hide();
}

static void load_panel();

void default_cb(fltk::Button*, void* v) {
  if (v == LOAD) return;
  const fltk::Style* default_style =
    ((WidgetType*)(current_widget->factory))->o->style();
  fltk::Style* s1 = (fltk::Style*)(current_widget->o->style());
  if (s1 == default_style) return;
  // Delete the style:
  delete s1;
  // this just assigns the pointer:
  current_widget->o->style((fltk::Style*)default_style);
  load_panel();
  current_widget->redraw();
}

void revert_cb(fltk::Button*, void*) {
  if (!current_widget) return;
  // We have to revert all dynamically changing fields:
  // but for now only the first label works...
  if (numselected == 1) current_widget->label(oldlabel);
  propagate_group(the_panel, LOAD);
  Fluid_Plugin *p = 0, **pp;
  for(pp = next_panel(plugins, p); pp-plugins<nbplugins; pp = next_panel(pp+1, p))
    if(p->panel_is_orphan) propagate_group(p->panel, LOAD);
}

void cancel_cb(fltk::Button* o, void* v) {
  revert_cb(o, v);
  the_panel->hide();
}

void toggle_overlays(fltk::Widget *,void *); // in WindowType.C
void overlay_cb(fltk::CheckButton*o,void *v) {
  toggle_overlays(o,v);
}

/**
 * Update the panel according to current widget set:
 */
static void load_panel() {
  if (!the_panel) return;

  // find all the fltk::Widget subclasses currently selected:
  numselected = 0;
  current_widget = 0;
  if (FluidType::current) {
    if (FluidType::current->is_widget())
      current_widget=(WidgetType*)FluidType::current;
    for_all_selected_widgets() {
      numselected++;
      if (!current_widget) current_widget = (WidgetType*)o;
    }
  }
  if (numselected) {
    Fluid_Plugin *p = 0, **pp;
    for(pp = next_panel(plugins, p); pp-plugins<nbplugins; pp = next_panel(pp+1, p))
      p->please_show_panel = 0;
    propagate_group(the_panel, LOAD);
    for(pp = next_panel(plugins, p); pp-plugins<nbplugins; pp = next_panel(pp+1, p))
    {
      if (p->panel_is_orphan) {
	propagate_group(p->panel, LOAD);
	if (p->please_show_panel) {
	  p->panel_is_orphan = 0;
	  panel_tabs->add(*p->panel);
	  if (p->was_visible) panel_tabs->selected_child(p->panel);
	  the_panel->redraw();
	  p->panel->redraw();
	}
      } else {
	if (!p->please_show_panel) {
	  if (panel_tabs->selected_child() == p->panel) {
	    panel_tabs->value(0);
	    p->was_visible = 1;
	  } else
	    p->was_visible = 0;
	  p->panel_is_orphan = 1;
	  panel_tabs->remove(*p->panel);
	  the_panel->redraw();
	}
      }
    }	
  }
}

/**
 * This is called when user double-clicks an item, open or update the panel:
 */
void WidgetType::open() {
  if (!the_panel) {
    the_panel = make_widget_panel();
    initialize_tab_colors();
    Fluid_Plugin *p = 0, **pp;
    for(pp = next_panel(plugins, p); pp-plugins<nbplugins; pp = next_panel(pp+1, p))
    {
      p->make_panel();
      // All plugin panels are initially not mapped in the main pannel
      p->panel_is_orphan = 1; 
      p->panel->position(panel_tabs->child(0)->x(), panel_tabs->child(0)->y());
      p->panel->layout();
      p->panel->resizable(0);
      p->panel->resize(panel_tabs->child(0)->w(), panel_tabs->child(0)->h());
      p->panel->label(p->name);
    }
  }

  load_panel();
  if (numselected) {
    the_panel->show();
  }
}

extern void redraw_overlays();

/**
 * Called when the select value on the passed object changes, or when
 * objects are deleted (in which case the passed object is null).
 */
void selection_changed(FluidType *p) {
  if (p && the_panel && the_panel->visible()) {
    // store all changes to the current selected objects:
    set_cb(0,0);
#if 0
    if (haderror) {
      // If there is an error, try to put selected flags and FluidType::current
      // back to the previous values:
      FluidType *q = 0;
      for (FluidType *o = FluidType::first; o; o = o->walk()) {
        o->new_selected = o->selected();
        if (!q && o->selected()) q = o;
      }
      if (!p || !p->selected()) p = q;
      FluidType::current = p;
      redraw_browser();
      return;
    }
#endif
  }
  // update the selected flags to new set:
  for (FluidType* o = FluidType::first; o; o = o->walk()) {
    o->selected(o->new_selected);
    //if (o->selected()) FluidType::current = o;
  }
  if (p && p->new_selected) FluidType::current = p;
  redraw_overlays();
  // load the panel with the new settings:
  load_panel();
}

/**
 *
 * Writing the C code:
 *
 * test to see if user named a function, or typed in code:
 */
int is_name(const char *c) {
  for (; *c; c++) if (ispunct(*c) && *c!='_' && *c!=':') return 0;
  return 1;
}

// Test to see if name() is an array entry.  If so, and this is the
// highest number, return name[num+1].  Return null if not the highest
// number or a field or function.  Return name() if not an array entry.
const char* WidgetType::array_name() const {
  const char *c = name();
  if (!c) return 0;
  const char *d;
  for (d = c; *d != '['; d++) {
    if (!*d) return c;
    if (ispunct(*d) && *d!='_') return 0;
  }
  char* p;
  long num = strtol(d+1, &p, 0); if (*p != ']') return 0;
  int sawthis = 0;
  FluidType* parent = this->parent;
  while (parent && !parent->is_class()) parent = parent->parent;
  for (FluidType* t = parent?parent->first_child : FluidType::first;
       t; t = t->walk(parent)) {
    if (t == (FluidType*)this) {sawthis=1; continue;}
    const char *e = t->name();
    if (!e) continue;
    if (strncmp(c,e,d-c)) continue;
    int n1 = strtol(e+(d-c)+1,0,0);
    if (n1 > num || (n1 == num && sawthis)) return 0;
  }
  static char buffer[128];
  strncpy(buffer,c,d-c+1);
  sprintf(buffer+(d-c+1), "%ld]", num+1);
  return buffer;
}

void WidgetType::write_static() {
  const char* subclass = this->subclass();
  if (!user_class() || !strncmp(subclass, "fltk::", 6)) 
    ::write_declare("#include <fltk/%s.h>", subclass+6);
  if (extra_code()) write_includes_from_code((char*)extra_code());
  if (callback()) {
    if (is_name(callback()))
      ::write_declare("extern void %s(%s*, %s);", callback(), subclass,
		      user_data_type() ? user_data_type() : "void*");
    else
      write_includes_from_code((char*)callback());
  }
  const char* array_name = this->array_name();
  const char* member_of = this->member_of(true);
  if (array_name && !member_of) {
    write_c("\n");
    if (!public_) write_c("static ");
    else write_h("extern %s* %s;\n", subclass, array_name);
    //write_c("%s* %s;\n", subclass, array_name);
    if (strchr(array_name, '[') == NULL) write_c("%s *%s=(%s *)0;\n", subclass, array_name, subclass);
    else write_c("%s *%s={(%s *)0};\n", subclass, array_name, subclass);
  }
  if (callback() && !is_name(callback())) {
    // see if 'o' or 'v' used, to prevent unused argument warnings:
    int use_o = 0;
    int use_v = 0;
    const char *d;
    for (d = callback(); *d;) {
      if (*d == 'o' && !is_id(d[1])) use_o = 1;
      if (*d == 'v' && !is_id(d[1])) use_v = 1;
      do d++; while (is_id(*d));
      while (*d && !is_id(*d)) d++;
    }
    const char* cn = callback_name();
    if (member_of) {
      write_c("\ninline void %s::%s_i(%s*", member_of, cn, subclass);
    } else {
      write_c("\nstatic void %s(%s*", cn, subclass);
    }
    if (use_o) write_c(" o");
    const char* ut = user_data_type() ? user_data_type() : "void*";
    write_c(", %s", ut);
    if (use_v) write_c(" v");
    write_c(")%s", get_opening_brace(1));
    indentation += 2;
    write_code_block((char *)callback());
    indentation -= 2;
    if (*(d-1) != ';') {
      const char *p = strrchr(callback(), '\n');
      if (p) p ++;
      else p = callback();
      // Only add trailing semicolon if the last line is not a preprocessor
      // statement...
      if (*p != '#' && *p) write_c(";");
    }
    write_c("}\n");
    if (member_of) {
      write_c("void %s::%s(%s* o, %s v)%s", member_of, cn, subclass,
	      ut, get_opening_brace(1));
      write_c("%s((%s*)(o->", get_indent_string(1), member_of);
      for (FluidType* p = parent; p->is_widget(); p = p->parent)
	     write_c("parent()->");
      write_c("user_data()))->%s_i(o,v);\n}\n", cn);
    }
  }
  if (image && image->written != write_number) {
    image->written = write_number; // do this first so they can recurse
    image->write_static();
  }
}

const char *FluidType::callback_name() {
  if (is_name(callback())) return callback();
  return unique_id(this, "cb", name(), label());
}

extern int varused_test, varused;
extern WidgetType* last_group;
  
void WidgetType::write_code1() {
  const char* subclass = this->subclass();
  const char* array_name = this->array_name();
  bool member_of = this->member_of() != 0;
  if (array_name) {
    if (member_of) {
      write_public(public_);
      write_h("%s%s *%s;\n", indent(), subclass, array_name);
    }
  }
  if (member_of && callback() && !is_name(callback())) {
    const char* cn = callback_name();
    const char* ut = user_data_type() ? user_data_type() : "void*";
    write_public(0);
    write_h("%sinline void %s_i(%s*, %s);\n", indent(), cn, subclass, ut);
    write_h("%sstatic void %s(%s*, %s);\n", indent(), cn, subclass, ut);
  }
  // figure out if local variable will be used (prevent compiler warnings):
  if (is_parent() || extra_code())
    varused = 1;
  else {
    varused_test = 1; varused = 0;
    write_widget_code();
    varused_test = 0;
  }
  write_c(indent());
  if (varused) write_c("%s%s* o = ", get_opening_brace(0), subclass);
  if (name()) write_c("%s = ", name());
  if (is_window()) {
    // Handle special case where user is faking a fltk::Group type as a window,
    // there is no 2-argument constructor in that case:
    if (set_xy)
      write_c("new %s(%d, %d, %d, %d", subclass, o->x(),o->y(),o->w(),o->h());
    else if (strstr(subclass, "Window"))
      write_c("new %s(%d, %d", subclass, o->w(), o->h());
    else
      write_c("new %s(0, 0, %d, %d", subclass, o->w(), o->h());
  } else if (is_menu_item()) {
    write_c("new %s(", subclass);
  } else {
    write_c("new %s(%d, %d, %d, %d", subclass, o->x(), o->y(), o->w(), o->h());
  }
  if (label() && *label()) {
    if (!is_menu_item()) write_c(", ");
    write_cstring(label());
  }
  write_c(");\n");
  indentation += 2;
  if (this == last_group) write_c("%sw = o;\n",indent());
  if (varused) write_widget_code();
}

static void write_color(const char* function, fltk::Color c) {
  if (c > 255) write_c("%so->%s((fltk::Color)0x%x);\n", indent(), function, c);
  else write_c("%so->%s((fltk::Color)%i);\n", indent(), function, c);
}

// this is split from write_code1() for WindowType:
void WidgetType::write_widget_code() {
  fltk::Widget* tplate = ((WidgetType*)factory)->o;

  if (o->type() != tplate->type()) {
    const Enumeration* e = subtypes();
    if (e) e = from_value(o->type(), e);
    if (e && e->symbol)
      write_c("%so->type(%s::%s);\n", indent(), subclass(), e->symbol);
    else
      write_c("%so->type(%d);\n", indent(), o->type());
  }

  if (o->vertical()) write_c("%so->set_vertical();\n", indent());

  if (image) image->write_code();

  if (o->box() != tplate->box())
    write_c("%so->box(fltk::%s);\n",indent(),to_text((void*)(o->box()),boxmenu));
  if (o->buttonbox() != tplate->buttonbox())
    write_c("%so->buttonbox(fltk::%s);\n", indent(), to_text((void*)(o->buttonbox()),boxmenu));
  if (o->labelfont() != tplate->labelfont())
    write_c("%so->labelfont(fltk::%s);\n", indent(), fontmenu[fontnumber(o->labelfont())].symbol);
  if (o->textfont() != tplate->textfont())
    write_c("%so->textfont(fltk::%s);\n", indent(), fontmenu[fontnumber(o->textfont())].symbol);
  if (o->labeltype() != tplate->labeltype())
    write_c("%so->labeltype(fltk::%s);\n", indent(),
	    to_text((void*)(o->labeltype()),labelstylemenu));
  if (o->color() != tplate->color())
    write_color("color", o->color());
  if (o->textcolor() != tplate->textcolor())
    write_color("textcolor", o->textcolor());
  if (o->selection_color() != tplate->selection_color())
    write_color("selection_color", o->selection_color());
  if (o->selection_textcolor() != tplate->selection_textcolor())
    write_color("selection_textcolor", o->selection_textcolor());
  if (o->buttoncolor() != tplate->buttoncolor())
    write_color("buttoncolor", o->buttoncolor());
  if (o->labelcolor() != tplate->labelcolor())
    write_color("labelcolor", o->labelcolor());
  if (o->highlight_color() != tplate->highlight_color())
    write_color("highlight_color", o->highlight_color());
  if (o->highlight_textcolor() != tplate->highlight_textcolor())
    write_color("highlight_textcolor", o->highlight_textcolor());
  if (o->labelsize() != tplate->labelsize())
    write_c("%so->labelsize(%g);\n", indent(), o->labelsize());
  if (o->textsize() != tplate->textsize())
    write_c("%so->textsize(%g);\n", indent(), o->textsize());
  if (o->state() && !tplate->state())
    write_c("%so->set_flag(fltk::STATE);\n", indent());
  if (o->shortcut())
    write_c("%so->shortcut(0x%x);\n", indent(), o->shortcut());

  if (is_valuator()) {
    fltk::Valuator* v = (fltk::Valuator*)o;
    fltk::Valuator* f = (fltk::Valuator*)(tplate);
    if (v->minimum()!=f->minimum())
      write_c("%so->minimum(%g);\n", indent(), v->minimum());
    if (v->maximum()!=f->maximum())
      write_c("%so->maximum(%g);\n", indent(), v->maximum());
    if (v->step()!=f->step())
      write_c("%so->step(%g);\n", indent(), v->step());
    if (v->linesize_setting()!=f->linesize_setting())
      write_c("%so->linesize(%g);\n", indent(), v->linesize_setting());
    if (v->value())
      write_c("%so->value(%g);\n", indent(), v->value());
    if (is_valuator()==2) {
      double x = ((fltk::Slider*)v)->slider_size();
      double y = ((fltk::Slider*)f)->slider_size();
      if (x != y) write_c("%so->slider_size(%g);\n", indent(), x);
    }
  }

  const char* ud = user_data();
  if (member_of() && !parent->is_widget()) ud = "this";
  if (callback()) {
    write_c("%so->callback((fltk::Callback*)%s", indent(), callback_name());
    if (ud)
      write_c(", (void*)(%s));\n", ud);
    else
      write_c(");\n");
  } else if (ud) {
    write_c("%so->user_data((void*)(%s));\n", indent(), ud);
  }
  if ((o->flags()&fltk::ALIGN_MASK) != (tplate->flags()&fltk::ALIGN_MASK)) {
    fltk::Flags i = o->flags() & fltk::ALIGN_MASK;
    write_c("%so->align(", indent());
    bool first = true;
    for (int n = 0; n < 8; n++) if (i & (1<<n)) {
      if (!first) write_c("|");
      first = false;
      write_c(number_to_text(1<<n, alignmenu));
    }
    if (first) write_c("fltk::ALIGN_CENTER");
    write_c(");\n");
  }
  if (o->when() != tplate->when())
    write_c("%so->when(fltk::WHEN_%s);\n", indent(), number_to_text(o->when(), whenmenu));
  if (!o->visible() && o->parent())
    write_c("%so->hide();\n", indent());
  if (!o->active())
    write_c("%so->deactivate();\n", indent());
  if (!is_group() && resizable())
    write_c("%sfltk::Group::current()->resizable(o);\n",indent());
  if (hotspot()) {
    write_c("%s((fltk::Window*)(o", indent());
    fltk::Widget* p = o->parent();
    while (p) { p = p->parent(); write_c("->parent()"); }
    write_c("))->hotspot(o);\n");
  }
  if (tooltip() && *tooltip()) {
    write_c("%so->tooltip(", indent());
    write_cstring(tooltip());
    write_c(");\n");
  }
}

void WidgetType::write_extra_code() {
  if (extra_code()) write_code_block((char*)extra_code());
}

void WidgetType::write_block_close() {
  indentation -= 2;
  if (is_parent() || varused) write_c("%s}\n", indent());
}

void WidgetType::write_code() {
  write_code1();
  for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
  write_extra_code();
  write_block_close();
}

////////////////////////////////////////////////////////////////

static void save_color(const char* name, fltk::Color color) {
  if (color > 255) write_string("%s 0x%x", name, color);
  else write_string("%s %u", name, color);
}

void WidgetType::write_properties() {
  FluidType::write_properties();
  int indent = 1;

  for (FluidType* p = parent; p; p = p->parent) indent++;
  write_indent(indent);
  if (!public_) write_string("private");
  if (!is_menu_item())
    write_string("xywh {%d %d %d %d}", o->x(), o->y(), o->w(), o->h());
  fltk::Widget* tplate = ((WidgetType*)factory)->o;
  if (o->type() != tplate->type()) {
    write_string("type");
    write_word(number_to_text(o->type(), subtypes()));
  }
  // write horizontal or vertical if it disagrees with dimensions:
  if (o->h()>o->w()) {
    if (!o->vertical()) write_string("horizontal");
  } else {
    if (o->vertical()) write_string("vertical");
  }
  if (set_xy && is_window())
    write_string("set_xy");

  if (o->align() != tplate->align())
    write_string("align %d", o->align());
  if (o->when() != tplate->when())
    write_string("when %s", number_to_text(o->when(), whenmenu));
  if (!o->visible()) write_string("hide");
  if (!o->active()) write_string("deactivate");
  if (resizable()) write_string("resizable");
  if (hotspot()) write_string("hotspot");

  if (o->box() != tplate->box()) {
    write_string("box"); write_word(to_text((void*)(o->box()),boxmenu));}
  if (o->buttonbox() != tplate->buttonbox()) {
    write_string("buttonbox"); write_word(to_text((void*)(o->buttonbox()),boxmenu));}
  if (o->labelfont() != tplate->labelfont())
    write_string("labelfont %d", fontnumber(o->labelfont()));
  if (o->textfont() != tplate->textfont())
    write_string("textfont %d", fontnumber(o->textfont()));
  if (o->labeltype() != tplate->labeltype()) {
    write_string("labeltype");
    write_word(to_text((void*)(o->labeltype()),labelstylemenu));
  }
  if (image) {
    write_indent(indent);
    write_string("image");  // when only one image is required
    if (!image->inlined) write_string("not_inlined");
    write_word(image->name());
  }
  if (o->color() != tplate->color())
    save_color("color", o->color());
  if (o->textcolor() != tplate->textcolor())
    save_color("textcolor", o->textcolor());
  if (o->selection_color() != tplate->selection_color())
    save_color("selection_color", o->selection_color());
  if (o->selection_textcolor() != tplate->selection_textcolor())
    save_color("selection_textcolor", o->selection_textcolor());
  if (o->buttoncolor() != tplate->buttoncolor())
    save_color("buttoncolor", o->buttoncolor());
  if (o->labelcolor() != tplate->labelcolor())
    save_color("labelcolor", o->labelcolor());
  if (o->highlight_color() != tplate->highlight_color())
    save_color("highlight_color", o->highlight_color());
  if (o->highlight_textcolor() != tplate->highlight_textcolor())
    save_color("highlight_textcolor", o->highlight_textcolor());
  if (o->labelsize() != tplate->labelsize())
    write_string("labelsize %g", o->labelsize());
  if (o->textsize() != tplate->textsize())
    write_string("textsize %g", o->textsize());
  unsigned s = o->shortcut(); if (s && s != tplate->shortcut())
    write_string("shortcut 0x%x", s);
  if (o->state() && !tplate->state())
    write_string("value 1");

  if (is_valuator()) {
    fltk::Valuator* v = (fltk::Valuator*)o;
    fltk::Valuator* f = (fltk::Valuator*)(tplate);
    if (v->minimum()!=f->minimum()) write_string("minimum %g",v->minimum());
    if (v->maximum()!=f->maximum()) write_string("maximum %g",v->maximum());
    if (v->step()!=f->step()) write_string("step %g",v->step());
    if (v->linesize_setting()!=f->linesize_setting())
      write_string("linesize %d",v->linesize());
    if (v->value()!=0.0) write_string("value %g",v->value());
    if (is_valuator()==2) {
      double x = ((fltk::Slider*)v)->slider_size();
      double y = ((fltk::Slider*)f)->slider_size();
      if (x != y) write_string("slider_size %g", x);
    }
  }

  if (extra_code()) {
    write_indent(indent);
    write_string("extra_code");
    write_word(extra_code());
  }
  if (user_class()) {
    write_indent(indent);
    write_string("class");
    write_word(user_class());
  }
}

int pasteoffset;

void WidgetType::read_property(const char *c) {
  fltk::Widget* o = this->o;
  int x,y,w,h;
  if (!strcmp(c,"private")) {
    public_ = 0;
  } else if (!strcmp(c,"xywh")) {
    if (sscanf(read_word(),"%d %d %d %d",&x,&y,&w,&h) == 4) {
      x += pasteoffset;
      y += pasteoffset;
      // adjust for older relative coordinates:
      if (read_version < 2.0001 && !is_menu_item() && o->parent()) {
	fltk::Group* p = o->parent();
	while (p->parent()) {x -= p->x(); y -= p->y(); p = p->parent();}
      }
      o->x(x); o->y(y); o->w(w); o->h(h);
      if (h > w) o->set_vertical(); else o->set_horizontal();
      //o->layout();
    }
  } else if (!strcmp(c,"horizontal")) {
    o->set_horizontal();
  } else if (!strcmp(c,"vertical")) {
    o->set_vertical();
  } else if (!strcmp(c,"type")) {
    const char* c = read_word();
    // Strip off leading "VERTICAL|fltk::Type::" written by older fltk2 fluid:
    if (!strncmp(c,"VERTICAL",8)) c += 8;
    else if (!strncmp(c,"HORIZONTAL",10)) c += 10;
    if (*c == '|') {
      c++;
      const char* d = c;
      while (*d && *d != '|') if (*d++ == ':') c = d;
    }
    o->type(number_from_text(c, subtypes()));
  } else if (!strcmp(c,"set_xy")) {
    set_xy = true;
  } else if (!strcmp(c,"align")) {
    o->align((int)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"when")) {
    o->when(number_from_text(read_word(), whenmenu));
  } else if (!strcmp(c,"hide")) {
    o->hide();
  } else if (!strcmp(c,"deactivate")) {
    o->deactivate();
  } else if (!strcmp(c,"resizable")) {
    resizable(1);
  } else if (!strcmp(c,"hotspot")) {
    hotspot(1);
  //} else if (!strcmp(c, "divider")) { // do something here...
  } else if (!strcmp(c,"class")) {
    user_class(read_word());
  } else if (!strcmp(c,"shortcut")) {
    o->shortcut(strtol(read_word(),0,0));
  } else if (!strcmp(c,"box") || !strcmp(c,"text_box") || !strcmp(c, "window_box")) {
    const char* value = read_word();
    const Enumeration* e = from_text(value, boxmenu);
    if (e) o->box((fltk::Box*)(e->compiled));
    else read_error("Box* '%s' not found", value);
  } else if (!strcmp(c,"buttonbox") || !strcmp(c,"button_box") || !strcmp(c,"glyph_box")) {
    const char* value = read_word();
    const Enumeration* e = from_text(value, boxmenu);
    if (e) o->buttonbox((fltk::Box*)(e->compiled));
    else read_error("Box* '%s' not found", value);
  } else if (!strcmp(c, "down_box")) { // ignore this fltk 1.0 item
    read_word();
  } else if (!strcmp(c,"labelfont") || !strcmp(c,"label_font")) {
    if (sscanf(read_word(),"%d",&x) == 1) o->labelfont(fltk::font(x));
  } else if (!strcmp(c,"textfont") || !strcmp(c,"text_font")) {
    if (sscanf(read_word(),"%d",&x) == 1) o->textfont(fltk::font(x));
  } else if (!strcmp(c,"labeltype") || !strcmp(c,"label_type")
	     || !strcmp(c,"label_style")) {
    c = read_word();
    // back compatability with 1.0 and Vincent's original graphical patch:
    if (!strncmp(c,"image",5) || !strcmp(c, "image_file")) {
      // WAS: since it used the label(), I don't think mulitple images
      // could have been supported.
      Fluid_Image *i = Fluid_Image::find(label());
      if (!i) read_error("Image file '%s' not found", label());
      set_image(i); label(0);
      if (!strcmp(c,"image_file")) {
        c = read_word();
        if (i && c[0]=='d') i->inlined = 0;
        // if (c[1]=='t') do something here to make it fltk::Tiled_Image
      }
    } else {
      const Enumeration* e = from_text(c, labelstylemenu);
      if (e) o->labeltype((fltk::LabelType*)(e->compiled));
    }

  } else if (!strcmp(c,"image") || !strcmp(c,"image1")) {
    c = read_word();
    // back compatability with a prefixed inlined indicator:
    bool inlined = true;
    if (!strcmp(c, "inlined")) {
      c = read_word();
    } else if (!strcmp(c, "not_inlined")) {
      inlined = 0;
      c = read_word();
    }
    Fluid_Image *i = Fluid_Image::find(c);
    if (!i) read_error("Image file '%s' not found", c);
    else {
      set_image(i);
      if (!inlined) i->inlined = 0;
    }

  } else if (!strcmp(c,"image2")||!strcmp(c,"image3")||!strcmp(c,"image4")) {
    // Read the multi-image api, this should be converted to a new
    // MultiImage instance for compatability. Unfortunatly I don't have
    // time for that, and instead throw all except image1 away...
    // 2 = INACTIVE_R
    // 3 = HIGHLIGHT
    // 4 = PUSHED

  } else if (!strcmp(c,"color") || !strcmp(c,"text_background")
	     || !strcmp(c,"off_color") || !strcmp(c,"window_color")) {
    char* p;
    o->color((fltk::Color)strtoul(read_word(),&p,0));
    // back compatability with very old fluid versions that wrote 2 numbers:
    if (*p) o->selection_color((fltk::Color)strtoul(p,0,0));
  } else if (!strcmp(c,"labelcolor") || !strcmp(c,"label_color")) {
    o->labelcolor((fltk::Color)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"selection_color")) {
    o->selection_color((fltk::Color)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"selection_textcolor") ||
	     !strcmp(c,"selected_text_color") || !strcmp(c,"selected_textcolor")) {
    o->selection_textcolor((fltk::Color)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"buttoncolor") ||
	     !strcmp(c,"button_color") || !strcmp(c,"off_color")) {
    o->buttoncolor((fltk::Color)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"highlight_color")) {
    o->highlight_color((fltk::Color)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"highlight_textcolor") ||
	     !strcmp(c,"highlight_labelcolor") ||
	     !strcmp(c,"highlight_label_color")) {
    o->highlight_textcolor((fltk::Color)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"textcolor") || !strcmp(c,"text_color")) {
    o->textcolor((fltk::Color)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"labelsize") || !strcmp(c,"label_size")) {
    o->labelsize((float)strtoul(read_word(),0,0));
  } else if (!strcmp(c,"textsize") || !strcmp(c,"text_size")) {
    o->textsize((float)strtoul(read_word(),0,0));

  } else if (!strcmp(c,"minimum") && is_valuator()) {
    ((fltk::Valuator*)o)->minimum(strtod(read_word(),0));
  } else if (!strcmp(c,"maximum") && is_valuator()) {
    ((fltk::Valuator*)o)->maximum(strtod(read_word(),0));
  } else if (!strcmp(c,"step") && is_valuator()) {
    ((fltk::Valuator*)o)->step(strtod(read_word(),0));
  } else if (!strcmp(c,"linesize") && is_valuator()) {
    ((fltk::Valuator*)o)->linesize(strtod(read_word(),0));
  } else if (!strcmp(c,"value")) {
    if (is_valuator()) {
      ((fltk::Valuator*)o)->value(strtod(read_word(),0));
    } else {
      const char* value = read_word();
      o->state(value[0]!=0 && value[0]!='0');
    }
  } else if ((!strcmp(c,"slider_size")||!strcmp(c,"size"))&&is_valuator()==2) {
    double v = strtod(read_word(),0);
    if (v < 1.0)
      ((fltk::Slider*)o)->slider_size(int(v*o->w()));
    else
      ((fltk::Slider*)o)->slider_size(int(v));

  } else if (!strcmp(c, "extra_code")) {
    extra_code(read_word());
  } else if (!strncmp(c,"code",4)) {
    // for back-compatabilty all old code0, code1, ... fields are concatenated
    const char* newtext = read_word();
    const char* oldtext = extra_code();
    if (!oldtext) {extra_code(newtext); return;}
    int n = strlen(oldtext)+1+strlen(newtext)+1;
    char* buffer = new char[n];
    sprintf(buffer, "%s\n%s", oldtext, newtext);
    extra_code(buffer);
    delete[] buffer;
  } else {
    FluidType::read_property(c);
  }
}

static const Enumeration boxmenu1[] = {
  // these extra ones are for looking up fdesign saved strings:
  {"x", "NO_FRAME",		(void *)fltk::NO_BOX},
  {"x", "ROUNDED3D_UPBOX",	(void *)fltk::ROUND_UP_BOX},
  {"x", "ROUNDED3D_DOWNBOX",	(void *)fltk::ROUND_DOWN_BOX},
  {"x", "OVAL3D_UPBOX",		(void *)fltk::ROUND_UP_BOX},
  {"x", "OVAL3D_DOWNBOX",	(void *)fltk::ROUND_DOWN_BOX},
  {"x", "0",			(void *)fltk::NO_BOX},
  {"x", "1",			(void *)fltk::UP_BOX},
  {"x", "2",			(void *)fltk::DOWN_BOX},
  {"x", "3",			(void *)fltk::FLAT_BOX},
  {"x", "4",			(void *)fltk::BORDER_BOX},
  {"x", "5",			(void *)fltk::SHADOW_BOX},
  {"x", "6",			(void *)fltk::ENGRAVED_BOX},
  {"x", "7",			(void *)fltk::ROUNDED_BOX},
  {"x", "8",			(void *)fltk::RFLAT_BOX},
  {"x", "9",			(void *)fltk::RSHADOW_BOX},
  {"x", "10",			(void *)fltk::UP_BOX},
  {"x", "11",			(void *)fltk::DOWN_BOX},
{0}};

extern int fdesign_flip;
extern int lookup_fdesign(const char *name, int &v, int numberok=0);

int WidgetType::read_fdesign(const char* name, const char* value) {
  int v;
  if (!strcmp(name,"box")) {
    float x,y,w,h;
    if (sscanf(value,"%f %f %f %f",&x,&y,&w,&h) == 4) {
      if (fdesign_flip) {
	FluidType *p;
	for (p = parent; p && !p->is_window(); p = p->parent);
	if (p && p->is_widget()) y = ((WidgetType*)p)->o->h()-(y+h);
      }
      x += pasteoffset;
      y += pasteoffset;
      o->resize(int(x),int(y),int(w),int(h));
      if (h>w) o->set_vertical();
      o->layout();
    }
  } else if (!strcmp(name,"label")) {
    label(value);
    if (value[0] == '@') o->labeltype(fltk::SYMBOL_LABEL);
  } else if (!strcmp(name,"name")) {
    this->name(value);
  } else if (!strcmp(name,"callback")) {
    callback(value); user_data_type("long");
  } else if (!strcmp(name,"argument")) {
    user_data(value);
  } else if (!strcmp(name,"shortcut")) {
    if (value[0]) {
      char buf[128]; sprintf(buf,"o->shortcut(\"%s\");",value);
      extra_code(buf);
    }
  } else if (!strcmp(name,"style")) {
    if (!strncmp(value,"fltk::NORMAL",9)) return 1;
    if (!lookup_fdesign(value,v,1)) return 0;
    o->labelfont(fltk::font(v)); o->labeltype((fltk::LabelType*)(v>>8));
  } else if (!strcmp(name,"size")) {
      if (!lookup_fdesign(value,v,1)) return 0;
    o->labelsize((float)v);
  } else if (!strcmp(name,"type")) {
    if (!strncmp(value,"NORMAL",6)) return 1;
    if (lookup_fdesign(value,v,1)) {o->type(v); return 1;}
    if (!strcmp(value+strlen(value)-5,"FRAME")) goto TRY_BOXTYPE;
    if (!strcmp(value+strlen(value)-3,"BOX")) goto TRY_BOXTYPE;
    return 0;
  } else if (!strcmp(name,"lcol")) {
    if (!lookup_fdesign(value,v,1)) return 0;
    o->labelcolor(v);
  } else if (!strcmp(name,"return")) {
    if (!lookup_fdesign(value,v,0)) return 0;
    o->when(v|fltk::WHEN_RELEASE);
  } else if (!strcmp(name,"alignment")) {
    if (!lookup_fdesign(value,v)) {
      // convert old numeric values:
      int v1 = strtol(value,0,0); if (v1 <= 0 && strcmp(value,"0")) return 0;
      v = 0;
      if (v1 >= 5) {v = fltk::ALIGN_INSIDE; v1 -= 5;}
      switch (v1) {
      case 0: v += fltk::ALIGN_TOP; break;
      case 1: v += fltk::ALIGN_BOTTOM; break;
      case 2: v += fltk::ALIGN_LEFT; break;
      case 3: v += fltk::ALIGN_RIGHT; break;
      case 4: v += fltk::ALIGN_CENTER; break;
      default: return 0;
      }
    }
    o->align(v);
  } else if (!strcmp(name,"resizebox")) {
    resizable(1);
  } else if (!strcmp(name,"colors")) {
    char* p = (char*)value;
    while (*p != ' ') {if (!*p) return 0; p++;}
    *p = 0;
    int v1;
    if (!lookup_fdesign(value,v,1) || !lookup_fdesign(p+1,v1,1)) {
      *p=' '; return 0;}
    o->color(v); o->selection_color(v1);
  } else if (!strcmp(name,"resize")) {
    return !strcmp(value,"fltk::RESIZE_ALL");
  } else if (!strcmp(name,"gravity")) {
    return !strcmp(value,"Fl_NoGravity Fl_NoGravity");
  } else if (!strcmp(name,"boxtype")) {
  TRY_BOXTYPE:
    const Enumeration* e = from_text(value, boxmenu);
    if (!e) {e = from_text(value, boxmenu1); if (!e) return 0;}
    fltk::Box* x = (fltk::Box*)(e->compiled);
    if (x == fltk::NO_BOX) {
      if (o->box() != ((WidgetType*)factory)->o->box()) return 1; // kludge for frame
    }
    o->box(x);
  } else {
    return 0;
  }
  return 1;
}

////////////////////////////////////////////////////////////////
// live mode

#include "WindowType.h"

void leave_live_mode_cb(Widget*, void*);

Window *live_window = 0L;
void live_mode_cb(LightButton*o,void *v) {
  /// \todo live mode should end gracefully when the application quits
  ///       or when the user closes the live widget
  static FluidType *live_type = 0L;
  static Widget *live_widget = 0L;
  // if 'o' is 0, we must quit live mode
  if (!o) {
    o = wLiveMode;
    o->value(0);
  }
  if (o->value()) {
    if (numselected == 1) {
      live_widget = current_widget->enter_live_mode(1);
      if (live_widget) {
        live_type = current_widget;
        Group::current(0);
        int w = live_widget->w();
        int h = live_widget->h();
	// create main live mode Window, green backgnd no border
        live_window = new DoubleBufferWindow(w+20, h+55, "Fluid Live Mode Widget");
	live_window->begin();
        live_window->box(fltk::FLAT_BOX);
        live_window->color(fltk::GREEN);
        Button *btn = new Button(10, h+20, 100, 25, "Exit Live Mode");
        btn->labelsize(12);
        btn->callback(leave_live_mode_cb);
        live_widget->position(10, 10);
        live_window->add(live_widget);
        live_window->resizable(live_widget);
        live_window->set_modal(); // block all other UI
        live_window->callback(leave_live_mode_cb);
        if (current_widget->is_window()) {
          WindowType *w = (WindowType*)current_widget;
          int mw = w->sr_min_w; if (mw>0) mw += 20;
          int mh = w->sr_min_h; if (mh>0) mh += 55;
          int MW = w->sr_max_w; if (MW>0) MW += 20; 
          int MH = w->sr_max_h; if (MH>2) MH += 55;
          if (mw || mh || MW || MH)
            live_window->size_range(mw, mh, MW, MH);
        }
	live_window->end();
        live_window->show();
      } else o->value(0);
    } else o->value(0);
  } else {
    if (live_type)
      live_type->leave_live_mode();
    if (live_window) {
	live_window->hide();
	live_window->clear_changed();
      delete live_window; // was delete_widget(live_window); but deferred call obsolete in fltk2
    }
    live_type = 0L;
    live_widget = 0L;
    live_window = 0L;
  }
}

void leave_live_mode_cb(Widget*, void*) {
  live_mode_cb(0, 0);
}

fltk::Widget *WidgetType::enter_live_mode(int top) {
  live_widget = widget(o->x(), o->y(), o->w(), o->h());
  if (live_widget)
    copy_properties();
  return live_widget;
}

void WidgetType::leave_live_mode() {
}

/**
 * copy all properties from the edit widget to the live widget
 */
void WidgetType::copy_properties() {
  if (!live_widget) 
    return;

  // copy all attributes common to all widget types
  Widget *w = live_widget;
  if (o->style() ) {
      if (o->style()->dynamic()) w->copy_style(o->style());
      else w->style(o->style());
  }
  w->default_style = o->default_style;
  w->default_glyph= o->default_glyph;

  w->flags(o->flags());
  w->label(o->label());
  w->image(o->image());
  w->tooltip(tooltip());
  w->type(o->type());
    
  w->align(o->align());
  w->shortcut(o->shortcut());
  w->Rectangle::set(o->x(), o->y(),o->w(),o->h());

  // copy all attributes specific to widgets derived from Button
  if (is_button()) {
    Button* d = (Button*)live_widget, *s = (Button*)o;
    d->value(s->value());
  }

  // copy all attributes specific to Valuator and derived classes
  if (is_valuator()) {
    Valuator* d = (Valuator*)live_widget, *s = (Valuator*)o;
    d->minimum(s->minimum());
    d->maximum(s->maximum());
    d->step(s->step());
    d->value(s->value());
    if (is_valuator()==2) {
      Slider *d = (Slider*)live_widget, *s = (Slider*)o;
      d->slider_size(s->slider_size());
    }
  }
 
/* TODO: implement this
  {Font ff; int fs; Color fc; if (textstuff(4,ff,fs,fc)) {
    Font f; int s; Color c; textstuff(0,f,s,c);
    if (f != ff) write_string("textfont %d", f);
    if (s != fs) write_string("textsize %d", s);
    if (c != fc) write_string("textcolor %d", c);
  }}*/

  if (!o->visible()) 
    w->hide();
  if (!o->active()) 
    w->deactivate();
  if (resizable() && w->parent()) 
    w->parent()->resizable(o);
}

int WidgetType::textstuff(int w, fltk::Font* f, float& s, fltk::Color c) {
  fltk::Widget *myo = (fltk::Widget *)(w==4 ? ((WidgetType*)factory)->o : o);
  switch (w) {
    case 4:
    case 0: f = myo->textfont(); s = myo->textsize(); c = myo->textcolor(); break;
    case 1: myo->textfont(f); break;
    case 2: myo->textsize((float)s); break;
    case 3: myo->textcolor(c); break;
  }
  return 1;
}

//
// End of "$Id: WidgetType.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
