// "$Id: Browser.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
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

#include <fltk/Browser.h>
#include <fltk/ItemGroup.h>
#include <fltk/Item.h>
#include <fltk/Button.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/Box.h>
#include <fltk/Item.h>
#include <fltk/draw.h>
#include <fltk/error.h>
#include <fltk/Cursor.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace fltk;

/*! \class fltk::Browser

  Displays a scrolling vertical list of text widgets, possibly with a
  hierarchical arrangement, and lets the user select one of them.

  \image html browser.gif

  The items may be created as child widgets (usually the same widgets
  as are used to create menus: fltk::Item widgets, or fltk::ItemGroup
  widgets to make a hierarchy).

  All the functions used to add, remove, or modify items in the list
  are defined by the base class fltk::Menu. See that for much more
  information.  For a simple constant list you can populate the list
  by calling browser->add("text of item") once for each item.
  \see
  - add()
  - add_group()
  - add_leaf()

  You can also us an fltk::List which allows you to control
  the storage by dynamically creating a temporary "fake" widget for
  the browser to use each time it wants to look at or draw an item.
  This is useful for creating browsers with hundreds of thousands
  of items, or when the set of items changes rapidly.

  If you give the browser a callback you can find out what item was
  selected with value(), the first item is zero (this is different
  from older versions of fltk that started at 1!), and will be
  negative if no item is selected. You can change the selected item
  with value(new_value).

  The subclass fltk::MultiBrowser lets the user select more than one
  item at the same time.

  The callback() is done when the user changes the selected items or
  when they open/close parents. In addition, if the user double-clicks
  a non-parent item, then it is "executed" which usually means that
  the callback() on the item itself is run. However, if no callback
  has been set on the item, the callback() of this widget is called
  with the user_data() of the item.

  You can control when callbacks are done with the when() method. The
  following values are useful, the default value is fltk::WHEN_CHANGED.

  - fltk::WHEN_NEVER - Callback is never done. changed() can be used
    to see if the user has modified the browser.

  - fltk::WHEN_CHANGED - Callback is called on every change to each
    item as it happens. The method item() will return the one that
    is being changed. Notice that selecting large numbers in a mulit
    browser will produce large numbers of callbacks.

  - fltk::WHEN_RELEASE - Callback is done when the user releases the
    mouse after some changes, and on any keystroke that
    changes the item. For a multi browser you will only be able
    to find out all the changes by scanning all the items in the callback.

  - fltk::WHEN_RELEASE_ALWAYS - Callback is done when the user
    releases the mouse even if the current item has not changed, and
    on any arrow keystroke even when at the top or bottom of the
    browser.

  - fltk::WHEN_ENTER_KEY - If you turn this on then the enter key is
    a shortcut and executes the current item like double-click.

*/

////////////////////////////////////////////////////////////////
// Compile-time style configuration options.
// For "run time configuration" copy this file, rename the widget,
// compile with your desired options, and use your renamed widget.
// Deal with it.

#define DRAW_STRIPES 0 // draw Macintosh-style stripes
#define TRIANGLE_GLYPH 0 // use Macintosh/fltk-style glyphs

/*! \fn int Browser::width() const
  \return The width of the longest item in the browser, measured in pixels. 
  
  If this is changed (by adding or deleting items or opening or closing 
  a parent item) then layout() must be called before this is correct.
*/

/*! \fn int Browser::height() const
  \return The height of all the visible items in the browser, measured in
  pixels. 
  
  If this is changed (by adding or deleting items or opening
  or closing a parent item) then layout() must be called before this
  is correct.
*/

/*! \fn int Browser::box_width() const
  \return The width of the display area of the browser in pixels; 
  this is w() minus the edges of the box() minus the width of the 
  vertical scrollbar, if visible. 
  
  If this is changed (by resizing the widget, adding or deleting items 
  or opening or closing a parent item such that the scrollbar visibility 
  changes) then layout() must be called before this is correct.
*/

/*! \fn int Browser::box_height() const
  \return The height of the display area of the browser in pixels,
  this is h() minus the edges of the box() minus the height of the 
  horizontal scrollbar, if visible. 
  
  If this is changed (by resizing the widget,
  adding or deleting items or opening or closing a parent item such
  that the scrollbar visibility changes) then layout() must be called
  before this is correct.
*/

/*! \fn void Browser::indented(bool v)
  \param v Turn this on to for space to be reserved for open/close 
  boxes drawn to the left of top-level items. You usually want this
  for a hierarchial browser. This should be off for a flat browser,
  or to emulate Windows Explorer where "My Computer" does not have 
  an open/close to the left of it.\n
  The default value is false.
*/

/*! \fn Widget *Browser::header(int col)
  \param col The index of the required column
  \return pointer to Widget in column \a col, starting from index 0.
  If column \a col is invalid, NULL is returned.
*/

/*! \fn int Browser::nheader() const
  \return The number of columns in the Browser.
*/

////////////////////////////////////////////////////////////////
// Moving between items:

// The Browser has space to store a fixed number of marks.

// The "current item" is the one pointed to by the mark called HERE.
// A pointer to it is stored in item(). To allow List to work, this
// pointer is only valid until the next item is referenced, so don't
// copy it to any storage. You can move the current item forward and
// backward, skipping invisible items. You can also place it on any
// item, even ones that cannot be seen, by using goto_item.

/*!
  \return true if the current item would be visible to the user 
  if the browser was scrolled to the correct location. This means 
  that the fltk::INVISIBLE flag is not set on it, and all parents 
  of it are open and visible as well.
*/
bool Browser::item_is_visible() const {
  return HERE.open_level >= HERE.level && item() && item()->visible();
}

/*! This function increases the number of levels we can store in each
  mark, and sets the level of the current mark to n.
  \param n The requested level of the current mark */
void Browser::set_level(unsigned n) {
  if (n+1 > HERE.indexes_size) {
    int* newlist = new int[n+1];
    memcpy(newlist, HERE.indexes, n*sizeof(int));
    if (HERE.indexes != &HERE.index0) delete[] HERE.indexes;
    HERE.indexes = newlist;
    HERE.indexes_size = n+1;
  }
  HERE.level = n;
}

/*!
  Because of the hierarchial structure it is difficult to identify
  an item in the browser. Instead of passing an identifier to all the
  calls that can modify an item, the browser provides several calls to
  set item() based on various rules, and then calls to modify the
  current item().

  This call sets item() to the very first visible widget in
  the browser. It returns the widget for that item, or null if the
  browser is empty.

  If you have invisible items in the browser you should use
  goto_index(0) if you want to go to the first item even if it is
  invisible.

  \returns the Widget for the very first visible Item, or NULL if this
  doesn't exist
*/
Widget* Browser::goto_top() {
  HERE.level = 0;
  HERE.open_level = 0;
  HERE.position = 0;
  HERE.indexes[0] = 0;
  siblings = children(HERE.indexes,0);
  // empty browser must return a null widget
  if (siblings <= 0) {
    item(0);
  } else {
    item(child(HERE.indexes,0));
    // skip leading invisible widgets:
    if (!item()->visible()) return next_visible();
  }
  return item();
}

/*! \fn Widget* Browser::goto_focus()
  Sets the item() to the "focus" (the item with the dotted square in
  an fltk::MultiBrowser, and the selected item in a normal
  fltk::Browser.
  \return The focused Item's Widget.
*/

/*! Go to a particular Mark
  \param mark The Mark to traverse to
  \return The Widget referred to by this mark */
Widget* Browser::goto_mark(const Mark& mark) {
  HERE = mark;
  for (unsigned L = 0; L <= HERE.level; L++) {
    int i = HERE.indexes[L];
    siblings = children(HERE.indexes, L);
    if (i < 0 || i >= siblings) {item(0); return 0;}
  }
  item(child(HERE.indexes, HERE.level));
  return item();
}

/** Copy constructor. Copies one Mark to another
  \param source The Mark to copy */
Browser::Mark::Mark(const Mark& source) {
  level = source.level;
  open_level = source.open_level;
  position = source.position;
  indexes_size = level+1;
  if (indexes_size > 1)
    indexes = new int[indexes_size];
  else
    indexes = &index0;
  for (unsigned i=0; i < indexes_size; i++)
    indexes[i] = source.indexes[i];
}

