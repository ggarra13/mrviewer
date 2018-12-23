//
// "$Id: EngravedLabel.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Engraved label drawing routines for the Fast Light Tool Kit (FLTK).
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

// Drawing code for XForms style engraved & embossed labels

#include <fltk/Widget.h>
#include <fltk/LabelType.h>
#include <fltk/draw.h>
using namespace fltk;

// data is dx, dy, color triples

void EngravedLabel::draw(const char* label, const Rectangle& r, Flags align) const
{
  Color saved_color = getcolor();
  Flags saved_flags = drawflags();
  setdrawflags(saved_flags|INACTIVE_R);
  Rectangle r1(r);
  for (const int *data = this->data; ; data += 3) {
    r1.x(r.x()+data[0]);
    r1.y(r.y()+data[1]);
    Color fg = (Color)(data[2]);
    if (!fg) break;
    setcolor(fg);
    drawtext(label, r1, align);
  }
  setdrawflags(saved_flags);
  setcolor(saved_color);
  drawtext(label, r1, align);
}

static const int shadow_data[2][3] = {{2,2,GRAY33},{0,0,0}};
static EngravedLabel shadowLabel("shadow", shadow_data);
LabelType* const fltk::SHADOW_LABEL = &shadowLabel;

static const int engraved_data[7][3] = {
  {1,0,GRAY99},{1,1,GRAY99},{0,1,GRAY99},
  {-1,0,GRAY33},{-1,-1,GRAY33},{0,-1,GRAY33},
  {0,0,0}};
static EngravedLabel engravedLabel("engraved", engraved_data);
LabelType* const fltk::ENGRAVED_LABEL = &engravedLabel;

static const int embossed_data[7][3] = {
  {-1,0,GRAY99},{-1,-1,GRAY99},{0,-1,GRAY99},
  {1,0,GRAY33},{1,1,GRAY33},{0,1,GRAY33},
  {0,0,0}};
static EngravedLabel embossedLabel("embossed", embossed_data);
LabelType* const fltk::EMBOSSED_LABEL = &embossedLabel;

//
// End of "$Id: EngravedLabel.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
