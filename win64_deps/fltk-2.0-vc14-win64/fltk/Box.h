//
// "$Id: Box.h 8636 2011-05-06 08:01:12Z bgbnbigben $"
//
// Define your own values for box() on a widget by making one of these.
//
// Copyright 2002 by Bill Spitzak and others.
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

#ifndef fltk_Box_h
#define fltk_Box_h

#include "Color.h"
#include "Flags.h"
#include "Symbol.h"

namespace fltk {

typedef Symbol Box;

class FL_API FrameBox : public Box {
protected:
  const char* data_;
  const Box* down_;
public:
  const char* data() const {return data_;}
  void data(const char* d) {data_ = d;}
  void _draw(const Rectangle&) const;
  void inset(Rectangle&) const;
  bool fills_rectangle() const;
  bool is_frame() const;
  FrameBox(const char* name, int dx,int dy,int dw,int dh, const char* pattern, const Box* down=0)
    : Box(name),data_(pattern),down_(down) {set_inset(dx,dy,-dw,-dh);}
};

class FL_API FlatBox : public Box {
public:
  void _draw(const Rectangle&) const;
  bool fills_rectangle() const;
  bool is_frame() const;
  FlatBox(const char* n);
};

class FL_API HighlightBox : public FlatBox {
  const Box* down_;
public:
  void _draw(const Rectangle&) const;
  bool fills_rectangle() const;
  bool is_frame() const;
  HighlightBox(const char* n, const Box* d);
};

}
#endif

//
// End of "$Id: Box.h 8636 2011-05-06 08:01:12Z bgbnbigben $".
//
