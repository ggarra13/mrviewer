//
// "$Id: event_key_state.cxx 1464 2006-12-28 21:37:21Z spitzak $"
//
// _WIN32 keyboard state routines for the Fast Light Tool Kit (FLTK).
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
// which are actually X keysyms.  So this has to translate to MSWindows
// VK_x symbols.

#include <fltk/events.h>
#include <fltk/x.h>
#include <ctype.h>
#ifndef VK_LWIN
# define VK_LWIN 0x5B
# define VK_RWIN 0x5C
# define VK_APPS 0x5D
#endif

using namespace fltk;

static const struct {unsigned short vk, fltk, ext;} vktab[] = {
  {VK_SPACE,	' '},
  {'1',		'!'},
  {0xde,	'\"'},
  {'3',		'#'},
  {'4',		'$'},
  {'5',		'%'},
  {'7',		'&'},
  {0xde,	'\''},
  {'9',		'('},
  {'0',		')'},
  {'8',		'*'},
  {0xbb,	'+'},
  {0xbc,	','},
  {0xbd,	'-'},
  {0xbe,	'.'},
  {0xbf,	'/'},
  {0xba,	':'},
  {0xba,	';'},
  {0xbc,	'<'},
  {0xbb,	'='},
  {0xbe,	'>'},
  {0xbf,	'?'},
  {'2',		'@'},
  {0xdb,	'['},
  {0xdc,	'\\'},
  {0xdd,	']'},
  {'6',		'^'},
  {0xbd,	'_'},
  {0xc0,	'`'},
  {0xdb,	'{'},
  {0xdc,	'|'},
  {0xdd,	'}'},
  {0xc0,	'~'},
  {VK_BACK,	BackSpaceKey},
  {VK_TAB,	TabKey},
  {VK_CLEAR,	ClearKey},
  {VK_RETURN,	ReturnKey},
  {VK_PAUSE,	PauseKey},
  {VK_SCROLL,	ScrollLockKey},
  {VK_ESCAPE,	EscapeKey},
  {VK_HOME,	HomeKey},
  {VK_LEFT,	LeftKey},
  {VK_UP,	UpKey},
  {VK_RIGHT,	RightKey},
  {VK_DOWN,	DownKey},
  {VK_PRIOR,	PageUpKey},
  {VK_NEXT,	PageDownKey},
  {VK_END,	EndKey},
  {VK_SNAPSHOT,	PrintKey},
  {VK_INSERT,	InsertKey},
  {VK_APPS,	MenuKey},
  {VK_NUMLOCK,	NumLockKey},
//{VK_RETURN,	KeypadEnter},
  {VK_MULTIPLY,	MultiplyKey},
  {VK_ADD,	AddKey},
  {VK_SUBTRACT,	SubtractKey},
  {VK_DELETE,	DecimalKey},
  {VK_DIVIDE,	DivideKey},
  {VK_INSERT,	Keypad0},
  {VK_END,	Keypad1},
  {VK_DOWN,	Keypad2},
  {VK_NEXT,	Keypad3},
  {VK_LEFT,	Keypad4},
  {VK_CLEAR,	Keypad5},
  {VK_RIGHT,	Keypad6},
  {VK_HOME,	Keypad7},
  {VK_UP,	Keypad8},
  {VK_PRIOR,	Keypad9},
  {VK_LSHIFT,	LeftShiftKey},
  {VK_RSHIFT,	RightShiftKey},
  {VK_LCONTROL,	LeftCtrlKey},
  {VK_RCONTROL,	RightCtrlKey},
  {VK_CAPITAL,	CapsLockKey},
  {VK_LWIN,	LeftMetaKey},
  {VK_RWIN,	RightMetaKey},
  {VK_LMENU,	LeftAltKey},
  {VK_RMENU,	RightAltKey},
  {VK_DELETE,	DeleteKey}
};

extern bool fl_last_was_extended;

static unsigned short fltk2ms(unsigned fltk) {
  if (fltk >= '0' && fltk <= '9') return fltk;
  if (fltk >= 'A' && fltk <= 'Z') return fltk;
  if (fltk >= 'a' && fltk <= 'z') return fltk-('a'-'A');
  if (fltk >= F1Key && fltk <= LastFunctionKey) return fltk-F1Key+VK_F1;
  if (event_state(NUMLOCK) || fl_last_was_extended) {
    if (fltk >= Keypad0 && fltk<=Keypad9) return fltk-Keypad0+VK_NUMPAD0;
    if (fltk == DecimalKey) return VK_DECIMAL;
  }
  if (fltk == KeypadEnter && fl_last_was_extended)
    return VK_RETURN;
  int a = 0;
  int b = sizeof(vktab)/sizeof(*vktab);
  while (a < b) {
    int c = (a+b)/2;
    if (vktab[c].fltk == fltk) return vktab[c].vk;
    if (vktab[c].fltk < fltk) a = c+1; else b = c;
  }
  return 0;
}

bool fltk::event_key_state(unsigned k) {
  return (GetKeyState(fltk2ms(k))&~1) != 0;
}

bool fltk::get_key_state(unsigned k) {
  return (GetAsyncKeyState(fltk2ms(k))&~1) != 0;
//    uchar foo[256];
//    GetKeyboardState(foo);
//    return (foo[fltk2ms(k)]&~1) != 0;
}

//
// End of "$Id: event_key_state.cxx 1464 2006-12-28 21:37:21Z spitzak $".
//
