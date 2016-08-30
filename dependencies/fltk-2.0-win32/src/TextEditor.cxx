//
// "$Id: TextEditor.cxx 6508 2008-11-09 23:35:19Z spitzak $"
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <fltk/events.h>
#include <fltk/TextEditor.h>
#include <fltk/ask.h>

using namespace fltk;

static void revert(Style* s) { }
static NamedStyle style("TextEditor", revert, &TextEditor::default_style);
NamedStyle* TextEditor::default_style = &::style;

TextEditor::TextEditor(int X, int Y, int W, int H, const char* l)
    : TextDisplay(X, Y, W, H, l) {
  style(default_style);
  cursor_on_ = false;
  insert_mode_ = true;
  key_bindings = 0;

  // handle the default key bindings
  add_default_key_bindings(&key_bindings);

  // handle everything else
  default_key_function(kf_default);
}

extern Widget* fl_pending_callback;
TextEditor::~TextEditor() {
  if (fl_pending_callback == this) fl_pending_callback = 0;
  remove_all_key_bindings();
}

TextEditor::Key_Binding* TextEditor::global_key_bindings = 0;

// These are the default key bindings every widget should start with
static struct {
    int key;
    int state;
    TextEditor::Key_Func func;
} default_key_bindings[] = {
    { EscapeKey,   TEXT_EDITOR_ANY_STATE, TextEditor::kf_ignore     },
    { ReturnKey,   TEXT_EDITOR_ANY_STATE, TextEditor::kf_enter      },
    { KeypadEnter, TEXT_EDITOR_ANY_STATE, TextEditor::kf_enter      },
    { BackSpaceKey,TEXT_EDITOR_ANY_STATE, TextEditor::kf_backspace  },
    { InsertKey,   TEXT_EDITOR_ANY_STATE, TextEditor::kf_insert     },
    { DeleteKey,   TEXT_EDITOR_ANY_STATE, TextEditor::kf_delete     },
    { HomeKey,     0,                     TextEditor::kf_move       },
    { EndKey,      0,                     TextEditor::kf_move       },
    { LeftKey,     0,                     TextEditor::kf_move       },
    { UpKey,       0,                     TextEditor::kf_move       },
    { RightKey,    0,                     TextEditor::kf_move       },
    { DownKey,     0,                     TextEditor::kf_move       },
    { PageUpKey,   0,                     TextEditor::kf_move       },
    { PageDownKey, 0,                     TextEditor::kf_move       },
    { HomeKey,     SHIFT,                 TextEditor::kf_shift_move },
    { EndKey,      SHIFT,                 TextEditor::kf_shift_move },
    { LeftKey,     SHIFT,                 TextEditor::kf_shift_move },
    { UpKey,       SHIFT,                 TextEditor::kf_shift_move },
    { RightKey,    SHIFT,                 TextEditor::kf_shift_move },
    { DownKey,     SHIFT,                 TextEditor::kf_shift_move },
    { PageUpKey,   SHIFT,                 TextEditor::kf_shift_move },
    { PageDownKey, SHIFT,                 TextEditor::kf_shift_move },
    { HomeKey,     CTRL,                  TextEditor::kf_ctrl_move  },
    { EndKey,      CTRL,                  TextEditor::kf_ctrl_move  },
    { LeftKey,     CTRL,                  TextEditor::kf_ctrl_move  },
    { UpKey,       CTRL,                  TextEditor::kf_ctrl_move  },
    { RightKey,    CTRL,                  TextEditor::kf_ctrl_move  },
    { DownKey,     CTRL,                  TextEditor::kf_ctrl_move  },
    { PageUpKey,   CTRL,                  TextEditor::kf_ctrl_move  },
    { PageDownKey, CTRL,                  TextEditor::kf_ctrl_move  },
    { HomeKey,     CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    { EndKey,      CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    { LeftKey,     CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    { UpKey,       CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    { RightKey,    CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    { DownKey,     CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    { PageUpKey,   CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    { PageDownKey, CTRL|SHIFT,            TextEditor::kf_c_s_move   },
    //{ Clear,    0,                        TextEditor::delete_to_eol },
    { 'z',          CTRL,                 TextEditor::kf_undo      },
    { '/',          CTRL,                 TextEditor::kf_undo      },
    { 'x',          CTRL,                 TextEditor::kf_cut        },
    { DeleteKey,    SHIFT,                TextEditor::kf_cut        },
    { 'c',          CTRL,                 TextEditor::kf_copy       },
    { InsertKey,    CTRL,                 TextEditor::kf_copy       },
    { 'v',          CTRL,                 TextEditor::kf_paste      },
    { InsertKey,    SHIFT,                TextEditor::kf_paste      },
    { 'a',          CTRL,                 TextEditor::kf_select_all },

#ifdef __APPLE__
    // Define CMD+key accelerators...
    { 'z',          COMMAND,              TextEditor::kf_undo       },
    { 'x',          COMMAND,              TextEditor::kf_cut        },
    { 'c',          COMMAND,              TextEditor::kf_copy       },
    { 'v',          COMMAND,              TextEditor::kf_paste      },
    { 'a',          COMMAND,              TextEditor::kf_select_all },
#endif // __APPLE__
    { 0,            0,                    0                         }
};

void TextEditor::add_default_key_bindings(Key_Binding** list) {
  for (int i = 0; default_key_bindings[i].key; i++) {
    add_key_binding(default_key_bindings[i].key,
                    default_key_bindings[i].state,
                    default_key_bindings[i].func,
                    list);
  }
}

TextEditor::Key_Func TextEditor::bound_key_function(int key, int state, Key_Binding* list) {
  Key_Binding* cur;
  for (cur = list; cur; cur = cur->next)
    if (cur->key == key)
      if (cur->state == TEXT_EDITOR_ANY_STATE || cur->state == state)
        break;
  if (!cur) return 0;
  return cur->function;
}

void TextEditor::remove_all_key_bindings(Key_Binding** list) {
  Key_Binding *cur, *next;
  for (cur = *list; cur; cur = next) {
      next = cur->next;
      delete cur;
  }
  *list = 0;
}

void TextEditor::remove_key_binding(int key, int state, Key_Binding** list) {
  Key_Binding *cur, *last = 0;
  for (cur = *list; cur; last = cur, cur = cur->next)
      if (cur->key == key && cur->state == state) break;
  if (!cur) return;
  if (last) last->next = cur->next;
  else *list = cur->next;
  delete cur;
}

void TextEditor::add_key_binding(int key, int state, Key_Func function,
                                Key_Binding** list) {
  Key_Binding* kb = new Key_Binding;
  kb->key = key;
  kb->state = state;
  kb->function = function;
  kb->next = *list;
  *list = kb;
}

////////////////////////////////////////////////////////////////

#define NORMAL_INPUT_MOVE 0

static void kill_selection(TextEditor* e) {
  if (e->buffer()->selected()) {
    e->insert_position(e->buffer()->primary_selection()->start());
    e->buffer()->remove_selection();
  }
}

int TextEditor::kf_default(int c, TextEditor* e) {
  if (!c || (!isprint(c) && c != '\t')) return 0;
  char s[2] = { c, '\0' };
  kill_selection(e);
  if (e->insert_mode()) e->insert(s);
  else e->overstrike(s);
  e->show_insert_position();
  e->maybe_do_callback();
  return 1;
}

int TextEditor::kf_ignore(int, TextEditor*) {
  return 0; // don't handle
}

int TextEditor::kf_backspace(int, TextEditor* e) {
  int oldpos = e->insert_position();
  if (!e->buffer()->selected() && e->move_left())
    e->buffer()->select(e->insert_position(), oldpos);
  kill_selection(e);
  e->show_insert_position();
  e->maybe_do_callback();
  return 1;
}

int TextEditor::kf_enter(int, TextEditor* e) {
  kill_selection(e);
  e->insert("\n");
  e->show_insert_position();
  e->maybe_do_callback();
  return 1;
}

int TextEditor::kf_move(int c, TextEditor* e) {
  int i;
  int selected = e->buffer()->selected();
  if (!selected)
      e->dragpos_ = e->insert_position();
  e->buffer()->unselect();
  switch (c) {
  case HomeKey:
    e->insert_position(e->buffer()->line_start(e->insert_position()));
    break;
  case EndKey:
    e->insert_position(e->buffer()->line_end(e->insert_position()));
    break;
  case LeftKey:
    e->move_left();
    break;
  case RightKey:
    e->move_right();
    break;
  case UpKey:
    e->move_up();
    break;
  case DownKey:
    e->move_down();
    break;
  case PageUpKey:
    for (i = 0; i < e->visible_lines() - 1; i++) e->move_up();
    break;
  case PageDownKey:
    for (i = 0; i < e->visible_lines() - 1; i++) e->move_down();
    break;
  }
  e->show_insert_position();
  return 1;
}

int TextEditor::kf_shift_move(int c, TextEditor* e) {
  kf_move(c, e);
  e->text_drag_me(e->insert_position());
  return 1;
}

int TextEditor::kf_ctrl_move(int c, TextEditor* e) {
  if (!e->buffer()->selected())
    e->dragpos_ = e->insert_position();
  if (c != UpKey && c != DownKey) {
    e->buffer()->unselect();
    e->show_insert_position();
  }
  switch (c) {
  case HomeKey:
    e->insert_position(0);
    e->scroll(0, 0);
    break;
  case EndKey:
    e->insert_position(e->buffer()->length());
    e->scroll(e->count_lines(0, e->buffer()->length(), 1), 0);
    break;
  case LeftKey:
    e->previous_word();
    break;
  case RightKey:
    e->next_word();
    break;
  case UpKey:
    e->scroll(e->top_line()-1, e->hor_offset());
    break;
  case DownKey:
    e->scroll(e->top_line()+1, e->hor_offset());
    break;
  case PageUpKey:
    e->insert_position(e->linestarts_[0]);
    break;
  case PageDownKey:
    e->insert_position(e->linestarts_[e->visible_lines()-2]);
    break;
  }
  e->show_insert_position();
  return 1;
}

int TextEditor::kf_c_s_move(int c, TextEditor* e) {
  kf_ctrl_move(c, e);
  e->text_drag_me(e->insert_position());
  return 1;
}

int TextEditor::kf_home(int, TextEditor* e) {
  return kf_move(HomeKey, e);
}

int TextEditor::kf_end(int, TextEditor* e) {
  return kf_move(EndKey, e);
}

int TextEditor::kf_left(int, TextEditor* e) {
  return kf_move(LeftKey, e);
}

int TextEditor::kf_up(int, TextEditor* e) {
  return kf_move(UpKey, e);
}

int TextEditor::kf_right(int, TextEditor* e) {
  return kf_move(RightKey, e);
}

int TextEditor::kf_down(int, TextEditor* e) {
  return kf_move(DownKey, e);
}

int TextEditor::kf_page_up(int, TextEditor* e) {
  return kf_move(PageUpKey, e);
}

int TextEditor::kf_page_down(int, TextEditor* e) {
  return kf_move(PageDownKey, e);
}

int TextEditor::kf_insert(int, TextEditor* e) {
  e->insert_mode(e->insert_mode() ? 0 : 1);
  return 1;
}

int TextEditor::kf_delete(int, TextEditor* e) {
  int oldpos = e->insert_position();
  if (!e->buffer()->selected() && e->move_right())
    e->buffer()->select(oldpos, e->insert_position());
  kill_selection(e);
  e->show_insert_position();
  e->maybe_do_callback();
  return 1;
}

int TextEditor::kf_copy(int, TextEditor* e) {
  if (!e->buffer()->selected()) return 1;
  char *copy = e->buffer()->selection_text();
  if (*copy) {
    fltk::copy(copy, strlen(copy), 1);
  }
  free(copy);
  e->show_insert_position();
  return 1;
}

int TextEditor::kf_cut(int c, TextEditor* e) {
  kf_copy(c, e);
  kill_selection(e);
  e->maybe_do_callback();
  return 1;
}

int TextEditor::kf_paste(int, TextEditor* e) {
  kill_selection(e);
  paste(*e, 1);
  e->show_insert_position();
  e->maybe_do_callback();
  return 1;
}

int TextEditor::kf_select_all(int, TextEditor* e) {
  e->buffer()->select(0, e->buffer()->length());
  return 1;
}

int TextEditor::kf_undo(int , TextEditor* e) {
  e->buffer()->unselect();
  int crsr;
  int ret = e->buffer()->undo(&crsr);
  e->insert_position(crsr);
  e->show_insert_position();
  e->maybe_do_callback();
  return ret;
}

int TextEditor::handle_key() {
  // Call FLTK's rules to try to turn this into a printing character.
  // This uses the right-hand ctrl key as a "compose prefix" and returns
  // the changes that should be made to the text, as a number of
  // bytes to delete and a string to insert:
  int del;
  if (fltk::compose(del)) {
    if (del) buffer()->select(insert_position()-del, insert_position());
    if (event_length()) {
      kill_selection(this);
      if (insert_mode()) insert(event_text());
      else overstrike(event_text());
    }
    show_insert_position();
    maybe_do_callback();
    return 1;
  }

  int key = event_key(), state = event_state(), c = event_text()[0];
  state &= SHIFT|CTRL|ALT|META; // only care about these states
  Key_Func f;
  f = bound_key_function(key, state, global_key_bindings);
  if (!f) f = bound_key_function(key, state, key_bindings);
  if (f) return f(key, this);
  if (default_key_function_ && !state) return default_key_function_(c, this);
  return 0;
}

// Called by any changes to the text, this correctly triggers callbacks:
void TextEditor::maybe_do_callback() {
  set_changed();
  if (when() & (WHEN_RELEASE|WHEN_ENTER_KEY)) {
    Widget* w = fl_pending_callback;
    if (w == this) return;
    if (w) {fl_pending_callback = 0; w->do_callback();}
    fl_pending_callback = this;
  } else {
    if (when()) do_callback();
  }
}

int TextEditor::handle(int event) {
  if (!buffer()) return 0;

  switch (event) {
  case FOCUS:
    show_cursor(true);
    if (buffer()->selected()) {
      redraw(); // Redraw selections...
    }
    fltk::focus(this);
    return 1;

  case UNFOCUS:
    show_cursor(false);
    if (buffer()->selected()) {
      redraw(); // Redraw selections...
    }
  case HIDE:
    return 1;

  case KEY:
    return handle_key();

  case PASTE:
    if (!event_text()) {
      return 1;
    }
    buffer()->remove_selection();
    if (insert_mode()) {
      insert(event_text());
    } else {
      overstrike(event_text());
    }
    show_insert_position();
    maybe_do_callback();
    return 1;

  case ENTER:
    // MRS: WIN32 only?  Need to test!
    //    case MOVE:
    show_cursor(cursor_on());
    return 1;

  case PUSH:
    if (event_button() == 2) {
      dragtype_ = -1;
      fltk::paste(*this, 0);
      fltk::focus(this);
      maybe_do_callback();
      return 1;
    }
    break;

  }

  return TextDisplay::handle(event);
}

//
// End of "$Id: TextEditor.cxx 6508 2008-11-09 23:35:19Z spitzak $".
//
