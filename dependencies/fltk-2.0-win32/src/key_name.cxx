//
// "$Id: key_name.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Turn a fltk (X) keysym + fltk shift flags into a human-readable string.
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

#include <config.h>
#include <fltk/events.h>
#include <fltk/Widget.h>
#include <fltk/draw.h>
#include <fltk/string.h>
#include <ctype.h>
#include <stdlib.h>
#if USE_X11
#include <fltk/x.h>
#endif
using namespace fltk;

// This table must be in numeric order by fltk (X) keysym number.
// On X the table is much shorter as it is only the names that
// are not returned correctly by XKeysymToString().
struct Keyname {unsigned key; const char* name;};
static Keyname table[] = {
#if !USE_X11
  {BackSpaceKey, "Backspace"},
  {TabKey,	"Tab"},
  {ClearKey,	"Clear"},
  {ReturnKey,	"Return"}, // older fltk said "Enter"
  {PauseKey,	"Pause"},
  {ScrollLockKey,"ScrollLock"},
  {EscapeKey,	"Escape"},
  {HomeKey,	"Home"},
  {LeftKey,	"Left"},
  {UpKey,	"Up"},
  {RightKey,	"Right"},
  {DownKey,	"Down"},
#endif
  {PageUpKey,	"PageUp"}, // X says "Prior"
  {PageDownKey,	"PageDown"}, // X says "Next"
#if !USE_X11
  {EndKey,	"End"},
  {PrintKey,	"Print"},
  {InsertKey,	"Insert"},
  {fltk::MenuKey,"Menu"},
  {NumLockKey,	"NumLock"},
  {KeypadEnter,	"KeypadEnter"},
  {LeftShiftKey,"LeftShift"},
  {RightShiftKey,"RightShift"},
  {LeftCtrlKey, "LeftCtrl"},
  {RightCtrlKey,"RightCtrl"},
  {CapsLockKey,	"CapsLock"},
  {LeftMetaKey,	"LeftMeta"},
  {RightMetaKey,"RightMeta"},
  {LeftAltKey,	"LeftAlt"},
  {RightAltKey,	"RightAlt"},
  {DeleteKey,	"Delete"}
#endif
};

/*!

  Unparse a fltk::Widget::shortcut(), an fltk::event_key(), or an
  fltk::event_key() or'd with fltk::event_state().  Returns a pointer
  to a human-readable string like "Alt+N". If \a hotkey is zero an
  empty string is returned. The return value points at a static buffer
  that is overwritten with each call.

  The opposite function is fltk::key().
*/
const char* fltk::key_name(unsigned hotkey) {
  static char buf[20];
  char *p = buf;
  if (!hotkey) {*p = 0; return buf;}
#ifdef __APPLE__
  if (hotkey & META) {strcpy(p,"Cmd+"); p += 4;}
#else
  if (hotkey & META) {strcpy(p,"Meta+"); p += 5;}
#endif
  if (hotkey & ALT) {strcpy(p,"Alt+"); p += 4;}
  if (hotkey & SHIFT) {strcpy(p,"Shift+"); p += 6;}
  if (hotkey & CTRL) {strcpy(p,"Ctrl+"); p += 5;}
  unsigned key = hotkey & 0xFFFF;
  // Turn control characters into the key that would produce them:
  if (key < 32 || key==127) key |= 0xff00;
  // binary search the table for a match:
  unsigned a = 0;
  unsigned b = sizeof(table)/sizeof(*table);
  const char* q = 0;
  while (a < b) {
    unsigned c = (a+b)/2;
    if (table[c].key == key) {q = table[c].name; break;}
    if (table[c].key < key) a = c+1;
    else b = c;
  }
  if (!q) {
#if USE_X11
    if (key <= 32 || key >= 0x100) q = XKeysymToString(key);
#else
    if (key >= F0Key && key <= LastFunctionKey) {
      *p++ = 'F';
      if (key > F9Key) *p++ = (key-F0Key)/10+'0';
      *p++ = (key-F0Key)%10 + '0';
      *p = 0;
      return buf;
    }
    if (key >= Keypad && key <= KeypadLast) {
      // mark keypad keys with Keypad prefix
      strcpy(p,"Keypad"); p += 6;
      *p++ = uchar(key & 127);
      *p = 0;
      return buf;
    }
#endif
  }
  if (q) {
    if (p == buf) return q;
    strcpy(p, q);
    return buf;
  }
  // if all else fails use the keysym as a character:
  *p++ = uchar(key);
  *p = 0;
  return buf;
}

/*!
  Turn a string into a fltk::event_key() value or'd with
  fltk::event_shift() flags. The returned value can be used by by
  fltk::Widget::add_shortcut().  Any error, or a null or zero-length
  string, returns 0.

  Currently this understands prefixes of "Alt+", "Shift+", and "Ctrl+"
  to turn on fltk::ALT, fltk::SHIFT, and fltk::CTRL. Case is ignored
  and the '+' can be a '-' instead and the prefixes can be in any
  order.  You can also use '#' instead of "Alt+", '+' instead of
  "Shift+", and '^' instead of Ctrl+.

  After the shift prefixes there can either be a single ASCII letter,
  "Fn" where n is a number to indicate a function key, or "0xnnnn"
  to get an arbitrary fltk::event_key() enumeration value.

  The inverse function to turn a number into a string is fltk::key_name().
  Currently this function does not parse some strings fltk::key_name() can
  return, such as the names of arrow keys!
*/
unsigned fltk::key(const char* name) {
  if (!name || !*name) return 0;
  // read all the shift prefixes:
  int shifts = 0;
  while (name[0] && name[1]) {
    if (*name == '#') {
      shifts |= fltk::ALT; name++;
    } else if (*name == '+') {
      shifts |= fltk::SHIFT; name++;
    } else if (*name == '^') {
      shifts |= fltk::COMMAND; name++; // ctrl on win/linux, meta on os/x
    } else if (!strncasecmp(name, "alt",  3) && (name[3]=='-'||name[3]=='+')) {
      shifts |= fltk::ALT; name += 4;
    } else if (!strncasecmp(name, "shift",5) && (name[5]=='-'||name[5]=='+')) {
      shifts |= fltk::SHIFT; name += 6;
    } else if (!strncasecmp(name, "ctrl", 4) && (name[4]=='-'||name[4]=='+')) {
      shifts |= fltk::CTRL; name += 5;
    } else if (!strncasecmp(name, "meta", 4) && (name[4]=='-'||name[4]=='+')) {
      shifts |= fltk::META; name += 5;
//     } else if (!strncasecmp(name,"accelerator",11)&& (name[11]=='-'||name[11]=='+')) {
//       shifts |= fltk::ACCELERATOR; name += 12;
    } else if (!strncasecmp(name, "cmd", 3) && (name[3]=='-'||name[3]=='+')) {
      shifts |= fltk::COMMAND; name += 4; // ctrl on win/linux, meta on os/x
    } else break;
  }
  if (!*name) return 0;
  if (name[1]) {
    if (name[0]=='F' || name[0]=='f')
      return shifts | (fltk::F0Key+(int)strtol(name+1,0,0));
    return shifts | (int)strtol(name,0,0); // allow 0xf00 to get any key
  }
  return shifts | *name;
}

//
// End of "$Id: key_name.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
