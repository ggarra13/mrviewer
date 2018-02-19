// "$Id: Choice.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
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

/*! \class fltk::Choice

Subclass of fltk::Menu that provides a button that pops up the menu, and
also displays the text of the most-recently selected menu item.

\image html choice.gif

The appearance is designed to look like an "uneditable ComboBox" in
Windows, but it is somewhat different in that it does not contain a
text editor, also the menu pops up with the current item under the
cursor, which is immensely easier to use once you get used to it. This
is the same UI as the Macintosh and Motif, which called this an
OptionButton.

The user can change the value by popping up the menu by clicking
anywhere in the widget and moving the cursor to a different item, or
by typing up and down arrow keys to cycle amoung the items.  Typing
the fltk::Widget::shortcut() of any of the
items will also change the value to that item.

If you set a shortcut() on this widget itself or put &x in the label,
that shortcut will pop up the menu. The user can then use arrow keys
or the mouse to change the selected item.

When the user changes the value() the callback is done.

If you wish to display text that is different than any of the menu
items, you may instead want an fltk::PopupMenu. It works identically
but instead displays an empty box with the label() inside it, you
can then change the label() as needed.

If you want a "real" ComboBox where the user edits the text, this is
a planned addition to the fltk::Input widget. All text input will have
menus of possible replacements and completions. Not yet implemented,
unfortunately.

*/

#include <fltk/Choice.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Box.h>
#include <fltk/Item.h>
#include <fltk/draw.h>
#include <fltk/run.h>
using namespace fltk;

// The dimensions for the glyph in this and the PopupMenu are exactly
// the same, so that glyphs may be shared between them.

extern bool fl_hide_underscore;

/*! You can change the icon drawn on the right edge by setting glyph()
  to your own function that draws whatever you want.
  Otherwise, draw draws its default glyph on the right of the box
*/
void Choice::draw() {
  if (damage() & DAMAGE_ALL) draw_frame();
  Rectangle r(w(),h()); box()->inset(r);
  int w1 = r.h()*4/4;
  r.move_r(-r.h());
  // draw the little mark at the right:
  if (damage() & (DAMAGE_ALL|DAMAGE_HIGHLIGHT)) {
    drawstyle(style(), (flags() & ~FOCUSED) | OUTPUT);
    Rectangle gr(r.r(), r.y(), w1, r.h());
    draw_glyph(ALIGN_BOTTOM|ALIGN_INSIDE, gr);
  }
  if (damage() & (DAMAGE_ALL|DAMAGE_VALUE)) {
    setcolor(color());
    fillrect(r);
    Widget* o = get_item();
    //if (!o && children()) o = child(0);
    if (o) {
      Item::set_style(this,false);
      Flags saved = o->flags();
      if (focused()) o->set_flag(SELECTED);
      else o->clear_flag(SELECTED);
      if (any_of(INACTIVE|INACTIVE_R)) o->set_flag(INACTIVE_R);
      push_clip(r);
      push_matrix();
      if (!o->h()) o->layout();
      // make it center on only the first line of multi-line item:
      int h = o->h();
      int n = h/int(o->labelsize()+o->leading());
      if (n > 1) h -= int((n-1)*o->labelsize()+(n-1.5)*o->leading());
      // center the item vertically:
      translate(r.x()+2, r.y()+((r.h()-h)>>1));
      int save_w = o->w(); o->w(r.w()-4);
      fl_hide_underscore = true;
      o->draw();
      fl_hide_underscore = false;
      Item::clear_style();
      o->w(save_w);
      o->flags(saved);
      pop_matrix();
      pop_clip();
    }
  }
}

static bool try_item(Choice* choice, int i) {
  Widget* w = choice->child(i);
  if (!w->takesevents()) return false;
  choice->value(i);
  choice->execute(w);
  return true;
}

/** The default handle() passes control to handle(e, rectangle)
    which attempts popup the menu / close the menu based on click area
    \param e The event to handle
    \return 1 if the event was successfully handled,\n 0 otherwise
*/ 
int Choice::handle(int e) {
  return handle(e,Rectangle(w(),h()));
}

