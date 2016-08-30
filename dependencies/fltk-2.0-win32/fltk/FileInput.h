//
// "$Id: FileInput.h 8316 2011-01-26 14:59:50Z bgbnbigben $"
//
// File_Input header file for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2006 by Bill Spitzak and others.
// Original version Copyright 1998 by Curtis Edwards.
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
//     http://www.fltk.org/str.php
//

#ifndef fltk_FileInput_H
#  define fltk_FileInput_H

#  include "Input.h"

namespace fltk {

class FL_API FileInput : public Input {
  Color	errorcolor_;
  char		ok_entry_;
  short		buttons_[120];
  short		pressed_;

  void		draw_buttons();
  int		handle_button(int event);
  void		update_buttons();

public:

  FileInput(int,int,int,int,const char *t=0);

  void draw_boxes(bool pressed, const Rectangle& r);
  virtual int handle(int);
  virtual void draw();

  Color			errorcolor() const { return errorcolor_; }
  void			errorcolor(Color c) { errorcolor_ = c; }
  int			text(const char*);
  int			text(const char*, int);
  const char *		text() const { return Input::text(); }

  int		value(const char* v) { return text(v); }
  int		value(const char* v, int n) { return text(v,n); }
  const char*	value() const { return text(); }
};

}
#endif // !Fl_File_Input_H


//
// End of "$Id: FileInput.h 8316 2011-01-26 14:59:50Z bgbnbigben $".
//
