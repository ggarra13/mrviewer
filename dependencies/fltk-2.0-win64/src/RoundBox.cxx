//
// "$Id: RoundBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Round box drawing routines for the Fast Light Tool Kit (FLTK).
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

// Box drawing code for an obscure box type.
// These box types are in seperate files so they are not linked
// in if not used.

#include <config.h>
#include <fltk/Box.h>
#include <fltk/Style.h>
#include <fltk/draw.h>
#include <string.h>
using namespace fltk;

// Circle with an edge pattern like FrameBox:
class RoundBox : public FrameBox {
public:
  RoundBox(const char* n, const char* s, const FrameBox* d=0) : FrameBox(n, 2,2,4,4, s, d) {}
  void _draw(const Rectangle&) const;
  bool fills_rectangle() const {return false;}
  bool is_frame() const {return false;}
};

enum {UPPER_LEFT, LOWER_RIGHT, CLOSED, FILL};

#if USE_CAIRO || USE_QUARTZ
static void addarc(const Rectangle& r, float start, float end) {
  addarc(r.x(),r.y(),r.w(),r.h(),start,end);
}
#endif

// Draw the oval shape. This is ugly because I need to cut the path
// up into individual arcs so the drawing library uses the server
// arc code.

static void lozenge(int which, int x,int y,int w,int h, Color color)
{
  int d = w <= h ? w : h;
  if (d <= 1) return;
  setcolor(color);
  Rectangle r1(x+w-d, y, d, d);
  Rectangle r2(x, y+h-d, d, d);

#if USE_CAIRO || USE_QUARTZ

  newpath();
  switch (which) {
  case UPPER_LEFT:
    addarc(r1, 45.0f, w<=h ? 180.0f : 90.0f);
    addarc(r2, w<=h ? 180.0f : 90.0f, 225.0f);
    break;
  case LOWER_RIGHT:
    addarc(r2, 225.0f, w<=h ? 360.0f : 270.0f);
    addarc(r1, w<=h ? 360.0f : 270.0f, 360.0f+45.0f);
    break;
  default:
    addarc(r1, w<=h ? 0.0f : -90.0f, w<=h ? 180.0f : 90.0f);
    addarc(r2, w<=h ? 180.0f : 90.0f, w<=h ? 360.0f : 270.0f);
    break;
  }
  which==FILL ? fillpath() : strokepath();

#else
  // Ugly version that uses pie pieces in order to get the better drawing
  // functions.

  if (which >= CLOSED) {
    addpie(r1, w<=h ? 0.0f : -90.0f, w<=h ? 180.0f : 90.0f);
  } else if (which == UPPER_LEFT) {
    addpie(r1, 45.0f, w<=h ? 180.0f : 90.0f);
  } else { // LOWER_RIGHT
    addpie(r1, w<=h ? 360.0f : 270.0f, 360.0f+45.0f);
  }
  which==FILL ? fillpath() : strokepath();

  if (which >= CLOSED) {
    addpie(r2, w<=h ? 180.0f : 90.0f, w<=h ? 360.0f : 270.0f);
  } else if (which == UPPER_LEFT) {
    addpie(r2, w<=h ? 180.0f : 90.0f, 225.0f);
  } else { // LOWER_RIGHT
    addpie(r2, 225.0f, w<=h ? 360.0f : 270.0f);
  }
  which==FILL ? fillpath() : strokepath();

  if (which == FILL) {
    if (w < h)
      fillrect(x, y+d/2, w, h-(d&-2));
    else if (w > h)
      fillrect(x+d/2, y, w-(d&-2), h);
  } else {
    if (w < h) {
      if (which != UPPER_LEFT) drawline(x+w-1, y+d/2, x+w-1, y+h-d/2);
      if (which != LOWER_RIGHT) drawline(x, y+d/2, x, y+h-d/2);
    } else if (w > h) {
      if (which != UPPER_LEFT) drawline(x+d/2, y+h-1, x+w-d/2, y+h-1);
      if (which != LOWER_RIGHT) drawline(x+d/2, y, x+w-d/2, y);
    }
  }
#endif
}

extern void fl_to_inactive(const char* s, char* to);

void RoundBox::_draw(const Rectangle& r) const
{
  if (drawflags(PUSHED|STATE) && down_) {
    down_->draw(r);
    return;
  }
  const char* s = data();
  char buf[26]; if (drawflags(INACTIVE_R) && Style::draw_boxes_inactive_) {
    fl_to_inactive(s, buf); s = buf;}
  const Color fg = getcolor();
  const Color bg = getbgcolor();
  int x = r.x();
  int y = r.y();
  int w = r.w();
  int h = r.h();
  if (!drawflags(INVISIBLE)) {
    // draw the interior, assumming the edges are the same thickness
    // as the normal square box:
    int d = strlen(s)/4;
    if (w > 2*d && h > 2*(d-1))
      lozenge(FILL, x+d, y+d-1, w-2*d, h-2*(d-1), bg);
  }
  const char* t;
  if (*s == '2') {t = s+1; s += 3;} else {t = s+2;}
  while (*s && *t && w > 0 && h > 0) {
    Color c1 = *t + (GRAY00-'A'); t += 4;
    Color c2 = *s + (GRAY00-'A'); s += 4;
    lozenge(UPPER_LEFT,  x+1, y,   w-2, h, *s&&*t ? c1 : bg);
    lozenge(UPPER_LEFT,  x,   y,   w,   h, c1);
    lozenge(LOWER_RIGHT, x+1, y,   w-2, h, *s&&*t ? c2 : bg);
    lozenge(LOWER_RIGHT, x,   y,   w,   h, c2);
    x++; y++; w -= 2; h -= 2;
  }
  setcolor(fg); // restore fg color
}

static RoundBox roundDownBox("round_down", "WWMMPPAA");
/** Inset oval or circle. */
Box* const fltk::ROUND_DOWN_BOX = &roundDownBox;
static RoundBox roundUpBox("round_up", "AAWWMMTT", &roundDownBox);
/** Raised oval or circle. */
Box* const fltk::ROUND_UP_BOX = &roundUpBox;

//
// End of "$Id: RoundBox.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