/** Assignment. Overwrites the current Mark with source
  \param source The Mark to overwrite with
*/
const Browser::Mark& Browser::Mark::operator=(const Mark& source) {
  if (&source != this) {
    if (indexes != &index0) delete[] indexes;
    level = source.level;
    open_level = source.open_level;
    position = source.position;
    indexes_size = level+1;
    if (indexes_size > 1)
      indexes = new int[indexes_size];
    else
      indexes = &index0;
    for (unsigned i=0; i < indexes_size; i++)
      indexes[i] = source.indexes[i];
  }
  return *this;
}

// compare the relative locations of these two marks
// return < 0 if mark1 is before mark2
// return   0 if mark1 is mark2
// return > 0 if mark1 is after mark2

static int compare_marks(const int* index1, unsigned L1,
                         const int* index2, unsigned L2) {
  for (unsigned L = 0; ; L++) {
    if (L > L1) {
      if (L > L2) return 0;
      return -1; // mark1 is on a parent of mark2
    } else if (L > L2) {
      return +1; // mark2 is on a parent of mark1
    }
    int n = index1[L] - index2[L];
    if (n) return n > 0 ? +2 : -2;
  }
}

/** Compares the current mark's position to another mark's position
  \param mark2 The mark to compare with
  \return -2 or less if this is before mark2\n
          -1 if this is a parent of mark2\n
           0 if this is equal to mark2\n
           1 if this is a child of mark2\n
           2 or greater if this is after mark2\n
*/
int Browser::Mark::compare(const Mark& mark2) const {
  return ::compare_marks(indexes, level, mark2.indexes, mark2.level);
}

/*!  \return true if the current item is a parent. Note that it may
  have zero children. */
bool Browser::item_is_parent() const {
  return children(HERE.indexes, HERE.level+1) >= 0;
}

/** \return true if item_is_parent() is true and this item is open. \n
    If this is not a parent the result is undefined. */
bool Browser::item_is_open() const {
  if (item()->flag(fltk::OPENED)) return true;
  for (unsigned i = 0; i <= HERE.level; i++) {
    if (i > OPEN.level) return false;
    if (HERE.indexes[i] != OPEN.indexes[i]) return false;
  }
  return true;
}

/** Gets the current item's height and calls layout if needed
  \return The current item's height. 
*/
int Browser::item_h() const {
  if (!item()->h()) item()->layout();
  return item()->h();
}

/*! Move forward to the next visible Item (what the down-arrow does).
  This does not move and returns null if we are at the bottom.
  \return The next visible Item's Widget or NULL if this isn't possible
*/
Widget* Browser::next_visible() {
  if (item_is_visible()) {
    HERE.position += item_h();
    // If we are on an open group title with children, go to first item in group
    if (item_is_open() && item_is_parent()) {
      int n = HERE.level+1;
      set_level(n);
      HERE.open_level = n;
      HERE.indexes[n] = 0;
      siblings = children(HERE.indexes, n);
    } else {
      // go to next item in this group
      HERE.indexes[HERE.level] ++;
    }
  } else {
    // item is not visible, point after it (or after closed/invisible parent)
    HERE.level = HERE.open_level;
    siblings = children(HERE.indexes, HERE.level);
    HERE.indexes[HERE.level] ++;
  }

  // loop to find the next real item:
  for (;;) {

    if (HERE.indexes[HERE.level] >= siblings) {
      // we moved off the end of a group
      if (!HERE.level) {item(0); return 0;} // end of the entire browser
      HERE.open_level = --HERE.level;
      HERE.indexes[HERE.level] ++;
      siblings = children(HERE.indexes, HERE.level);
      continue;
    }

    item(child(HERE.indexes, HERE.level));

    // skip invisible items:
    if (item()->visible()) break;
    HERE.indexes[HERE.level] ++;
  }

  return item();
}

/*! Move backward to previous visible item:
  This does not move and returns null if we are at the top. 
  \return The Widget of the previous Item, or NULL if this is not possible
*/
Widget* Browser::previous_visible() {

  // if we are on a child of a closed or invisible parent, pretend
  // we are on the item after that parent:
  if (HERE.level > HERE.open_level) {
    HERE.level = HERE.open_level;
    HERE.indexes[HERE.level] ++;
  }

  // there's got to be simpler logic for this loop...
  for (;;) {
    // go up to parent from first item in a group:
    if (!HERE.indexes[HERE.level]) {
      if (!HERE.level) { // start of browser
	HERE.position = 0;
	return 0;
      }
      HERE.open_level = --HERE.level;
      item(child(HERE.indexes, HERE.level));
      siblings = children(HERE.indexes, HERE.level);
      break;
    }

    // go back to previous item in this group:
    HERE.indexes[HERE.level] --;
    item(child(HERE.indexes, HERE.level));

    // go to last child in a group:
    while (item_is_open() && item()->visible() && item_is_parent()) {
      int n = children(HERE.indexes, HERE.level+1);
      if (n <= 0) break; // the group is empty, remain on it's title
      set_level(HERE.level+1);
      HERE.open_level = HERE.level;
      HERE.indexes[HERE.level] = n-1;
      item(child(HERE.indexes, HERE.level));
      siblings = n;
    }

    if (item()->visible()) break;
  }

  HERE.position -= item_h();
  return item();
}

/*!  Move the current item to the next item. If if is a parent it
  moves to the first child. If not a parent, it moves to the next
  child of it's parent. If it is the last child it moves to the
  parent's brother. Repeatedly calling this will visit every child of
  the browser. 
  \return This returns the widget. If the current widget is the
  last one this returns null, but the current widget remains on the
  last one.

  The current_position() is NOT set by this! It cannot be calculated
  efficiently and would slow down the use of this function for
  visiting all items.
*/
Widget* Browser::next() {
  int n = children(HERE.indexes, HERE.level+1);
  if (n > 0) {
    set_level(HERE.level+1);
    HERE.indexes[HERE.level] = 0;
    siblings = n;
  } else {
    HERE.indexes[HERE.level]++;
  }
  for (;;) {
    if (HERE.indexes[HERE.level] < siblings) {
      item(child(HERE.indexes, HERE.level));
      return item();
    }
    if (HERE.level <= 0) {item(0); return 0;}
    HERE.level--;
    HERE.indexes[HERE.level]++;
    siblings = children(HERE.indexes, HERE.level);
  }
}

/*! Set the current item() to the last one whose top is at or before
  \a Y pixels from the top. 
  \param Y The minimum distance from the top
  \return The last Item's Widget whose top is at or before \a Y*/
Widget* Browser::goto_position(int Y) {
  if (Y < 0) Y = 0;
  if (layout_damage() || Y<=yposition_/2 || !goto_mark(FIRST_VISIBLE)) {
    goto_top();
  } else {
    // move backwards until we are before or at the position:
    while (HERE.position > Y) {
      if (!previous_visible()) {goto_top(); break;}
    }
  }
  // move forward to the item:
  if (item()) for (;;) {
    if (HERE.position+item_h() > Y) break;
    if (!next_visible()) {previous_visible(); return 0;}
  }
  return item();
}

/*! \fn void Browser::damage_item()
  Tell the browser to redraw the current item. Do this if you know it
  has changed appearance. This is better than redrawing the entire
  browser because it will blink a lot less.
*/

static bool nodamage;
/*! Set item referenced by this mark as being damaged. 
  \param mark The damaged item being referenced through its Mark
*/
void Browser::damage_item(const Mark& mark) {
  if (nodamage) return;
  if (!mark.is_set()) return;
  unsigned i;
  for (i = 0; i < NUM_REDRAW; i++)
    if (!mark.compare(REDRAW[i])) return;
  for (i = 0; i < NUM_REDRAW; i++)
    if (!REDRAW[i].is_set()) {
      REDRAW[i] = mark;
      redraw(DAMAGE_VALUE);
      return;
    }
  // if both marks are used we give up and damage the whole thing:
  redraw(DAMAGE_CONTENTS);
}

/*! \fn int Browser::current_level() const
  \return The nesting level of the current item (how many parents it has).  */

/*! \fn const int* Browser::current_index() const
 \return An array of current_level()+1 indexes saying which child at
 each level including the current item.
*/

/*! \fn int Browser::current_position() const
  \return The y position, in pixels, of the top edge of the current
  item.
  
  You may also want the height, which is in item_h().
*/

/*! \fn int Browser::focus_level() const
  \return The nesting level of the focus (how many parents it has).
  The focus is the selected item the user sees.
*/

/*! \fn const int* Browser::focus_index() const
 \return An array of focus_level()+1 indexes saying which child at
 each level includes the focus.
*/