/** This function attempts to deal with where the user clicked in an effort
    to pop the menu up in the right place.
    \param e The event to handle
    \param rectangle The Choice's rectangle
    \return 1 if the event was successfully handled.\n 0 otherwise
*/
int Choice::handle(int e, const Rectangle& rectangle) {
  int children = this->children(0,0);
  switch (e) {

  case FOCUS:
  case UNFOCUS:
    redraw(DAMAGE_VALUE);
    return 1;

  case ENTER:
  case LEAVE:
    redraw_highlight();
  case MOVE:
    return 1;

  case PUSH:
    // Normally a mouse click pops up the menu. If you uncomment this line
    // (or make a subclass that does this), a mouse click will re-pick the
    // current item (it will popup the menu and immediately dismiss it).
    // Depending on the size and usage of the menu this may be more
    // user-friendly.
//  event_is_click(0);
    if (click_to_focus()) {
      take_focus();
      fltk::flush(); // this is a temporary fix for Nuke!
      // Nuke is destroying widgets in layout(), not a good idea...
      if (fltk::focus() != this) return 1; // detect if take_focus destroys this
    }
  EXECUTE:
    if (!children) return 1;
    if (popup(rectangle, 0)) redraw(DAMAGE_VALUE);
    return 1;

  case SHORTCUT:
    if (test_shortcut()) goto EXECUTE;
    if (handle_shortcut()) {redraw(DAMAGE_VALUE); return 1;}
    return 0;

  case KEY:
    switch (event_key()) {

    case ReturnKey:
    case SpaceKey:
      goto EXECUTE;

    case UpKey: {
      if (!children) return 1;
      int i = value(); if (i < 0) i = children;
      while (i > 0) {--i; if (try_item(this, i)) return 1;}
      return 1;}
    case DownKey: {
      if (!children) return 1;
      int i = value();
      while (++i < children) if (try_item(this,i)) return 1;
      return 1;}
    }
    return 0;

  default:
    return 0;
  }
}

#define MOTIF_STYLE 0
#define MAC_STYLE 0

#if MOTIF_STYLE
// Glyph erases the area and draws a long, narrow box:
static void glyph(int, const Rectangle& r, const Style* s, Flags)
{
  color(s->buttoncolor());
  fillrect(r);
  // draw a long narrow box:
  Rectangle r1(r, r.w()-2, r.h()/3, ALIGN_LEFT);
  Widget::default_glyph(0, r1, s, 0);
}
#endif

#if MAC_STYLE
// Attempt to draw an up/down arrow like the Mac uses, since the
// popup menu is more like how the Mac works:
static void glyph(int, const Rectangle& r, const Style* s, Flags flags)
{
  Widget::default_glyph(0, r, s, flags);
  Rectangle r1(r);
  r1.h(r1.h()/2);
  r1.move_y(r1.h()/3);
  Widget::default_glyph(GLYPH_UP, r1, s, flags);
  r1.move(0,r1.h());
  Widget::default_glyph(GLYPH_DOWN, r1, s, flags);
}
#endif

static void revert(Style* s) {
#if MOTIF_STYLE
  s->color_ = GRAY75;
  s->box_ = s->buttonbox_ = Widget::default_style->buttonbox_;
  s->glyph_ = ::glyph;
#endif
#if MAC_STYLE
  s->glyph_ = ::glyph;
#endif
}
static NamedStyle style("Choice", revert, &Choice::default_style);
/** Sets the default style to nothing unless either of MOTIF_STYLE or
    MAC_STYLE are defined (within the source file) which they aren't, by 
    default. Inherits from Group::default_style and the 
    "default" style, unless either of the above are defined (in which case
    the glyph will be changed and, for MOTIF_STYLE, the buttonbox_ will be
    changed
*/
NamedStyle* Choice::default_style = &::style;

/*! The constructor makes the menu empty. See Menu and StringList
  for information on how to set the menu to a list of items.
  \param x The x position of the Choice, relative to the Group
  \param y The y position of the Choice, relative to the Group
  \param w The width of the Choice, relative to the Group
  \param h The height of the Choice, relative to the Group
  \param l The label of the Choice
*/
Choice::Choice(int x,int y,int w,int h, const char *l) : Menu(x,y,w,h,l) {
  value(0);
  // copy the leading from Menu:
  if (!default_style->leading_) default_style->leading_ = style()->leading_;
  style(default_style);
  clear_flag(ALIGN_MASK);
  set_flag(ALIGN_LEFT);
  set_click_to_focus();
}

//
// End of "$Id: Choice.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
