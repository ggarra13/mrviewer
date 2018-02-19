//
// "$Id: align_widget.cxx 4561 2005-09-09 11:36:36Z matt $"
//
// Alignment code for the Fast Light Tool Kit (FLTK).
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
//     http://www.fltk.org/str.php
//

#include <fltk/run.h>
#include <fltk/Window.h>
#include <fltk/Font.h>
#include "WidgetType.h"
#include "undo.h"

using namespace fltk;
/**
 * the first behaviour always uses the first selected widget as a reference
 * the second behaviour uses the largest widget (most extreme positions) as
 * a reference.
 */
#define BREAK_ON_FIRST break
//#define BREAK_ON_FIRST

void align_widget_cb(Widget*, long how) 
{
  const int max = 32768, min = -32768;
  int left, right, top, bot, wdt, hgt, n;
  FluidType *o;
  int changed = 0;
  switch ( how )
  {
  //---- align
  case 10: // align left
    left = max;
    for (o = FluidType::first; o; o = o->walk(0)) 
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->x()<left)
	  left = w->x();
	BREAK_ON_FIRST;
      }
    if (left!=max)
	for (FluidType* o = FluidType::first; o; o = o->walk(0)) 
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize(left, w->y(), w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    break;
  case 11: // align h.center
    left = max; right = min;
    for (o = FluidType::first; o; o = o->walk(0)) 
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->x()<left)
	  left = w->x();
	if (w->x()+w->w()>right)
	  right = w->x()+w->w();
	BREAK_ON_FIRST;
      }
    if (left!=max)
    {
      int center2 = left+right;
      for (FluidType *o = FluidType::first; o; o = o->walk(0)) 
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize((center2-w->w())/2, w->y(), w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    }
    break;
  case 12: // align right
    right = min;
    for (o = FluidType::first; o; o = o->walk(0)) 
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->x()+w->w()>right)
	  right = w->x()+w->w();
	BREAK_ON_FIRST;
      }
    if (right!=min)
      for (FluidType *o = FluidType::first; o; o = o->walk(0)) 
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize(right-w->w(), w->y(), w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    break;
  case 13: // align top
    top = max;
    for (o = FluidType::first; o; o = o->walk(0)) 
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->y()<top)
	  top = w->y();
	BREAK_ON_FIRST;
      }
    if (top!=max)
      for (FluidType *o = FluidType::first; o; o = o->walk(0)) 
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize(w->x(), top, w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    break;
  case 14: // align v.center
    top = max; bot = min;
    for (o = FluidType::first; o; o = o->walk(0)) 
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->y()<top)
	  top = w->y();
	if (w->y()+w->h()>bot)
	  bot = w->y()+w->h();
	BREAK_ON_FIRST;
      }
    if (top!=max)
    {
      int center2 = top+bot;
      for (FluidType *o = FluidType::first; o; o = o->walk(0))
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize(w->x(), (center2-w->h())/2, w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    }
    break;
  case 15: // align bottom
    bot = min;
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->y()+w->h()>bot)
	  bot = w->y()+w->h();
	BREAK_ON_FIRST;
      }
    if (bot!=min)
      for (FluidType *o = FluidType::first; o; o = o->walk(0))
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize( w->x(), bot-w->h(), w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    break;
  //---- space evently
  case 20: // space evenly across
    left = max; right = min; wdt = 0, n = 0;
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->x()<left)
	  left = w->x();
	if (w->x()+w->w()>right)
	  right = w->x()+w->w();
	wdt += w->w();
	n++;
      }
    wdt = (right-left)-wdt;
    n--;
    if (n>0)
    {
      int cnt = 0, wsum = 0;
      for (FluidType *o = FluidType::first; o; o = o->walk(0))
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize(left+wsum+wdt*cnt/n, w->y(), w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	  cnt++;
	  wsum += w->w();
	}
    }
    break;
  case 21: // space evenly down
    top = max; bot = min; hgt = 0, n = 0;
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->y()<top)
	  top = w->y();
	if (w->y()+w->h()>bot)
	  bot = w->y()+w->h();
	hgt += w->h();
	n++;
      }
    hgt = (bot-top)-hgt;
    n--;
    if (n>0)
    {
      int cnt = 0, hsum = 0;
      for (FluidType *o = FluidType::first; o; o = o->walk(0))
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize(w->x(), top+hsum+hgt*cnt/n, w->w(), w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	  cnt++;
	  hsum += w->h();
	}
    }
    break;
  //---- make same size
  case 30: // same width
    wdt = min;
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->w()>wdt)
	  wdt = w->w();
	BREAK_ON_FIRST;
      }
    if (wdt!=min)
      for (FluidType *o = FluidType::first; o; o = o->walk(0))
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize(w->x(), w->y(), wdt, w->h());
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    break;
  case 31: // same height
    hgt = min;
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->h()>hgt)
	  hgt = w->h();
	BREAK_ON_FIRST;
      }
    if (hgt!=min)
      for (FluidType *o = FluidType::first; o; o = o->walk(0))
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize( w->x(), w->y(), w->w(), hgt);
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    break;
  case 32: // same size
    hgt = min; wdt = min;
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget())
      {
	Widget *w = ((WidgetType *)o)->o;
	if (w->w()>wdt)
	  wdt = w->w();
	if (w->h()>hgt)
	  hgt = w->h();
	BREAK_ON_FIRST;
      }
    if (hgt!=min)
      for (FluidType *o = FluidType::first; o; o = o->walk(0))
	if (o->selected() && o->is_widget())
	{
	  if (!changed) {
	    changed = 1;
	    modflag = 1;
	    Undo::checkpoint();
	  }

	  Widget *w = ((WidgetType *)o)->o;
          w->resize( w->x(), w->y(), wdt, hgt);
	  w->redraw();
	  if (w->window()) w->window()->redraw();
	}
    break;
  //---- center in group
  case 40: // center hor
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget() && o->parent)
      {
	if (!changed) {
	  changed = 1;
	  modflag = 1;
	  Undo::checkpoint();
	}

	Widget *w = ((WidgetType *)o)->o;
	Widget *p = ((WidgetType *)o->parent)->o;
	int center2;

	if (w->window() == p) center2 = p->w();
	else center2 = 2*p->x()+p->w();
        w->resize((center2-w->w())/2, w->y(), w->w(), w->h());
	w->redraw();
	if (w->window()) w->window()->redraw();
      }
    break;
  case 41: // center vert
    for (o = FluidType::first; o; o = o->walk(0))
      if (o->selected() && o->is_widget() && o->parent)
      {
	if (!changed) {
	  changed = 1;
	  modflag = 1;
	  Undo::checkpoint();
	}

	Widget *w = ((WidgetType *)o)->o;
	Widget *p = ((WidgetType *)o->parent)->o;
	int center2;

	if (w->window() == p) center2 = p->h();
	else center2 = 2*p->y()+p->h();
        w->resize(w->x(), (center2-w->h())/2, w->w(), w->h());
	modflag = 1;
	w->redraw();
	if (w->window()) w->window()->redraw();
      }
    break;
  }
}

// Set sizes of selected widgets...
void widget_size_cb(Widget *, long size) {
  // Update any selected widgets...
  int changed = 0;
  for (FluidType *o = FluidType::first; o; o = o->walk(0)) {
    if (o->selected() && o->is_widget()) {
      if (!changed) {
	changed = 1;
	modflag = 1;
	Undo::checkpoint();
      }

      Widget *w = ((WidgetType *)o)->o;
      w->labelsize((float) size);
      Font f;
      float s = (float) size;
      Color c=fltk::BLACK;
      ((WidgetType *)o)->textstuff(2, &f, s, c);

      w->redraw();
      // since this may be a major change, the whole window should be redrawn
      if (w->window()) w->window()->redraw();
    }
  }
}


//
// End of "$Id: align_widget.cxx 4561 2005-09-09 11:36:36Z matt $".
//