/*! \fn int Browser::focus_position() const
  \return The y position, in pixels, of the top edge of the focus
  item.
  
  You may also want the height, which is in 
  \code goto_focus(); item_h() \endcode.
*/

/*! \fn void Browser::value(int v)
  Same as \code goto_index(v);set_focus();\endcode, to change 
  the current item in a non-hierarchial browser.
  \param v The item to change to
*/

/*! \fn int Browser::value() const
  \return focus_index(v)[0], to get the current item in a
  non-hierarchial browser.
*/

////////////////////////////////////////////////////////////////
// Drawing

// Glyphs that are drawn to the left of items.  Enumeration order
// is important!
enum {
  NONE, BAR, ELL, TEE, CLOSED_ELL, CLOSED_TEE, OPEN_ELL, OPEN_TEE
};

#define BOXSIZE 9

class BrowserGlyph : public Symbol {
public:
  void _draw(const Rectangle& r) const
{
  int lx = r.center_x();
  int ly = r.y()+(r.h()-1)/2; // slightly higher than center_y() for some reason
  int glyph = drawflags()&15;

  switch (glyph) {
  case NONE:
    break;
  case BAR:
    drawline(lx, r.y(), lx, r.b()-1);
    break;
  case ELL:
    drawline(lx, r.y(), lx, ly);
    drawline(lx, ly, r.r(), ly);
    break;
  case TEE:
    drawline(lx, r.y(), lx, r.b()-1);
    drawline(lx, ly, r.r(), ly);
    break;
#if TRIANGLE_GLYPH
  case CLOSED_TEE:
  case OPEN_TEE:
    drawline(lx, r.y(), lx, r.b()-1);
    goto J1;
  case CLOSED_ELL:
  case OPEN_ELL:
    drawline(lx, r.y(), lx, ly);
  J1:
    setdrawflags(drawflags()&15 | (glyph<OPEN_ELL? ALIGN_RIGHT: ALIGN_BOTTOM));
    Widget::default_glyph.draw(r);
    break;
#else
  default: {
    const int boxsize = BOXSIZE/2;
    drawline(lx, r.y(), lx, ly-boxsize);
    if (glyph&1) drawline(lx, ly+boxsize, lx, r.b()-1);
    drawline(lx+boxsize, ly, r.r(), ly);
    strokerect(lx-boxsize, ly-boxsize, BOXSIZE, BOXSIZE);
    drawline(lx-boxsize+2, ly, lx+boxsize-2, ly);
    if (glyph < OPEN_ELL) drawline(lx, ly-boxsize+2, lx, ly+boxsize-2);
    }
#endif
  }
}
  BrowserGlyph() : Symbol("browser") {}
};
static BrowserGlyph glyph;

// this is non-zero if a drag was started in a group open/close box:
static char openclose_drag;

/** Draws the current item
  \param damage If \a damage is non-zero it draws the left-hand glyphs 
  and OR's that damage into the item.
*/
void Browser::draw_item(int damage) {

  int x = interior.x()-xposition_;
  int y = interior.y()+HERE.position-yposition_;
  int arrow_size = int(textsize())|1;
  int inset = (HERE.level+indented())*arrow_size;

  // draw the open/close glyphs at the left:
	if (damage && inset > xposition_) {
		drawstyle(style(), 0);
		Color fg = getcolor();
		setcolor(getbgcolor());
		fillrect(0, y, inset-xposition_+arrow_size, item_h());
		setcolor(fg);
		bool preview_open = openclose_drag == 1 && pushed() && at_mark(FOCUS);
		for (unsigned j = indented() ? 0 : 1; j <= HERE.level; j++) {
			int g = (HERE.indexes[j] < children(HERE.indexes,j) - 1) ? 1 : 0;
			if (j == HERE.level) {
	if (children(HERE.indexes,j+1)>=0)
		if (item_is_open() != preview_open)
			g += OPEN_ELL;
		else
			g += CLOSED_ELL;
	else
		g += ELL;
			}
			// if (getcolor()==BLACK) setcolor(GRAY33);
			if (displaylines_) draw_glyph(g, Rectangle(x, y, arrow_size, item_h()));
			x += arrow_size;
		}
	} else {
	x += inset;
	}
  // Shift first column width, so labels after 1. column are lined up correctly.
  int saved_colw = 0;
  int *cols = (int *)column_widths_p;
  if (cols) {
    saved_colw = cols[0];
    cols[0] -= inset;
    if (cols[0]==0) cols[0]--;
  }

  push_matrix();
  item()->x(x);
  item()->y(y+(int(leading())-1)/2);
  item()->w(interior.w()+xposition_-inset);
  translate(x, y);
  if (at_mark(FOCUS)) {
    if (flag(FOCUSED)) item()->set_flag(FOCUSED);
    else item()->clear_flag(FOCUSED);
    if (!multi()) item()->set_flag(SELECTED);
  } else {
    item()->clear_flag(FOCUSED);
    if (!multi()) item()->clear_flag(SELECTED);
  }
  if (/*flag(HIGHLIGHT) &&*/ at_mark(BELOWMOUSE))
    item()->set_flag(HIGHLIGHT);
  else
    item()->clear_flag(HIGHLIGHT);
  if (damage) item()->set_damage(DAMAGE_ALL|DAMAGE_EXPOSE);
  if (!item()->image()) {
    item()->image(item_is_parent() ? group_symbol_ : leaf_symbol_);
    item()->draw();
    item()->image(0);
  } else {
    item()->draw();
  }
  item()->set_damage(0);
  pop_matrix();

  // Restore column width
  if (cols) cols[0] = saved_colw;

}

/** The callback that forces the Browser to redraw the clip
    \param v The Browser that called this function
    \param r The Rectangle for the clip region
*/
void Browser::draw_clip_cb(void* v, const Rectangle& r) {
  ((Browser*)v)->draw_clip(r);
}

/** Draws the Browser's clip region
  \param r The Rectangle representing the clip region
*/
void Browser::draw_clip(const Rectangle& r) {
  push_clip(r);

  int draw_all = damage() & (DAMAGE_ALL|DAMAGE_CONTENTS);
  if (goto_mark(FIRST_VISIBLE)) for (;;) {
    int item_y = interior.y()+HERE.position-yposition_;
    if (item_y >= r.b()) break;
    if (draw_all) {
      draw_item(DAMAGE_ALL);
    } else {
      // draw all items that are not going to be drawn later:
      for (unsigned i = 0; ; i++) {
        if (i >= NUM_REDRAW) {draw_item(DAMAGE_ALL); break;}
        if (at_mark(REDRAW[i])) break;
      }
    }
    if (!next_visible()) break;
  }

  // erase the area below the last item:
  int bottom_y = interior.y()+HERE.position-yposition_;
  if (bottom_y < r.b()) {
    setcolor(color());
    fillrect(r.x(), bottom_y, r.w(), r.b()-bottom_y);
  }
  pop_clip();
}

void Browser::draw() {
  const int *last_columns = fltk::column_widths();
  fltk::column_widths(column_widths_p);
  uchar d = damage();
  Item::set_style(this,false);
  if (d & DAMAGE_ALL) { // full redraw
    //printf("full redraw damage %x\n", d);
    draw_frame();
    draw_clip(interior);
  } else if (d & DAMAGE_CONTENTS) { // redraw contents
    //printf("contents redraw damage %x\n", d);
    draw_clip(interior);
  } else { // minimal update
    //printf("minimal redraw damage %x\n", d);
    if (scrolldx || scrolldy) {
      scrollrect(interior, scrolldx, scrolldy, draw_clip_cb, this);
    }
    bool clipped = false;
    for (unsigned n = 0; n < NUM_REDRAW; n++) {
      if (goto_mark(REDRAW[n])) {
	if (!clipped) {push_clip(interior); clipped = true;}
	draw_item(DAMAGE_ALL);
      }
    }
    if (d & DAMAGE_CHILD) {
      if (goto_mark(FIRST_VISIBLE)) for (;;) {
	if (HERE.position-yposition_ > interior.h()) break;
	if (item()->damage()) {
	  if (!clipped) {push_clip(interior); clipped = true;}
	  draw_item(0);
	}
	if (!next_visible()) break;
      }
    }
    if (clipped) pop_clip();
  }
  fltk::column_widths(last_columns);
  //fltk::column_widths(0);
  scrolldx = scrolldy = 0;
  for (unsigned n = 0; n < NUM_REDRAW; n++)
    REDRAW[n].unset();

  // draw the scrollbars:
  if (d & DAMAGE_ALL) {
    scrollbar.set_damage(DAMAGE_ALL);
    hscrollbar.set_damage(DAMAGE_ALL);
    if (scrollbar.visible() && hscrollbar.visible()) {
      // fill in the little box in the corner
      setcolor(buttoncolor());
      fillrect(scrollbar.x(), hscrollbar.y(), scrollbar.w(),hscrollbar.h());
    }
    if (header_) {
      for (int i=0; i<nHeader; i++) {
	header_[i]->set_damage(DAMAGE_ALL);
      }
    }
  }
  update_child(scrollbar);
  update_child(hscrollbar);

  if (header_) {
    int h = header_[0]->h();
    Rectangle r(interior.x(), interior.y()-h, interior.w(), h);
    push_clip(r);
    for (int i=0; i<nHeader; i++) {
      update_child(*header_[i]);
      r.set_x(header_[i]->r());
    }
    // Update box in upper-right corner, if necessary
    if (r.w()>0) {
      setcolor(buttoncolor());
      fillrect(r);
    }
    pop_clip();
  }
  Item::clear_style();
}

