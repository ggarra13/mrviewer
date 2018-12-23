//
// "$Id: keyboard.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Keyboard/event test program for the Fast Light Tool Kit (FLTK).
//
// Continuously display FLTK's event state.
//
// Known bugs:
//
// X insists on reporting the state *before* the shift key was
// pressed, rather than after, on shift key events.  I fixed this for
// the mouse buttons, but it did not seem worth it for shift.
//
// X servers do not agree about any shift flags except shift, ctrl,
// lock, and alt.  They may also not agree about the symbols for the extra
// keys Micro$oft put on the keyboard.
//
// Windows has a number of keys that do not report that they are held down.
// On IRIX the backslash key does not report it is held down.
//
// Copyright 2001 by Bill Spitzak and others.
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

#include "keyboard_ui.cxx"
#include <stdio.h>
#include <ctype.h>
#include <fltk/run.h>

// these are used to identify which buttons are which:
void key_cb(fltk::Button*, void*) {}
void shift_cb(fltk::Button*, void*) {}

static struct {int n; const char* text;} table[] = {
  {fltk::EscapeKey, "fltk::EscapeKey"},
  {fltk::BackSpaceKey, "fltk::BackSpaceKey"},
  {fltk::TabKey, "fltk::TabKey"},
  {fltk::ReturnKey, "fltk::ReturnKey"},
  {fltk::PrintKey, "fltk::PrintKey"},
  {fltk::ScrollLockKey, "fltk::ScrollLockKey"},
  {fltk::PauseKey, "fltk::PauseKey"},
  {fltk::InsertKey, "fltk::InsertKey"},
  {fltk::HomeKey, "fltk::HomeKey"},
  {fltk::PageUpKey, "fltk::PageUpKey"},
  {fltk::DeleteKey, "fltk::DeleteKey"},
  {fltk::EndKey, "fltk::EndKey"},
  {fltk::PageDownKey, "fltk::PageDownKey"},
  {fltk::LeftKey, "fltk::LeftKey"},
  {fltk::UpKey, "fltk::UpKey"},
  {fltk::RightKey, "fltk::RightKey"},
  {fltk::DownKey, "fltk::DownKey"},
  {fltk::LeftShiftKey, "fltk::LeftShiftKey"},
  {fltk::RightShiftKey, "fltk::RightShiftKey"},
  {fltk::LeftCtrlKey, "fltk::LeftCtrlKey"},
  {fltk::RightCtrlKey, "fltk::RightCtrlKey"},
  {fltk::CapsLockKey, "fltk::CapsLockKey"},
  {fltk::LeftAltKey, "fltk::LeftAltKey"},
  {fltk::RightAltKey, "fltk::RightAltKey"},
  {fltk::LeftMetaKey, "fltk::LeftMetaKey"},
  {fltk::RightMetaKey, "fltk::RightMetaKey"},
  {fltk::MenuKey, "fltk::MenuKey"},
  {fltk::NumLockKey, "fltk::NumLockKey"},
  {fltk::KeypadEnter, "fltk::KeypadEnter"},
  {fltk::MultiplyKey, "fltk::MultiplyKey"},
  {fltk::AddKey, "fltk::AddKey"},
  {fltk::SubtractKey, "fltk::SubtractKey"},
  {fltk::DecimalKey, "fltk::DecimalKey"},
  {fltk::DivideKey, "fltk::DivideKey"},
  {fltk::Keypad0, "fltk::Keypad0"},
  {fltk::Keypad1, "fltk::Keypad1"},
  {fltk::Keypad2, "fltk::Keypad2"},
  {fltk::Keypad3, "fltk::Keypad3"},
  {fltk::Keypad4, "fltk::Keypad4"},
  {fltk::Keypad5, "fltk::Keypad5"},
  {fltk::Keypad6, "fltk::Keypad6"},
  {fltk::Keypad7, "fltk::Keypad7"},
  {fltk::Keypad8, "fltk::Keypad8"},
  {fltk::Keypad9, "fltk::Keypad9"},
  {fltk::SpaceKey,"fltk::SpaceKey (' ')"}
};

int main(int argc, char** argv) {
  fltk::Window *window = make_window();
  window->show(argc,argv);
  for (;;) {
    fltk::wait();
    if (!window->visible()) break;

    // update all the buttons with the current key and shift state:
    for (int i = 0; i < window->children(); i++) {
      fltk::Widget* b = window->child(i);
      if (b->callback() == (fltk::Callback*)key_cb) {
	unsigned i = (unsigned)b->argument();
	if (!i) i = tolower(b->label()[0]);
	((fltk::Button*)b)->value(fltk::event_key_state(i));
	if (i == fltk::event_key()) ((fltk::Button*)b)->take_focus();
      } else if (b->callback() == (fltk::Callback*)shift_cb) {
	unsigned i = (unsigned)b->argument();
	((fltk::Button*)b)->value(fltk::event_state(i));
      }
    }

    // figure out the keyname:
    char buffer[100];
    const char *keyname = buffer;
    int k = fltk::event_key();
    if (!k) {
      keyname = "0";
    } else if (k <= 7) {
      sprintf(buffer, "(mouse button %d)", k);
    } else if (k < 256 && k != 32) {
      sprintf(buffer, "'%c'", k);
    } else if (k >= fltk::F0Key && k <= fltk::LastFunctionKey) {
      sprintf(buffer, "fltk::F%dKey", k - fltk::F0Key);
    } else {
      sprintf(buffer, "0x%04x", k);
      for (int i = 0; i < int(sizeof(table)/sizeof(*table)); i++)
	if (table[i].n == k) {keyname = table[i].text; break;}
    }
    key_output->value(keyname);

    if (fltk::event_text()[0] == 32)
      text_output->value("\" \"");
    else if (fltk::event_text()[0])
      text_output->value(fltk::event_text());
    else
      text_output->value("\"\"");
  }
  return 0;
}

//
// End of "$Id: keyboard.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
