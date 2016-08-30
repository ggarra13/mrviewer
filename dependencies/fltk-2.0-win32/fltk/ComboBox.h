//
// 
//
// single line input field with predefined choices via popup menu
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

#ifndef fltk_ComboBox_h
#define fltk_ComboBox_h

#include "Choice.h"
#include "Input.h"

namespace fltk {

class FL_API ComboBox : public Choice {
public:
  ComboBox(int,int,int,int,const char * = 0);
  ~ComboBox();
  static NamedStyle* default_style;

  void draw();
  void layout();
  int handle(int);
  int popup(const Rectangle&,const char* title=0,bool menubar=false);

  int choice(int v);
  int choice() const;
  int find_choice() const;
  bool text(const char *txt)
	{ return text_changed_(input_->text(txt)); }
  bool text(const char *txt, int n)
	{ return text_changed_(input_->text(txt, n)); }
  bool static_text(const char *txt)
	{ return text_changed_(input_->static_text(txt)); }
  bool static_text(const char *txt, int n)
	{ return text_changed_(input_->static_text(txt, n)); }
  const char* text() const { return input_->text();}
  char at(int i) const { return input_->at(i); }
  
  int size(bool ofText) const 
	{ return ofText ? input_->size() : Choice::size(); }

  int position() const { return input_->position();}
  int mark() const { return input_->mark();}
  void position(int p, int m)
	{ input_->position(p, m); text_changed_(); }
  void position(int p) { position(p, p); }
  void up_down_position(int p, bool m) 
	{ input_->up_down_position(p, m); text_changed_(); }
  void mark(int m) { position(position(), m);}

  virtual bool replace(int a, int b, const char *c, int d)
	{ return text_changed_(input_->replace(a, b, c, d)); }
  bool cut() {return replace(position(), mark(), 0, 0);}
  bool cut(int n) {return replace(position(), position()+n, 0, 0);}
  bool cut(int a, int b) {return replace(a, b, 0, 0);}
  bool insert(const char* t, int l=0)
	{ return replace(input_->position(), input_->mark(), t, l); }
  bool replace(int a, int b, char c) { return replace(a,b,&c,1); }
  bool copy(bool clipboard = true) { return input_->copy(clipboard); }
  bool undo() { return text_changed_(input_->undo()); }

  int word_start(int i) const { return input_->word_start(i); }
  int word_end(int i) const { return input_->word_end(i); }
  int line_start(int i) const { return input_->line_start(i); }
  int line_end(int i) const  { return input_->line_end(i); }
  int mouse_position(const Rectangle& r) const
	{ return input_->mouse_position(r); }
  int xscroll() const { return input_->xscroll();}
  int yscroll() const { return input_->yscroll();}
  
protected:
  bool text_changed_(bool ret=true);

private:
  Input *input_;
#if defined (_WIN32) || (defined( __GNUC__ ) && __GNUC__ < 3) 
 public:
#endif
  static void input_callback_(Widget*,void*);
};

}

#endif

//
// 
//