////////////////////////////////////////////////////////////////
// Scrolling and layout:

/*! Opens the current item (which must be a parent)
  \param open If the current item is a parent, set the open state (the
  fltk::STATE flags) to the given value and redraw the browser
  correctly. 
  \returns true if the state was actually changed, or false if 
  it was already in that state.  */
bool Browser::set_item_opened(bool open)
{
  if (!item() || item_is_open()==open || !item_is_parent()) return false;
  if (open) {
    item()->set_flag(fltk::OPENED);
    set_mark(OPEN);
  } else {
    item()->clear_flag(fltk::OPENED);
    if (item_is_open()) {
      if (HERE.level) OPEN.level = HERE.level-1;
      else OPEN.unset();
    }
  }
  list()->flags_changed(this, item());
  relayout(LAYOUT_CHILD);
  return true;
}

/*! Turns off or on the fltk::INVISIBLE flag on the given item and
  redraw the browser if necessary. 
  \param value The new value of the flag on the given item 
  \return true if the state was actually changed or false if it 
  was already in that state.  */
bool Browser::set_item_visible(bool value)
{
  if (!item()) return false;
  if (value) {
    if (item()->visible()) return false;
    item()->clear_flag(INVISIBLE);
  } else {
    if (!item()->visible()) return false;
    item()->set_flag(INVISIBLE);
  }
  list()->flags_changed(this, item());
  if (HERE.open_level >= HERE.level) relayout(LAYOUT_CHILD);
  return true;
}

void Browser::layout() {
  // This flag is used by relayout() to indicate that autoscroll is needed:
  bool scroll_to_item = (layout_damage()&LAYOUT_CHILD) != 0;

  // clear the flags first so the other methods know it is ok to measure
  // the widgets:
  Widget::layout();

  Item::set_style(this,false);
  const int *last_columns = fltk::column_widths();
  fltk::column_widths(column_widths_p);

  // figure out the visible area:
  const int sw = scrollbar_width();
  interior.set(0,0,w(),h());
  box()->inset(interior);
  if (scrollbar.visible()) interior.move_r(-sw);
  if (hscrollbar.visible()) interior.move_b(-sw);

  int headerh = 0;
  if (header_) {
    for (int i=0; i<nHeader; ++i) {
      if (headerh < header_[i]->h())
        headerh = header_[i]->h();
    }
    interior.move_y(headerh);
  }

  // Measure the height of all items and find widest one, also
  // find vertical position of focus & first visible.
  width_ = 0;
  int arrow_size = int(textsize())|1;
  bool saw_first_visible = false;
  for (goto_top(); item(); next_visible()) {
    int border = arrow_size*HERE.level;
    item()->x(interior.x()+border);
    item()->w(interior.w()-border);
    item()->layout_damage(LAYOUT_X|LAYOUT_W);
    item()->layout();
    //if (!indented_ && item_is_parent()) indented_ = true;
    int w = item()->w()+border;
    if (w > width_) width_ = w;
    if (at_mark(FOCUS)) set_mark(FOCUS);
    if (!saw_first_visible && HERE.position+item()->h() > yposition_) {
      saw_first_visible = true;
      set_mark(FIRST_VISIBLE);
    }
  }
  if (!saw_first_visible) set_mark(FIRST_VISIBLE);
  if (indented()) width_ += arrow_size;
  height_ = HERE.position;

  // Do we have flexible column?
  bool has_flex = false;

  if (header_) {
    // calculate the combined column width and detect flexible column
    int col_width = 0, i;
    for (i=0; i<nHeader; i++) {
      int itemwidth = (i<nColumn)?column_widths_i[i]:0;
      if (itemwidth==0) itemwidth = -1;
      if (itemwidth<0)
	has_flex = true;
      else
	col_width += itemwidth;
    }
    //if (col_width > width_)
    // Always set to headers width
    width_ = col_width;
  }

  // Act as width=0, if we have flexible column so hcrollbar is invisible always
  if (has_flex) width_ = 0;

  // turn the scrollbars on and off as necessary:
  for (int z = 0; z<2; z++) {
    if (height_ > interior.h() || yposition_) {
      if (!scrollbar.visible()) {
        scrollbar.set_visible();
        interior.move_r(-sw);
        redraw(DAMAGE_ALL);
      }
      width_ -= scrollbar_width();
    } else {
      if (scrollbar.visible()) {
        scrollbar.clear_visible();
        interior.move_r(sw);
        redraw(DAMAGE_ALL);
      }
    }
    if (width_ > interior.w() || xposition_) {
      if (!hscrollbar.visible()) {
        hscrollbar.set_visible();
        interior.move_b(-sw);
        redraw(DAMAGE_ALL);
      }
    } else {
      if (hscrollbar.visible()) {
        hscrollbar.clear_visible();
        interior.move_b(sw);
        redraw(DAMAGE_ALL);
      }
    }
  }

  // If we have flexible column, set width to W
  if (has_flex) width_ = interior.w();

  if (scrollbar.visible() && scrollbar_align()&ALIGN_LEFT) interior.move(sw,0);
  if (hscrollbar.visible() && scrollbar_align()&ALIGN_TOP) interior.move(0,sw);

  scrollbar.resize(scrollbar_align()&ALIGN_LEFT ?
		   interior.x()-sw : interior.r(),
		   interior.y(), sw, interior.h());
  scrollbar.value(yposition_, interior.h(), 0, height_);
  scrollbar.linesize(textsize()+leading());

  hscrollbar.resize(interior.x(),
		    scrollbar_align()&ALIGN_TOP ?
		    interior.y()-sw : interior.b(),
		    interior.w(), sw);
  hscrollbar.value(xposition_, interior.w(), 0, width_);
  hscrollbar.linesize(scrollbar.linesize());

  if (header_) {
    // first, calculate the combined column width
    int width = 0, nflex = 0, i;
    for (i=0; i<nHeader; i++) {
      int itemwidth = (i<nColumn)?column_widths_i[i]:0;
      if (itemwidth==0) itemwidth = -1;
      if (itemwidth<0)
	nflex -= itemwidth;
      else
	width += itemwidth;
    }
    int space = interior.w()-width; // number of pixels that will fill the flex columns
    int hx = interior.x();          // current x position for this column
    // now set the actual column widths
    for (i=0; i<nHeader; i++) {
      Widget *hi = header_[i];
      int itemwidth = (i<nColumn)?column_widths_i[i]:0;
      if (itemwidth==0) itemwidth = -1;
      if (itemwidth<0)
	itemwidth = -itemwidth*space/nflex;
      int ww = itemwidth;
      if (ww<0) ww = 0;
      if (column_widths_p)
	column_widths_p[i] = itemwidth;
      hi->resize(-xposition_+hx, interior.y()-headerh, ww, headerh);
      hi->layout();
      hx += itemwidth;
    }
  }

  layout_damage(0); // resize of scrollbars may have turned this on

  // Now that we got the sizes of everything, scroll to show current item:
  if (scroll_to_item) {
    goto_mark(FOCUS);
    if (!item()) yposition(0);
    else make_item_visible(NOSCROLL);
  }

  redraw(DAMAGE_CONTENTS); // assumme we need to redraw
  fltk::column_widths(last_columns);
  Item::clear_style();
}

/** The callback to move horizontal scrollbar
  \param o The horizontal Scrollbar
*/
void Browser::hscrollbar_cb(Widget* o, void*) {
  ((Browser*)(o->parent()))->xposition(int(((Scrollbar*)o)->value()));
}

