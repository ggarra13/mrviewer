// "$Id: TiledImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

/*! \class fltk::TiledImage

  Draws the image supplied to it's constructor repeatedly to cover
  the area provided to draw(). Clips the images as necessary to
  fit them in the rectangle.
*/

#include <fltk/TiledImage.h>
#include <fltk/draw.h>
using namespace fltk;

/*! \fn TiledImage::TiledImage(Symbol*)

  A pointer to the image must be provided to the constructor. If it
  is null then the TiledImage draws nothing.
*/

/*! Returns w and h unchanged, indicating that it can draw any size
  of rectangle, with no preference. */
void TiledImage::_measure(int& w, int& h) const {}

/*! Repeatedly draws the image to fill the area, putting the top-left
  corner at \a x,y. This checks the current clip region and does
  minimal drawing of only the visible portions of the image.
*/
void TiledImage::_draw(const Rectangle& r) const
{
  if (!image_) return;
  int iw = r.w();
  int ih = r.h();
  image_->measure(iw,ih); if (iw <= 0 || ih <= 0) return;
  int cx = 0;
  int cy = 0;
  // Perhaps this should use the Align flags to set cx, cy.

  // Figure out the smallest rectangle enclosing this and the clip region:
  Rectangle cr(r); intersect_with_clip(cr);
  if (cr.empty()) return;
  cx += cr.x()-r.x(); cy += cr.y()-r.y();
  push_clip(cr);

  int temp = -cx % iw;
  cx = (temp>0 ? iw : 0) - temp;
  temp = -cy % ih;
  cy = (temp>0 ? ih : 0) - temp;

  int ccx=cx;
  Rectangle ir(iw,ih);

  while (-cy < cr.h()) {
    ir.y(cr.y()-cy);
    while (-cx < cr.w()) {
      ir.x(cr.x()-cx);
      image_->draw(ir);
      cx -= iw;
    }
    cy -= ih;
    cx = ccx;
  }
  pop_clip();
}

//
// End of "$Id: TiledImage.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
