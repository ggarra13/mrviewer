//
// "$Id: TextEditor.h 4899 2006-04-04 13:53:37Z fabien $"
//
// Header file for TextEditor class.
//
// Copyright 2001-2006 by Bill Spitzak and others.
// Original code Copyright Mark Edel.  Permission to distribute under
// the LGPL for the FLTK library granted by Mark Edel.
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


#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include "TextDisplay.h"

namespace fltk {

// key will match in any state
#define TEXT_EDITOR_ANY_STATE  (-1L)

/** TextEditor */
class FL_API TextEditor : public TextDisplay {
public:
  typedef int (*Key_Func)(int key, TextEditor* editor);

  struct Key_Binding {
    int          key;
    int          state;
    Key_Func     function;
    Key_Binding* next;
  };

  static NamedStyle* default_style;

  TextEditor(int X, int Y, int W, int H, const char* l = 0);
  ~TextEditor();

  virtual int handle(int e);

  /** Set new insert mode. true=insert, false=overstrike */
  void insert_mode(bool b) { insert_mode_ = b; }
  /** Return current insert mode */
  bool insert_mode() const { return insert_mode_; }

  void add_key_binding(int key, int state, Key_Func f, Key_Binding** list);
  void add_key_binding(int key, int state, Key_Func f)
    { add_key_binding(key, state, f, &key_bindings); }
  void remove_key_binding(int key, int state, Key_Binding** list);
  void remove_key_binding(int key, int state)
    { remove_key_binding(key, state, &key_bindings); }
  void remove_all_key_bindings(Key_Binding** list);
  void remove_all_key_bindings() { remove_all_key_bindings(&key_bindings); }
  void add_default_key_bindings(Key_Binding** list);
  Key_Func bound_key_function(int key, int state, Key_Binding* list);
  Key_Func bound_key_function(int key, int state)
    { return bound_key_function(key, state, key_bindings); }
  void default_key_function(Key_Func f) { default_key_function_ = f; }

  // functions for the built in default bindings
  static int kf_default(int c, TextEditor* e);
  static int kf_ignore(int c, TextEditor* e);
  static int kf_backspace(int c, TextEditor* e);
  static int kf_enter(int c, TextEditor* e);
  static int kf_move(int c, TextEditor* e);
  static int kf_shift_move(int c, TextEditor* e);
  static int kf_ctrl_move(int c, TextEditor* e);
  static int kf_c_s_move(int c, TextEditor* e);
  static int kf_home(int, TextEditor* e);
  static int kf_end(int c, TextEditor* e);
  static int kf_left(int c, TextEditor* e);
  static int kf_up(int c, TextEditor* e);
  static int kf_right(int c, TextEditor* e);
  static int kf_down(int c, TextEditor* e);
  static int kf_page_up(int c, TextEditor* e);
  static int kf_page_down(int c, TextEditor* e);
  static int kf_insert(int c, TextEditor* e);
  static int kf_delete(int c, TextEditor* e);
  static int kf_copy(int c, TextEditor* e);
  static int kf_cut(int c, TextEditor* e);
  static int kf_paste(int c, TextEditor* e);
  static int kf_select_all(int c, TextEditor* e);
  static int kf_undo(int c, TextEditor* e);

protected:
  int handle_key();
  void maybe_do_callback();

  bool insert_mode_;
  Key_Binding* key_bindings;
  static Key_Binding* global_key_bindings;
  Key_Func default_key_function_;
};

} /* namespace fltk */

#endif

//
// End of "$Id: TextEditor.h 4899 2006-04-04 13:53:37Z fabien $".
//