/*! Set the horizontal scrolling position, measured in pixels. Zero is
  the normal position where the left edge of the child widgets is visible. 
  \param X The new scrolling position
*/
void Browser::xposition(int X) {
  int dx = xposition_-X;
  if (dx) {
    xposition_ = X;
    scrolldx += dx;
    if (nHeader) relayout();
    redraw(DAMAGE_VALUE);
  }
}

/** The vertical Scrollbar callback
  \param o The vertical Scrollbar
*/
void Browser::scrollbar_cb(Widget* o, void*) {
  ((Browser*)(o->parent()))->yposition(int(((Scrollbar*)o)->value()));
}

/*! Set the vertical scrolling position, measured in pixels. Zero means
  the top of the first item is visible. Positive numbers scroll the
  display up.
  \param Y The new vertical Scrollbar position
*/
void Browser::yposition(int Y) {
  if (Y == yposition_) return;
  ((Slider*)(&scrollbar))->value(Y);
  goto_position(Y);
  set_mark(FIRST_VISIBLE);
  scrolldy += (yposition_-Y); redraw(DAMAGE_VALUE);
  yposition_ = Y;
}

////////////////////////////////////////////////////////////////
// Event handling

/*! Change the focus (the selected item, or in an fltk::MultiBrowser
  the item that has a dotted box around it, to the current item.
  This calls make_item_visible().
  \return If the focus is changed, returns true, otherwise returns false */
bool Browser::set_focus() {
  bool ret = !at_mark(FOCUS);
  if (ret) {
    damage_item(HERE); // so will draw focus box around item
    damage_item(FOCUS); // so focus box around old focus item will be removed
    set_mark(FOCUS); // current item is new focus item
  }
  // make the item do it's own focus highlighting:
  if (item() && item()->take_focus());
  // otherwise take the focus from any other child:
  else if (contains(fltk::focus())) fltk::focus(this);
  // scroll to show the item:
  make_item_visible();
  return ret;
}

/*! This makes the current item visible to the user.

  First it turns off the fltk::INVISIBLE flag on the current item, and
  turns off the fltk::INVISIBLE flag and opens (turning on the
  fltk::STATE flag) all parent items. These flag changes cause
  flags_changed() to be called on any fltk::List that you have
  assigned to the browser.

  The browser is then scrolled by calling yposition() so the item is
  visible. 
  \param where The optional argument tells how to scroll. If not specified
  (or the default value of fltk::Browser::NOSCROLL is given) then the
  browser is scrolled as little as possible to show the item. If it is
  fltk::Browser::TOP then the item is put at the top of the
  browser. If it is fltk::Browser::MIDDLE then the item is centered
  vertically in the browser. If it is fltk::Browser::BOTTOM then the
  item is put at the bottom of the browser.

  This does nothing if the current item is null.
  \return false if the current item is null or the item was already visible\n
          true otherwise

*/
bool Browser::make_item_visible(linepos where) {
  if (!item()) return false;
  bool changed = set_item_visible(true);
  // make any parents open and visible:
  if (HERE.open_level < HERE.level) {
    for (unsigned n = HERE.open_level; n < HERE.level; n++) {
      if (HERE.indexes[n] < 0) break;
      int children = this->children(HERE.indexes, n);
      if (HERE.indexes[n] >= children) break;
      Widget* i = child(HERE.indexes, n);
      i->set_visible();
      i->set_flag(fltk::OPENED);
      list()->flags_changed(this, item());
    }
    changed = true;
    relayout(LAYOUT_CHILD);
  } else if (!layout_damage()) {
    Mark TEMP(HERE);
    int h = item_h();
    int p = HERE.position;
    switch (where) {
    case 0:
      if (p < yposition_) break; // act like TOP
      if (p+h-yposition_ <= interior.h()) return changed;
      // fall through:
    case BOTTOM:
      p += h-interior.h(); break;
    case MIDDLE:
      p += h-interior.h()/2; break;
    case TOP:
      break;
    }
    // clip scrolling to the useful range:
    if (p > height_-interior.h()) p = height_-interior.h();
    if (p < 0) p = 0;
    // go there:
    yposition(p);
    goto_mark(TEMP);
  } else {
    // turn on flag indicating that scrolling to item is wanted:
    relayout(LAYOUT_CHILD);
  }
  return changed;
}

/*! This is for use by the MultiBrowser subclass. \n
  This method changes item position in the tree.
  \param e Used to select or deselect an Widget, 
  \param v true selects, false deselects
  \param do_callback optionally execute a callback 
  (calls set_item_selected() - see this for further documentation).
  \return Returns set_item_selected(); see this function for more information.
*/
bool Browser::select(Widget* e, int v, int do_callback) {
  if (!item()) goto_top();
    Widget* i = item();
    Widget *c;
    while  (item()!=e) {
      c=next_visible();
      if (c==i || (!c && (c=goto_top())==i)) return false;
    }
  return set_item_selected(v ? true: false, do_callback);
}
/*! This is for use by the MultiBrowser subclass.
  Turn the fltk::SELECTED flag on or off in the current item (use
  goto_index() to set the current item before calling this).

  \param value This is the toggle switch; if true this item is SELECTED 
  otherwise it's deselected
  \param do_callback A logical OR of when this callback should be executed,
  which is internally logical AND'd with when(). If when() & \a do_callback
  returns 0 but do_callback is positive, this does set_changed()

  If this is not a MultiBrowser, this does select_only_this()
  if \a value is true, and deselect() if \a value is false.

  \return If the Browser is a MultiBrowser, returns false if 
  the item doesn't exist or the item is already selected and value 
  is true (or the item is not selected and value is false)
  or true otherwise\n
  If the item is a regular Browser, returns set_only_this() or 
  deselect(do_callback) based on \a value
*/

bool Browser::set_item_selected(bool value, int do_callback) {
  if(!item()) return false;

  if (multi()) {
    if (value) {
      if (item()->selected()) return false;
      item()->set_selected();
    } else {
      if (!item()->selected()) return false;
      item()->clear_selected();
    }
    list()->flags_changed(this, item());
    damage_item(HERE);
    // allow callbacks to search through items to see what is on/off
    // by remembering the current location and then restoring it
    // afterwards. This means the callback cannot destroy the widget!
    if (when() & do_callback) {
      clear_changed();
      Mark TEMP(HERE);
      this->do_callback();
      goto_mark(TEMP);
    } else if (do_callback) {
      set_changed();
    }
    return true;
  } else {
    if (value) return (select_only_this(do_callback));
    else return deselect(do_callback);
  }
}

/*! Turn off current selection of all items in the browser. For 
  the regular Browser, this puts it in a special state where nothing
  is highlighted and index(0) returns -1. The user cannot get it into
  this state with the GUI.

  For a MultiBrowser the user can get this state by ctrl+clicking the
  selected items off.

  \param do_callback If \a do_callback has some bits that are also in 
  when() then the callback is done for each item that changes 
  selected state.
  \return true if there were items to turn off, false otherwise.
  \todo CHECK THIS!
*/
bool Browser::deselect(int do_callback) {
  HERE.unset(); item(0);
  return select_only_this(do_callback);
}

/*! Make an item be the only one selected. For the MultiBrowser subclass
  this will turn off selection of all other items and turn it on
  for this one and also set the focus here. If the selection
  changes and when() & do_callback is non-zero, the callback is done.

  For the MultiBrowser, the callback is done for each item that changes,
  whether it turns on or off.

  \param do_callback Logical OR of WHEN_* conditions to trigger the callback
  \return For a Browser, returns false if the focus couldn't be changed\n
          returns true otherwise.\n
	  For a MultiBrowser, returns false if the item is not already
	  selected\n returns true otherwise
*/
bool Browser::select_only_this(int do_callback) {
  if (multi()) {
    set_focus();
    bool ret = false;
    // Turn off all other items and set damage:
    if (goto_top()) do {
      if (!at_mark(FOCUS))
        if (set_item_selected(false,do_callback)) ret = true;
    } while (next_visible());
    // Turn off closed items:
    nodamage = true;
    if (goto_top()) do {
      if (!at_mark(FOCUS))
        if (set_item_selected(false,do_callback)) ret = true;
    } while (next());
    nodamage = false;
    goto_mark(FOCUS);
    if (set_item_selected(true,do_callback)) ret = true;
    return ret;
  } else {
    if (!set_focus()) return false;
    if (when() & do_callback) {
      clear_changed();
      this->do_callback();
    } else if (do_callback) {
      set_changed();
    }
    return true;
  }
}

