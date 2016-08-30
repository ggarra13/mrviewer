//
// "$Id: BarGroup.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// The BarGroup is a closable strip that can be put in the edges of a Pack,
// usually it contains toolbars or buttons.
//
// Copyright 2002-2006  by Bill Spitzak and others.
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

#ifndef fltk_BarGroup_h
#define fltk_BarGroup_h

#include "Group.h"

namespace fltk {

class FL_API BarGroup : public Group {
  bool open_;
  bool highlighted;
  bool pushed;
  int glyph_size_;
  int saved_size;
  void glyph_box(Rectangle&) const;
public:
  BarGroup(int x,int y,int w ,int h,const char *l = 0,bool begin=false);
  static NamedStyle* default_style;
  void layout();
  int handle(int);
  void draw();
  bool opened() const {return open_;}
  bool opened(bool);
  bool open() {return opened(true);}
  bool close() {return opened(false);}
  int glyph_size() const {return glyph_size_;}
  void glyph_size(int v) {glyph_size_ = v;}
};

}
#endif

//
// End of "$Id: BarGroup.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
