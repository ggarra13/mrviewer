//
// "$Id: PlasticBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Copyright 2001-2005 by Michael Sweet.
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

// Aqua-like boxes. This needs some significant rewrite so they are faster.
// I also think they should be put into the themes, but it looks like too
// many fltk 1.1 programs use them.

#include <fltk/Box.h>
#include <fltk/Style.h>
#include <fltk/draw.h>
#include <string.h>
using namespace fltk;

extern void fl_to_inactive(const char* s, char* to);

static inline Color shade_color(uchar gc, Color bc) {
  return lerp(gc+(GRAY00-'A'), bc, 0.25f);
}

class PlasticBox : public FrameBox {
public:
  void _draw(const Rectangle&) const;
  PlasticBox(const char* n, const char* s, const FrameBox* d=0)
    : FrameBox(n, 2,2,4,4, s, d) {}
};

void PlasticBox::_draw(const Rectangle& r) const
{
  if (drawflags(PUSHED|STATE) && down_) {
    down_->draw(r);
    return;
  }
  const char* c = data();
  char buf[26]; if (drawflags(INACTIVE_R) && Style::draw_boxes_inactive_) {
    fl_to_inactive(c, buf); c = buf;}

  const Color bc = getbgcolor();
  const Color fg = getcolor();

  int		i, j;
  int		clen = strlen(c) - 1;
  int		chalf = clen / 2;
  int		cstep = 1;

  const int x = r.x();
  const int y = r.y();
  const int w = r.w();
  const int h = r.h();

  if (h < (w * 2)) {
    // Horizontal shading...
    if (clen >= h) cstep = 2;

    for (i = 0, j = 0; j < chalf; i ++, j += cstep) {
      // Draw the top line and points...
      setcolor(shade_color(c[i], bc));
      fillrect(x+1, y+i, w-1, 1);

      setcolor(shade_color(c[i] - 2, bc));
      fillrect(x, y+i, 1, 1);
      fillrect(x+w-1, y+i+1, 1, 1);

      // Draw the bottom line and points...
      setcolor(shade_color(c[clen - i], bc));
      fillrect(x+1, y+h-1-i, w-1, 1);

      setcolor(shade_color(c[clen - i] - 2, bc));
      fillrect(x, y+h-i-1, 1, 1);
      fillrect(x+w-1, y+h-i-1, 1, 1);
    }

    // Draw the interior and sides...
    i = chalf / cstep;

    setcolor(shade_color(c[chalf], bc));
    fillrect(x + 1, y + i, w - 2, h - 2 * i);

    setcolor(shade_color(c[chalf] - 2, bc));
    fillrect(x, y+i, 1, h-2*i);
    fillrect(x+w-1, y+i, 1, h-2*i);

  } else {
    // Vertical shading...
    if (clen >= w) cstep = 2;

    for (i = 0, j = 0; j < chalf; i ++, j += cstep) {
      // Draw the left line and points...
      setcolor(shade_color(c[i], bc));
      fillrect(x+i, y+1, 1, h-1);

      setcolor(shade_color(c[i] - 2, bc));
      fillrect(x+i, y, 1, 1);
      fillrect(x+i, y+h-1, 1, 1);

      // Draw the right line and points...
      setcolor(shade_color(c[clen - i], bc));
      fillrect(x+w-1-i, y+1, 1, h-1);

      setcolor(shade_color(c[clen - i] - 2, bc));
      fillrect(x+w-1-i, y, 1, 1);
      fillrect(x+w-1-i, y+h-1, 1, 1);
    }

    // Draw the interior, top, and bottom...
    i = chalf / cstep;

    setcolor(shade_color(c[chalf], bc));
    fillrect(x + i, y + 1, w - 2 * i, h - 2);

    setcolor(shade_color(c[chalf - 2], bc));
    fillrect(x+i, y, w-2*i, 1);
    fillrect(x+i, y+h-1, w-2*i, 1);
  }
  setcolor(fg);
}

static PlasticBox plasticDownBox(0, "STUVWWWVT");
/**  Pushed in version of PLASTIC_UP_BOX */
Box* const fltk::PLASTIC_DOWN_BOX = &plasticDownBox;
static PlasticBox plasticUpBox(0, "TXSPPQQRSSTTUVS", &plasticDownBox);
/** Box designed to vaguely resemble a certain fruit-themed operating system.*/
Box* const fltk::PLASTIC_UP_BOX = &plasticUpBox;

// up_frame =  "MNFKKLNO"
// down_frame = "LLRRTTLL"

//
// End of "$Id: PlasticBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