/** Checks if the current item is not the highlighted one and needs 
    highligting, and trigger the old and new highlighted ones to 
    redraw if so
*/
void Browser::set_belowmouse() {
  // update highlighting:
  if (at_mark(BELOWMOUSE)) return;
  damage_item(BELOWMOUSE);
  // needs way to detect if image will redraw due to highlight!
  if (leaf_symbol_ || group_symbol_ || item()->highlight_color()) {
    set_mark(BELOWMOUSE);
    damage_item(BELOWMOUSE);
  } else {
    BELOWMOUSE.unset();
  }
}

/** Indicates that nothing should be highlighted */
void Browser::clear_belowmouse() {
  damage_item(BELOWMOUSE);
  BELOWMOUSE.unset();
}

int Browser::handle(int event) {
  static bool drag_type; // for multibrowser

  switch (event) {

  case fltk::FOCUS:
    if (goto_mark(FOCUS) && item()) item()->take_focus();

  case UNFOCUS:
    damage_item(FOCUS);
    return 1;

  case LEAVE:
    clear_belowmouse();
    break;

  case PUSH:
  case ENTER:
  case MOVE: {
    // For all mouse events check to see if we are in the scrollbar
    // areas and send to them:
    if (scrollbar.visible() && event_inside(scrollbar)) {
      clear_belowmouse();
      return scrollbar.send(event);
    }
    if (hscrollbar.visible() && event_inside(hscrollbar)) {
      clear_belowmouse();
      return hscrollbar.send(event);
    }
    if (header_ && nHeader && event_y() <= interior.y()) {
      clear_belowmouse();
      for (int i=0; i<nHeader; i++) {
        Widget *hi = header_[i];
        if (event_x()>=hi->x() && event_x()<hi->x()+hi->w())
          return hi->send(event);
      }
      return 0; // ignore other clicks on header area
    }

    openclose_drag = false;
    drag_type = true;

    // find the item we are pointing at
    if (!goto_position(event_y()-interior.y()+yposition_)) {
      // click below the bottom item must be handled specially
      clear_belowmouse();
      event_clicks(0); // make it not be a double-click for callback
      if (event != PUSH) return 1;
      if (multi() && event_state(CTRL|SHIFT)) {
        if (event_state(SHIFT)) goto DRAG;
        FOCUS.unset();
      } else {
	deselect(WHEN_CHANGED);
      }
      return 1;
    }

    // set x to left edge of item:
    int arrow_size = int(textsize())|1;
    int x = interior.x()+(HERE.level+indented())*arrow_size-xposition_;
    // see if they are inside the widget and it takes the event:
    if (event_x() >= x) {
      if (item()->send(event)) return 1;
      set_belowmouse();
    } else {
      clear_belowmouse();
    }

    // accept enter/move events so the browser's tooltip appears:
    if (event != PUSH) return 1;

    // drag must start & end in open/close box for it to work:
    if (event_x()<x && event_x()>=x-arrow_size && item_is_parent()) {
      openclose_drag = true;
      set_focus();
      damage_item(HERE);
      return 1;
    }

    //take_focus();
    if (multi()) {
      if (event_state(CTRL)) {
	// toggle this item and set any items we drag over the same:
	if (item()->selected()) drag_type = false;
	event_clicks(0); // make it not be a double-click for callback
        if (event_state(SHIFT)) goto DRAG;
	set_focus();
	set_item_selected(drag_type, WHEN_CHANGED);
	return 1;
      } else if (event_state(SHIFT)) {
        // select between focus and here:
	event_clicks(0); // make it not be a double-click for callback
        goto DRAG;
      }
    }
    select_only_this(WHEN_CHANGED);
    return 1;}

  case DRAG: {
  DRAG:
    // find the item they are now pointing at:
    Widget * hit = goto_position(event_y()-interior.y()+yposition_);
    if (hit) set_belowmouse(); else clear_belowmouse();
    if (openclose_drag && hit && item()) {
      set_focus();
      // set x to left edge of item:
      int arrow_size = int(textsize())|1;
      int x = interior.x()+(HERE.level+indented())*arrow_size-xposition_;
      if (event_x()<x && event_x()>=x-arrow_size && item_is_parent()) {
	if (openclose_drag != 1) {openclose_drag = 1; damage_item(HERE);}
      } else {
	if (openclose_drag == 1) {openclose_drag = 2; damage_item(HERE);}
      }
      return 1;
    }
    if (multi()) {
      if (hit) {
        int direction = FOCUS.compare(HERE);
        if (direction) {
          Mark TEMP(HERE);
          if (goto_mark(FOCUS)) {
            for (;;) {
              set_item_selected(drag_type, WHEN_CHANGED);
              if (at_mark(TEMP)) break;
              if (!(direction<0 ? next_visible() : previous_visible())) break;
            }
          }
          goto_mark(TEMP);
          set_focus();
        } else {
          set_focus();
          set_item_selected(drag_type, WHEN_CHANGED);
        }
      } else if (goto_mark(FOCUS)) {
        // drag from inside to below last item
        for (;;) {
          set_item_selected(drag_type, WHEN_CHANGED);
          if (!next_visible()) {previous_visible(); break;}
        }
        set_focus(); // leave focus on last item
      }
    } else {
      if (hit || FOCUS.is_set()) select_only_this(WHEN_CHANGED);
    }
    return 1;}

  case RELEASE:
    goto_mark(FOCUS);
    if (!item()) goto RELEASE;
    if (openclose_drag == 1 || (event_clicks() && item_is_parent())) {
      // toggle the open/close state of this item:
      set_item_opened(!item_is_open());
      event_is_click(0); // make next click not be double
      if (when()&WHEN_CHANGED) do_callback();
      return 1;
    } else if (openclose_drag) return 1;
    if (event_clicks()) {
      // double clicks act like ReturnKey
      //e_keysym = ReturnKey; // fltk1 did this
      clear_changed();
      execute(item());
      return 1;
    }
    goto RELEASE;

  case KEY:
    event_clicks(0); // make program not think it is a double-click
    switch (event_key()) {
    case RightKey:
      if (goto_visible_focus())
	{set_item_opened(true); next_visible();}
      goto AFTER_MOVEMENT_KEY;
    case LeftKey:
      if (goto_visible_focus())
	if (!set_item_opened(false)) {previous_visible(); set_item_opened(false);}
      goto AFTER_MOVEMENT_KEY;
    case UpKey:
      if (goto_visible_focus()) previous_visible();
      goto AFTER_MOVEMENT_KEY;
    case DownKey:
      if (goto_visible_focus()) next_visible();
    AFTER_MOVEMENT_KEY:
      if (!item()) return 1;
      if (multi() && event_state(SHIFT|CTRL)) {
	if (event_state(SHIFT)) set_item_selected(1,WHEN_CHANGED);
	set_focus();
      } else {
	bool did_callback = when()&WHEN_CHANGED;
	select_only_this(WHEN_CHANGED);
	// allow selection of item to destroy the browser:
	if (did_callback) return 1;
      }
      goto RELEASE;
    case SpaceKey:
      if (!multi() || !goto_visible_focus()) break;
      set_item_selected(!item()->selected(), WHEN_CHANGED);
    RELEASE:
      if ((when()&WHEN_RELEASE) &&
	  (changed() || (when()&WHEN_NOT_CHANGED))) {
	clear_changed();
	do_callback();
      }
      return 1;
    case ReturnKey:
    case KeypadEnter:
      if (!(when() & WHEN_ENTER_KEY)) break;
      if (!goto_visible_focus()) break;
      clear_changed();
      execute(item());
      return 1;
    default:
      if (scrollbar.send(event)) return 1;
      if (hscrollbar.send(event)) return 1;
    }
    break;

  case MOUSEWHEEL:
    if (event_key() == WheelLeft || event_key() == WheelRight) return hscrollbar.send(event);
    return scrollbar.send(event);
#if 0
    int n = event_dy() * Style::wheel_scroll_lines;
    goto_mark(FIRST_VISIBLE);
    while (n < 0 && previous_visible()) n++;
    while (n > 0 && next_visible()) n--;
    set_top();
    return 1;
#endif
  }
  return 0;
}

