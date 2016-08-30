//
// "$Id: CheckButton.cxx 8752 2011-05-29 08:44:01Z bgbnbigben $"
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

#include <fltk/CheckButton.h>
#include <fltk/Group.h>
#include <fltk/Box.h>
#include <fltk/draw.h>

using namespace fltk;

/*! \class fltk::CheckButton
  This button turns the value() on and off each release of a click
  inside of it, and displays a checkmark to show the user this:

  \image html Fl_Check_Button.gif

  You can control the color of the checkbox with color() and the
  color of the checkmark with textcolor(). You can make it draw
  different colors when turned on by setting selection_color()
  and selection_textcolor() on the widget (these are ignored if
  set in an inherited style()).
*/

static class CheckBox : public Symbol {
public:
  void _draw(const Rectangle& R) const {
    Box* box = drawstyle()->box();
    // for back compatability with some programs that changed the
    // square into a diamond or circle, where the checkmark does
    // not look too good. Draw the box colored in with the
    // selected color instead:
    if (!box->fills_rectangle()) {
      if (drawflags()&STATE) setbgcolor(drawstyle()->selection_color());
      box->draw(R);
      return;
    }
    // Otherwise draw the box with normal colors and then draw checkmark
    // inside it:
    if (drawflags()&PUSHED) setbgcolor(GRAY50);
    box->draw(R);
    if (drawflags()&STATE) {
      Rectangle r(R); box->inset(r);
      if (r.h() < 6) {r = Rectangle(R,6,6); r.move(1,1);}
      int x = r.x()+1;
      int w = r.h()-2;
      int d1 = w/3;
      int d2 = w-d1;
      int y = r.y()+(r.h()+d2)/2-d1-2;
      for (int n = 0; n < 3; n++, y++) {
        drawline(x, y, x+d1, y+d1);
        drawline(x+d1, y+d1, x+w-1, y+d1-d2+1);
      }
    }
  }
  CheckBox() : Symbol("checkbox") {}
} glyph;

/** Handballs drawing onto Button::draw(), with a glyph width of textsize()+2*/
void CheckButton::draw() {
  Button::draw(int(textsize())+2);
}

static void revert(Style* s) {
  s->buttonbox_ = NO_BOX;
  //s->box_ = DOWN_BOX;
  s->glyph_ = &glyph;
}
static NamedStyle style("Check_Button", revert, &CheckButton::default_style);
/** The default style for a CheckButton reverts to NO_BOX and a default glyph
 (the check mark to the left)
*/
NamedStyle* CheckButton::default_style = &::style;

/** The constructor handballs creation to Button(), sets type(TOGGLE) and
    sets ALIGN_LEFT|ALIGN_INSIDE on the label
    \param x The x position of the CheckButton, relative to the group
    \param y The y position of the CheckButton, relative to the group
    \param w The width of the CheckButton
    \param h The height of the CheckButton
    \param l The CheckButton's label
*/
CheckButton::CheckButton(int x, int y, int w, int h, const char *l)
  : Button(x, y, w, h, l)
{
  style(default_style);
  type(TOGGLE);
  set_flag(ALIGN_LEFT|ALIGN_INSIDE);
}
