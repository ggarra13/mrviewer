//
// "$Id: Fl_get_key_mac.cxx 3887 2004-11-12 06:50:20Z spitzak $"
//
// MacOS keyboard state routines for the Fast Light Tool Kit (FLTK).
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

// Return the current state of a key.  Keys are named by fltk symbols,
// which are actually X keysyms.  So this has to translate to macOS
// symbols.

#include <fltk/events.h>
#include <fltk/x.h>
using namespace fltk;

// convert an FLTK (X) keysym to a MacOS symbol:
// See also the inverse converter in mac.cxx
// This table is in numeric order by FLTK symbol order for binary search:

static const struct {unsigned short vk, fltk;} vktab[] = {
  { 49, ' ' }, { 39, '\'' }, { 43, ',' }, { 27, '-' },
  { 47, '.' }, { 44, '/' }, 
  { 29, '0' }, { 18, '1'  }, { 19, '2'  }, { 20, '3'  }, 
  { 21, '4' }, { 23, '5'  }, { 22, '6'  }, { 26, '7'  }, 
  { 28, '8' }, { 25, '9'  }, { 41, ';'  }, { 24, '='  },
  {  0, 'A' }, { 11, 'B'  }, {  8, 'C'  }, {  2, 'D'  }, 
  { 14, 'E' }, {  3, 'F'  }, {  5, 'G'  }, {  4, 'H'  }, 
  { 34, 'I' }, { 38, 'J'  }, { 40, 'K'  }, { 37, 'L'  }, 
  { 46, 'M' }, { 45, 'N'  }, { 31, 'O'  }, { 35, 'P'  }, 
  { 12, 'Q' }, { 15, 'R'  }, {  1, 'S'  }, { 17, 'T'  }, 
  { 32, 'U' }, {  9, 'V'  }, { 13, 'W'  }, {  7, 'X'  }, 
  { 16, 'Y' }, {  6, 'Z'  }, 
  { 33, '[' }, { 42, '\\'}, { 30, ']' }, { 50, '`' },  { 42, '|' },
  { 51, BackSpaceKey }, { 48, TabKey }, { 36, ReturnKey }, { 106, PauseKey },
  { 113, ScrollLockKey }, { 53, EscapeKey }, { 115, HomeKey }, {123, LeftKey},
  { 126, UpKey }, { 124, RightKey }, { 125, DownKey }, { 116, PageUpKey },
  { 121, PageDownKey },  { 119, EndKey }, { 107, PrintKey}, {114, InsertKey},
  { 127, fltk::MenuKey }, { 114, HelpKey }, { 71, NumLockKey }, 
  { 76, KeypadEnter }, { 67, MultiplyKey }, { 69, AddKey}, { 78, SubtractKey},
  { 65, DecimalKey }, { 75, DivideKey }, 
  { 82, Keypad0 }, { 83, Keypad1 }, { 84, Keypad2 }, { 85, Keypad3 }, 
  { 86, Keypad4 }, { 87, Keypad5 }, { 88, Keypad6 }, { 89, Keypad7 }, 
  { 91, Keypad8 }, { 92, Keypad9 }, { 81, Keypad+'=' }, 
  { 122, F1Key }, { 120, F2Key }, { 99, F3Key }, { 118, F4Key }, 
  { 96, F5Key }, { 97, F6Key }, { 98, F7Key }, { 100, F8Key }, 
  { 101, F9Key }, { 109, F10Key }, { 103, F11Key }, { 111, F12Key }, 
  { 105, F0Key+13 }, { 107, F0Key+14 }, { 113, F0Key+15}, { 106, F0Key+16 },
  { 56, LeftShiftKey }, { 56, RightShiftKey },
  { 59, LeftCtrlKey }, { 59, RightCtrlKey }, 
  { 57, CapsLockKey },
  { 55, LeftMetaKey }, { 55, RightMetaKey },
  { 58, LeftAltKey }, { 58, RightAltKey }, {117, DeleteKey }
};

static unsigned short fltk2mac(unsigned fltk) {
  if (fltk >= 'a' && fltk <= 'z') fltk-=('a'-'A');
  int a = 0;
  int b = sizeof(vktab)/sizeof(*vktab);
  while (a < b) {
    int c = (a+b)/2;
    if (vktab[c].fltk == fltk) return vktab[c].vk;
    if (vktab[c].fltk < fltk) a = c+1; else b = c;
  }
  return 127;
}

//: returns true, if that key was pressed during the last event
bool fltk::event_key_state(unsigned k) {
  return get_key_state(k);
}

//#include <stdio.h>

//: returns true, if that key is pressed right now
bool fltk::get_key_state(unsigned k) {
  KeyMap foo;
  GetKeys(foo);
#ifdef MAC_TEST_FOR_KEYCODES
 static int cnt = 0;
 if (cnt++>1024) {
  cnt = 0;
  printf("%08x %08x %08x %08x\n", (ulong*)(foo)[3], (ulong*)(foo)[2], (ulong*)(foo)[1], (ulong*)(foo)[0]);
 }
#endif
  int i = fltk2mac(k);
  unsigned char *b = (unsigned char*)foo;
  return (b[i>>3]>>(i&7))&1;
}

//
// End of "$Id: Fl_get_key_mac.cxx 3887 2004-11-12 06:50:20Z spitzak $".
//