/*! Go to the focus if it is visible and return it.
  If it is not visible, go to the top of the visible region and return
  zero. This is used by keystrokes so the browser does not scroll
  unexpectedly. 
  \return The Widget for the Item that holds the focus, if it's visible.
  Returns NULL otherwise
*/
Widget* Browser::goto_visible_focus() {
  if (FOCUS.position >= yposition_ &&
      FOCUS.position <= yposition_+interior.h()) {
    if (goto_mark(FOCUS)) return item();
  }
  if (goto_mark(FIRST_VISIBLE)) {
    if (HERE.position < yposition_) next_visible();
    if (item()) return 0;
  }
  goto_top();
  return 0;
}

/*! Go to a nested item. This sets the current item() to the given 
    item.
    
     and also returns it. If
    the values are out of range then null is returned.
  
  \param indexes An integer array. indexes[0] must contain the index 
  of the top-level item, indexes[1] must contain the index of the
  child of this top level item relative to the parent, and so on.
  \param level The depth of the node to go to (and the length of indexes,
  + 1)

  A negative number in indexes[0] will make it go into a special
  no-item state where select_only_this() will do deselect().
  \return NULL if the Browser is empty or a negative number was passed
  in index[0] (or the value(s) passed were out of range)\n
  the Widget that was moved to otherwise
*/
Widget* Browser::goto_index(const int* indexes, unsigned level) {
  // negative numbers make nothing be selected:
  if (indexes[0] < 0) {
    HERE.indexes[0] = -1;
    item(0);
    return 0;
  }
  // go to the 0'th item if needed (otherwise go to the focus):
  if (!(indexes[0] || level) || layout_damage() || !goto_mark(FOCUS)) {
    HERE.level = 0;
    HERE.open_level = 0;
    HERE.position = 0;
    HERE.indexes[0] = 0;
    siblings = children(HERE.indexes,0);
    if (siblings <= 0) {item(0); return 0;}// empty browser
    item(child(HERE.indexes,0));
    // quit if this is correct:
    if (!level && !indexes[0]) return item();
  } else {
    // move from the focus backwards until we are before it:
    while (::compare_marks(HERE.indexes,HERE.level,indexes,level)>0)
      if (!previous_visible()) {goto_index(0); break;}
  }
  // move forward until we are after it:
  for (;;) {
    int n = ::compare_marks(HERE.indexes,HERE.level,indexes,level);
    if (!n) return item(); // we found it!
    if (n > 0 || !next_visible()) break;
  }
  // if we get here we passed the item because it was invisible or a
  // parent was closed or invisible, directly set the indexes in this case:
  set_level(level);
  for (unsigned n = 0; n <= level; n++)
    HERE.indexes[n] = indexes[n];
  item(child(HERE.indexes, HERE.level));
  return item();
}

/*! Go to the i'th item in the top level. If i is out of range null is
  returned.
  \param i The item to go to
  \return The Widget the Browser has moved to, or NULL if this is out of range
  */
Widget* Browser::goto_index(int i) {
  return goto_index(&i,0);
}

/*! Go to an item at any level up to 5. Negative numbers indicate that
  no more levels should be looked at. 
  \param a The top level index
  \param b The child of the top level's index
  \param c The child of \a b's index 
  \param d The child of \a c's index
  \param e The child of \a d's index
*/
Widget* Browser::goto_index(int a, int b, int c, int d, int e) {
  int indexes[6];
  int i = 0;
  indexes[0] = a;
  if (b>=0) {
    indexes[++i] = b;
    if (c>=0) {
      indexes[++i] = b;
      if (d>=0) {
	indexes[++i] = d;
	if (e>=0) {
	  indexes[++i] = e;
	}
      }
    }
  }
  return goto_index(indexes,i);
}

/*!  Sets the horizontal locations that each '\\t' character in an item
  should start printing text at. These are measured from the left edge
  of the browser, including any area for the open/close + glyphs.

  \param t A constant integer array, with the 'i'th position representing
  the width of column i
  \li Array must end with 0 (zero) always
  \li You can define flexible column by setting column width to -1.
      If you have flexible column in browser, all columns are resized to
      match width of the browser, by resizing flexible column.

\code
  // Example 1: make three columns, total width of columns is 300 pixels.
  // Columns are resizable, but total width is kept always.
  const int widths[]   = { 100, 100, 100, 0 };

  // Example 2: make three columns, total width of columns is always width of the browser.
  // Columns are resizable, third column is flexible and will take remaining space left.
  const int widths[]   = { 100, 100, -1, 0 };
\endcode
  */
void Browser::column_widths(const int *t) {
  column_widths_ = t;
  int pnc = nColumn;
  nColumn = 0;
  // count the number of columns, end with 0 or <-1 , -1 is used for flexible column
  if (t) while (*t++!=0) nColumn++;
  if (nColumn==0) {
    // free the column memory
    delete[] column_widths_i;
    delete[] column_widths_p;
    column_widths_p = 0;
    column_widths_i = 0;
  } else {
    // reallocate the column storage if needed
    if (nColumn>pnc) {
      delete[] column_widths_i;
      delete[] column_widths_p;
      column_widths_p = new int[nColumn+1];
      column_widths_i = new int[nColumn+1];
    }
    // copy the widths over into the new array
    memcpy(column_widths_p, column_widths_, (nColumn+1)*sizeof(int));
    memcpy(column_widths_i, column_widths_, (nColumn+1)*sizeof(int));
  }
  // now recalculate the layout
  relayout();
  redraw();
}

/** Sets the start position of a column to a certain position
  \param col The index of the column to move
  \param x The x position, in pixels, that this column is to be moved to
  \return -1 if the index is out of bounds or the user is attempting to adjust
  the first column,\n
  the distance in pixels the column had to move, otherwise
*/
int Browser::set_column_start(int col, int x) {
  // we must adjust all this column and the column to the left so that the
  // resulting edge ends at x
  if (col<=0) return -1; // we don't adjust the first column
  if (col>nColumn) return -1; // out of bounds

  bool has_flex = false;
  // find the current column x and calculate the desired delta
  int ox = 0;
  for (int i=0; i<col; i++) {
    ox += column_widths_p[i];
    if (column_widths_i[i]==-1) has_flex = true;
  }
  int dx = x + xposition_ - ox;
  int cwp, cwi;

  if (col==nColumn) {
    // Cant tesize last column, if we have flexible column
    int *p=column_widths_i; while (*p!=0) { if (*p++==-1) return 0; }
    // Resize last column.
    if (column_widths_p[col]==-1) return 0;
    if (column_widths_p[col-1]+dx<4) dx = -column_widths_p[col-1]+4;
    column_widths_i[col-1] += dx;
    goto RETURN;
  }

  // make sure that no column is smaller than 4 pixels (to the left)
  if (column_widths_p[col-1]+dx<4) {
    dx = -column_widths_p[col-1]+4;
  }
  // make sure that no column is smaller than 4 pixels (to the right)
  cwp = column_widths_p[col], cwi = column_widths_i[col];
  if (cwi>0 && cwi<cwp) cwp = cwi;
  if (cwp-dx<4) {
    dx = cwp-4;
  }
  // now adjust the columns in the interactive field
  if (column_widths_i[col-1]>0) column_widths_i[col-1] += dx;
  if (has_flex) { column_widths_i[col] -= dx; }

RETURN:
  // finally recalculate the layout
  relayout();
  redraw();
  return dx;
}

/** The callback that is triggered when the user clicks on a column header
  \param ww The widget that was clicked on
  \param d An integer representing the index of the column that was clicked,
  used to trigger the column's callback
*/
void Browser::column_click_cb_(Widget *ww, void *d) {
  Browser *w = (Browser*)(ww->parent());
  w->selected_column_ = int(long(d));
  w->do_callback();
  w->selected_column_ = NO_COLUMN_SELECTED;
}
#include <fltk/Font.h>
class BButton : public Button {
  uchar sides; // bit 0 set: user can drag left side, bit 1: right side
public:
  BButton(uchar s, const char *l=0)
  : Button(0,0,0,0, l)
  {
    sides = s;
    align(ALIGN_INSIDE|ALIGN_CLIP);
  }

