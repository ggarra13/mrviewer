//
// "$Id: Group.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Group is the base class for all container widgets. For historical
// reasons it also provides a default version of layout.
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

#ifndef fltk_Group_h
#define fltk_Group_h

#ifndef fltk_Widget_h
# include "Widget.h"
#endif

namespace fltk {

class FL_API Group : public Widget {
public:

  int children() const {return children_;}
  Widget* child(int n) const {return array_[n];}

  void draw();
  void layout();
  int handle(int);

  void begin() {current_ = this;}
  void end() {current_ = (Group*)parent();}
  static Group *current() {return current_;}
  static void current(Group *g) {current_ = g;}

  int find(const Widget*) const;
  int find(const Widget& o) const {return find(&o);}

  Group(int,int,int,int, const char * = 0, bool begin=false);
  virtual ~Group();
  void add(Widget&);
  void add(Widget* o) {add(*o);}
  void insert(Widget&, int index);
  void insert(Widget& o, Widget* before) {insert(o,find(before));}
  void remove(int index);
  void remove(Widget& o) {remove(find(o));}
  void remove(Widget* o) {remove(find(*o));}
  void remove_all();
  void replace(int index, Widget&);
  void replace(Widget& old, Widget& o) {replace(find(old),o);}
  void swap(int indexA, int indexB);
  void clear();

  void resizable(Widget& o) {resizable_ = &o;}
  void resizable(Widget* o) {resizable_ = o;}
  Widget* resizable() const {return resizable_;}
  void add_resizable(Widget& o) {resizable_ = &o; add(o);}
  void init_sizes();

  void focus_index(int v) {focus_index_ = v;}
  void set_focus(Widget* w) {focus_index_ = find(w);}
  int  focus_index() const {return focus_index_;}
  static int navigation_key();

  // back compatability function:
  friend FL_FORMS_API void end_group(); // forms emulation function
  void fix_old_positions();

  Flags resize_align() const {return resize_align_;}
  void resize_align(Flags f) {resize_align_ = f;}

protected:

  void draw_child(Widget&) const;
  void update_child(Widget&) const;
  void draw_outside_label(Widget&) const ;
  int initial_w, initial_h;
  int* sizes();
  void layout(const Rectangle&, int layout_damage);

private:

  int children_;
  int focus_index_;
  Widget** array_;
  Widget* resizable_;
  Flags resize_align_;
  int *sizes_; // remembered initial sizes of children

  static Group *current_;

};

}

#endif

//
// End of "$Id: Group.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
