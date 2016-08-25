//
// "$Id: DiamondBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Diamond box code for the Fast Light Tool Kit (FLTK).
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

// A less-used boxtype, it is here so that it is not linked in if
// not used.

// The diamond box draws best if the area is square!

#include <fltk/Box.h>
#include <fltk/Style.h>
#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <string.h>

using namespace fltk;

// Diamond with an edge pattern like FrameBox:
class DiamondBox : public Box {
public:
  const char* data;
  const DiamondBox* down;
  void _draw(const Rectangle& r) const;
  void inset(Rectangle& r) const {r.inset(5);}
  DiamondBox(const char* n, const char* s, const DiamondBox* d=0)
    : Box(n), data(s), down(d) {}
};

extern void fl_to_inactive(const char* s, char* to);

void DiamondBox::_draw(const Rectangle& r) const
{
  Color saved_color = getcolor();
  int x1 = r.center_x();
  int x = r.x(); int w = r.w(); if (w&1) w--; else {w -= 2; x++;}
  int y1 = r.center_y();
  int y = r.y(); int h = r.h(); if (h&1) h--; else {h -= 2; y++;}
  const char* s = drawflags(PUSHED|STATE) && down ? down->data : data;
  char buf[26]; if (drawflags(INACTIVE_R) && Style::draw_boxes_inactive_) {
    fl_to_inactive(s, buf); s = buf;}
  const char* t;
  if (*s == '2') {t = s+1; s += 3;} else {t = s+2;}
  while (*s && *t && w > 0 && h > 0) {
    // draw upper-right line:
    setcolor(*s++ + (GRAY00-'A'));
    drawline(x+w, y1, x1, y);
    // draw upper-left line:
    setcolor(*s++ + (GRAY00-'A'));
    drawline(x1, y, x, y1);
    s += 2;
    // draw lower-left line:
    setcolor(*t++ + (GRAY00-'A'));
    drawline(x, y1, x1, y+h);
    // draw lower-right line:
    setcolor(*t++ + (GRAY00-'A'));
    drawline(x1, y+h, x+w, y1);
    t += 2;
    x++; y++; w -= 2; h -= 2;
  }
  if (w > 0 && h > 0 && !drawflags(INVISIBLE)) {
    // draw the interior, assumming the edges are the same thickness
    // as the normal square box:
    newpath();
    addvertex(x, y1);
    addvertex(x1, y);
    addvertex(x+w,y1);
    addvertex(x1,y+h);
    setcolor(getbgcolor());
    fillstrokepath(getbgcolor());
  }
  setcolor(saved_color);
}

static DiamondBox diamondDownBox("diamond_down", "2WWMMPPAA");

/** Diamond shape used to draw Motif-style checkboxes. */
Box* const fltk::DIAMOND_DOWN_BOX = &diamondDownBox;
static DiamondBox diamondUpBox("diamond_up", "2AAWWMMTT", &diamondDownBox);

/** Raised diamond shape used to draw Motif-style checkboxes. */
Box* const fltk::DIAMOND_UP_BOX = &diamondUpBox;

//
// End of "$Id: DiamondBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