  int handle(int event) {
    static int ox = -1;
    static bool left = true;
    static bool enter_before_leave = false;
    switch (event) {
      case fltk::LEAVE:
        if (!enter_before_leave) cursor(fltk::CURSOR_DEFAULT);
        enter_before_leave = false;
        break;
      case fltk::ENTER:
        enter_before_leave = true;
        // fall through
      case fltk::MOVE:
        if (sides&1 && fltk::event_x()<=2) {
          cursor(fltk::CURSOR_WE);
        } else if (sides&2 && fltk::event_x()>=w()-2) {
          cursor(fltk::CURSOR_WE);
        } else {
          cursor(fltk::CURSOR_DEFAULT);
        }
        if (event==fltk::ENTER) { Button::handle(event); return 1; }
        break;
      case fltk::PUSH:
        if (sides&1 && fltk::event_x()<=2) {
          left = true;
          ox = fltk::event_x_root() - x();
          return 1;
        } else if (sides&2 && fltk::event_x()>=w()-2) {
          left = false;
          ox = fltk::event_x_root() - x() - w();
          return 1;
        }
        break;
      case fltk::DRAG: {
        if (ox==-1) break;
          Browser *w = (Browser*)(parent());
          int col = (int) argument();
          if (left)
            w->set_column_start(col, fltk::event_x_root()-ox);
          else
            w->set_column_start(col+1, fltk::event_x_root()-ox);
          return 1; 
        }
      case fltk::RELEASE:
        if (ox==-1) break;
        ox = -1;
        return 1;
    } // switch (event)
    return Button::handle(event);
  } // handle() method

  void layout() {
    if (h() == 0) {
      int wide, high;
      measure_label(wide, high);
      h(high + leading() + 3);
    }
    Button::layout();
  }
}; // BButton class

/*! Set an array of labels to put at the top of the browser. The initial
  sizes of them are set with column_widths(). Items in the browser can
  print into the correct columns by putting '\\t' characters into
  their text. Or they can look at fltk::column_widths() to find
  the settings from their draw() methods.

  \param t A const char* array of labels
*/
void Browser::column_labels(const char **t) {
  column_labels_ = t;
  int i;
  if (header_) {
    for (i=0; i<nHeader; i++) delete header_[i];
    delete[] header_;
  }
  nHeader = 0; header_ = 0;
  if (t) { // create new header_ widgets
    Group *g = Group::current();
    Group::current(0);
    while (*t++) nHeader++;
    header_ = new Widget*[nHeader];
    for (i=0; i<nHeader; i++) {
      uchar sides = 0;
      if (i>0) sides |= 1;
      if (i<nHeader) sides |= 2;
      header_[i] = new BButton(sides, column_labels_[i]);
      header_[i]->parent(this);
      header_[i]->callback(column_click_cb_, (void*)i);
    }
    Group::current(g);
  }
  layout();
}

/*! \fn int Browser::selected_column()
  It will call the callback() if the user clicks on a column title.
  Check this to see which one they clicked. This will return a
  negative number if the callback is being done for some other
  reason, such as the user clicking on an item.
  \return The index of the selected column, or negative if no column was 
  selected
*/

////////////////////////////////////////////////////////////////
// Fltk 1.0 emulation
// Items are numbered and children of top-level items are ignored.
// In 1.0 the item numbers started at 1, I changed this to zero so
// this matches Menu.

/*! Same as goto_index(line),set_item_selected(value), to change the
  selected state of an item in a non-hierarchial
  MultiBrowser.
  \param line The line of the widget to look at
  \param value Flag representing whether or not to select this item
  \return If \a line is out of range this returns false, else returns true. */
bool Browser::select(int line, bool value) {
  if (!goto_index(line)) return false;
  return set_item_selected(value, false);
}

/*! Does goto_index(line),item_selected() to return the selection
  state of an item in a non-hierarchial MultiBrowser.
  \param line The line to look at
  \return If \a line is out of range it returns false, else returns true
*/
bool Browser::selected(int line) {
  if (!goto_index(line)) return false;
  return item()->selected();
}

/*! Convenience function for non-hierarchial browsers.
  \param line The line this item is on
  \return true if the indexed item is visible,\n false otherwise
*/
bool Browser::displayed(int line) {
  if (!goto_index(line)) return false;
  return item()->visible();
}

/*! Convenience function for non-hierarchial browsers. Make the
  indexed item be visible() and scroll the browser so it can be seen
  by the user. */
bool Browser::display(int line, bool value) {
  if (!goto_index(line)) return false;
  return set_item_visible(value);
}

/*! \fn int Browser::topline() const
  Convenience function for non-hierarchial browsers.
  \return The index of the top-level item that is at the top of the
  scrolling window. */

/*! \fn void Browser::topline(int line)
  Convenience function for non-hierarchial browsers.
  \param line Make the indexed item visible and scroll to put it 
  at the top of the browser. */

/*! \fn void Browser::middleline(int line)
  Convenience function for non-hierarchial browsers.
  \param line Make the indexed item visible and scroll to put it 
  in the middle of the browser if it is not already visible 
  (passes NO_SCROLL to make_item_visible(). */

/*! \fn void Browser::bottomline(int line)
  Convenience function for non-hierarchial browsers.
  \param line Make the indexed item visible and scroll to put it 
  at the bottom of the browser. */

/** Accessor method 
  \return true if lines should be displayed, or false otherwise
*/
bool Browser::display_lines() const {
  return displaylines_;
}

/**
  Accessor (set) method which is used to set the value of the 
  "displaylines_" member. 
   \param display If you set display to false it will mean that 
   you do not want lines of the tree to be displayed.
*/
void Browser::display_lines(bool display) {
  displaylines_ = display;
}

////////////////////////////////////////////////////////////////
// Constructor

#define SLIDER_WIDTH 16

static void revert(Style* s) {
  s->glyph_ = &::glyph;
}
static NamedStyle style("Browser", revert, &Browser::default_style);
/*! This style mostly serves to set the parenting back to
  Widget::default_style to avoid the gray color and larger leading
  set by Menu::default_style. However it also sets it's own glyph()
  function to one that draws the [+] and [-] indicators.
*/
NamedStyle* Browser::default_style = &::style;

static int nogroup(int x) {Group::current(0); return x;}

/*! The constructor makes an empty browser. */
Browser::Browser(int X,int Y,int W,int H,const char* L)
  : Menu(X,Y,W,H,L),
    scrollbar(nogroup(X+W-SLIDER_WIDTH),Y,SLIDER_WIDTH,H-SLIDER_WIDTH),
    hscrollbar(X,Y+H-SLIDER_WIDTH,W-SLIDER_WIDTH,SLIDER_WIDTH) {
  set_click_to_focus();
  style(default_style);
  xposition_ = 0;
  yposition_ = 0;
  scrolldx = scrolldy = 0;
  hscrollbar.parent(this);
  hscrollbar.callback(hscrollbar_cb);
  scrollbar.set_vertical();
  scrollbar.parent(this);
  scrollbar.callback(scrollbar_cb);
  indented_ = 0;
  column_widths_ = 0;
  column_widths_p = 0;
  column_widths_i = 0;
  column_labels_ = 0;
  selected_column_ = -1;
  nColumn = 0;
  nHeader = 0; header_ = 0;
  leaf_symbol_ = 0;
  group_symbol_ = 0;
  displaylines_ = true;
  OPEN.unset();
  Group::current(parent());
}

/*! The destructor deletes all the list items (because they are child
  fltk::Widgets of an fltk::Group) and destroys the browser. */
Browser::~Browser() {
  delete[] column_widths_p;
  delete[] column_widths_i;
  if (header_) {
    for (int i=0; i<nHeader; i++) delete header_[i];
    delete[] header_;
  }
}

////////////////////////////////////////////////////////////////

/*! \class fltk::MultiBrowser

  The fltk::MultiBrowser class is a subclass of fltk::Browser which
  lets the user select any set of the lines. Clicking on an item
  selects only that one. Ctrl+click toggles items on/off. Shift+drag
  (or shift+arrows) will extend selections. Normally the callback is
  done when any item changes it's state, but you can change this with
  when().

  See fltk::Browser for methods to control the display and "current
  item", and fltk::Menu for methods to add and remove lines from the
  browser.

  The methods on fltk::Browser for controlling the "value" control which
  item has the keyboard focus in a multi-browser. You must use the
  "select" methods described here to change what items are turned on:
  - set_item_selected()
  - select_only_this()
  - deselect()
  - select()
  - selected()

*/

/*! \fn Browser::leaf_symbol(const Symbol*);
  Sets a default value for image() on each item that is not a parent
  of other items. If the item has no image() then this one is used
  for it.
  \param s The symbol to use for the image
*/

/*! \fn Browser::group_symbol(const Symbol*);
  Sets a default value for image() on each item that is a hierarchy
  parent. If the parent item has no image() then this one is used
  for it.
  \param s The symbol to use for the image
*/

//
// End of "$Id: Browser.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
