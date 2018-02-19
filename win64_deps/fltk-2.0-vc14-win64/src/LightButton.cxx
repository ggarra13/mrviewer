//
// "$Id: LightButton.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/LightButton.h>
#include <fltk/Box.h>
#include <fltk/draw.h>

using namespace fltk;

/*! \class fltk::LightButton

  This button turns the value() on and off each release of a click
  inside of it, and displays a light to show the user this. This style
  was designed for the original Forms library and has been copied
  into Discreet products and many other pieces of software that
  originated with SGI machines:

  \image html Fl_Light_Button.gif

  When off the light is color(), when on the light is selection_color().
*/

static class LightButtonGlyph : public Symbol {
public:
  void _draw(const Rectangle& r) const {
    Color saved = getbgcolor();
    Flags f = drawflags();
    if (f & STATE) {
      setbgcolor(drawstyle()->selection_color());
    } else if (!drawstyle()->color_) {
      // if user did not set color make the interior invisible
      setdrawflags(INVISIBLE);
    }
    int ww = r.w()/2+1; // we will center box of this width in area
    THIN_DOWN_BOX->draw(Rectangle(r, ww, r.h()));
    drawflags(f);
    setbgcolor(saved);
  }
  LightButtonGlyph() : Symbol("light") {}
} glyph;

static void revert(Style* s) {
  s->selection_color_ = YELLOW;
  s->glyph_ = &glyph;
}
static NamedStyle style("LightButton", revert, &LightButton::default_style);
NamedStyle* LightButton::default_style = &::style;

LightButton::LightButton(int x, int y, int w, int h, const char *l)
  : CheckButton(x, y, w, h, l)
{
  default_style->parent_ = Button::default_style;
  style(default_style);
}

//
// End of "$Id: LightButton.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
