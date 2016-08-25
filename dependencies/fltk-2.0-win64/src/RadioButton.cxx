//
// "$Id: RadioButton.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Radio button widget for the Fast Light Tool Kit (FLTK).
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

#include <fltk/RadioButton.h>
#include <fltk/draw.h>
#include <fltk/Box.h>
using namespace fltk;

/** \class fltk::RadioButton

  This button turns the value() on when clicked, and turns all other
  RadioButton widgets in the same group off.  It displays a round dot
  to show the user this:

  \image html Fl_Round_Button.gif

  You can control the color of the circle with color() and the color
  of the dot with textcolor(). You can make it draw different colors
  when turned on by setting selection_color() and
  selection_textcolor() on the widget (these are ignored if set in an
  inherited style()).

  If you want you can make any other button act like a RadioButton
  by doing type(Button::RADIO) to it. Be sure to lay out and decorate
  your interface so it is clear to the user that they are radio buttons.
*/

static class RadioButtonGlyph : public Symbol {
public:
  void _draw(const Rectangle& R) const {
    // Use the white rather than the gray color:
    Box* box = drawstyle()->box();
    if (drawflags()&PUSHED) setbgcolor(GRAY50);
    box->draw(R);
    if (drawflags(STATE)) {
      Rectangle r(R);
      box->inset(r);
      // use the selection color only if they directly set it:
      Color c = drawstyle()->selection_color_;
      if (c) {
        if (drawflags()&INACTIVE_R) c = inactive(c);
        setcolor(c);
      }
      r.inset((r.h()+1)/6);
      addchord(r, 0, 360);
      fillpath();
    }
  }
  RadioButtonGlyph() : Symbol("radio") {}
} glyph;

static void revert(Style* s) {
  s->buttonbox_ = NO_BOX;
  s->box_ = ROUND_DOWN_BOX;
  s->glyph_ = &glyph;
}
static NamedStyle style("RadioButton", revert, &RadioButton::default_style);
NamedStyle* RadioButton::default_style = &::style;

RadioButton::RadioButton(int x, int y, int w, int h, const char *l)
  : CheckButton(x, y, w, h, l)
{
  style(default_style);
  type(RADIO);
}
