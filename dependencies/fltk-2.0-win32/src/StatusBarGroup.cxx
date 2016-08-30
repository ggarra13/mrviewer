//
// "$Id"
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

/*! \class fltk::StatusBarGroup

  This class permits to create and handle a statusbar with minimum efforts.<br>
    It features automatic positionning and resizing adapting to parent Group/Window.<br>
    Only height matters when constructing a StatusBarGroup. <br>
  You can use the set() API's to easily print formatted text 
  at one of the three standard position : left, center right.<br>
  You can also setup an optional custom box to the incorporated texts with child_box(), by default FLAT_BOX is used.<br>
  The StatusBarGroup widget can also be visually created in Fluid2.<br><br>

  Here's some typical code you can use to create a status bar:<br>
  Sample implementation:
\code
  my_status_bar = new StatusBarGroup(24);
  my_status_bar->child_box(StatusBarGroup::SBAR_RIGHT, THIN_DOWN_BOX);
  //... more code ...
  // sets a right-aligned formatted text :
  my_status_bar->set(StatusBarGroup::SBAR_RIGHT, "%d items", count); 
  // sets a centered text :
  my_status_bar->set("Hi", StatusBarGroup::SBAR_CENTER); 
  //... more code ...
  // using a null or 0-len text removes the text box :
  my_status_bar->set(0, StatusBarGroup::SBAR_CENTER); 
\endcode

*/

#include <fltk/StatusBarGroup.h>
#include <fltk/Box.h>
#include <fltk/InvisibleBox.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/draw.h>
#include <stdio.h>
#include <stdarg.h>

using namespace fltk;

const int BORDER_WIDTH=2;

static void revert(Style *s) {
    s->box_ = THIN_UP_BOX;
    //s->box_ = FLAT_BOX;
    s->color_ = GRAY75;
    s->labelsize_ = 10;
}
static NamedStyle style("StatusBarGroup", revert, &StatusBarGroup::default_style);
NamedStyle* StatusBarGroup::default_style = &::style;

void StatusBarGroup::init() {
  tf_[0] = tf_[1] = tf_[2] = 0;
  b_[0] = b_[1] = b_[2] = fltk::FLAT_BOX; 
  saved_h_=this->h();
  style(default_style);
  align(ALIGN_INSIDE);
  box(THIN_DOWN_BOX);
  resize_from_parent();
  resizable(0);
}

StatusBarGroup::~StatusBarGroup() {
    for (int i=0; i<3;i++) if (tf_[i]) delete tf_[i];
}

/** make sure that width will match optimally parent's width and that it will fit in the bottom
    also make sure that any child in the same group does not override the status bar and resize if necessary
*/
void StatusBarGroup::resize_from_parent() {
  // resize will work if you set resizable()
    if (!parent()) return;
    int i;
    Rectangle inside(0,0,parent()->w(),parent()->h());
    parent()->box()->inset(inside);
    x(inside.x());
    if (visible()) {
	w(inside.w()); // set proper width 
	if (h()==0) h(saved_h_);
        y(inside.b()-h());
	for (i = 0; i < parent()->children(); i++) {
	    Widget* w = parent()->child(i);
	    if (((Widget*) this)!= w) {
		int delta = w->b()-this->y();
		if (delta>0) {
		    w->resize(w->x(), w->y(), w->w(), w->h()-delta > 0 ? w->h()-delta : 0);
		    if (w->is_group()) ((Group*)w)->init_sizes();
		    w->redraw();
		}
	    }
	}
	for (i=0; i<3;i++) update_box(tf_[i], (StatusBarGroup::Position) i);
    } else {
	if (h()!=0) saved_h_ = h();
	w(0);h(0);
	bool c = false;
	for (i = 0; i < parent()->children(); i++) {
	    Widget* w = parent()->child(i);
	    if (((Widget*) this)!= w) {
		int delta = w->b()-this->y();
		if (delta==0) {
		    w->resize(w->x(), w->y(), w->w(), w->h()+saved_h_);
		    if (w->is_group())  {
			((Group*)w)->init_sizes();
		    }
		    w->relayout();
		    w->redraw();
		    c=true;
		}
	    }
	}
	if(c) {parent()->relayout();}
    }
}

//! sets the label at a particular pos, automatically handles alloc + drawing
void StatusBarGroup::set(const char * t, StatusBarGroup::Position pos) {

    if (!t || !*t) {
	if (tf_[pos]) {delete tf_[pos]; tf_[pos]=0;}
	return;
    }

    if (!tf_[pos]) {
	Group::current(0);
	tf_[pos]= new InvisibleBox(b_[pos],box()->dx(),box()->dh()+BORDER_WIDTH,10,10,0);
	tf_[pos]->align(ALIGN_LEFT|ALIGN_INSIDE);
	add(tf_[pos]);
    }
    tf_[pos]->copy_label(t);

    update_box(tf_[pos], pos);

    redraw();
}

void StatusBarGroup::update_box(InvisibleBox *b, Position pos) {
    if (!b) return;
    int X=0, Y=0;
    b->measure_label(X,Y);
    X += (b_[pos]->dw()+1)*2;
    b->resize(X, h()-(box()->dh()+BORDER_WIDTH)*2);
    switch (pos) {
    case SBAR_LEFT:
	b->x(box()->dx());
	break;
    case SBAR_CENTER:
	b->x((r() - b->w())/2);
	break;
    case SBAR_RIGHT:
	b->x(r()-b->w()-box()->dw()-BORDER_WIDTH);
	break;
    }
    b->damage();
}
//! sets the label at a particular pos, automatically handles alloc + drawing, variable parmeter format
void StatusBarGroup::set(StatusBarGroup::Position pos, const char* format,...) {
  static char buffer[512];
  va_list args;
  va_start(args, format);
  vsprintf(buffer, format, args);
  va_end(args);
  set(buffer,pos);
}

// make sure no widget will overwrite the status bar in the parent
void StatusBarGroup::layout() {
    Group::layout();
    resize_from_parent();
}
void StatusBarGroup::show() {
    bool v = visible();
    Group::show();
    if(!v) resize_from_parent();
}

void StatusBarGroup::hide() {
    bool v = visible();
    Group::hide();
    if(v) resize_from_parent();
}

//
// End of "$Id"
//
