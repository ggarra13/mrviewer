
// Return the current state of a key.  This is the X version.  I identify
// keys (mostly) by the X keysym.  So this turns the keysym into a keycode
// and looks it up in the X key bit vector, which x.C keeps track of.

#include <fltk/events.h>
#include <fltk/x.h>

extern char fl_key_vector[32]; // in x.C

bool fltk::event_key_state(unsigned keysym) {
  if (keysym > 0 && keysym <= 8)
    return event_state(BUTTON(keysym)) != 0;
  int keycode;
  // Newer X servers have somehow mapped both Meta and Alt to the Alt
  // key, and put Super on the Windows key. Detect and work around this:
  // Even newer X servers are further screwed up. What the hell are
  // they thinking??
  if (keysym == LeftMetaKey) {
    keycode = XKeysymToKeycode(xdisplay, 0xffeb); // try XK_Super_L
    if (keycode) goto DONE;
    keycode = XKeysymToKeycode(xdisplay, F0Key+13); // try F13 (!)
    if (keycode && !XKeysymToKeycode(xdisplay, F0Key+14)) goto DONE;
  } else if (keysym == RightMetaKey) {
    keycode = XKeysymToKeycode(xdisplay, 0xffec); // try XK_Super_R
    if (keycode) goto DONE;
    keycode = XKeysymToKeycode(xdisplay, 0xff20); // try XK_Multi_key
    if (keycode) goto DONE;
  } else if (keysym == RightAltKey) {
#if defined(__linux) || defined(__FreeBSD__)
    keycode = XKeysymToKeycode(xdisplay, 0xfe03); // AltGr european iso
    if (keycode) goto DONE;
#endif
    keycode = XKeysymToKeycode(xdisplay, 0xff7e); // try XK_Mode_switch
    if (keycode) goto DONE;
  }    
  keycode = XKeysymToKeycode(xdisplay, keysym);
  if (!keycode) {
#ifdef __sgi
    // get some missing PC keyboard keys:
    if (keysym == LeftMetaKey) keycode = 147;
    else if (keysym == RightMetaKey) keycode = 148;
    else if (keysym == MenuKey) keycode = 149;
    else
#endif
      keycode = keysym & 0xff; // undo the |0x8000 done to unknown keycodes
  }
 DONE:
  return (fl_key_vector[keycode/8] & (1 << (keycode%8))) != 0;
}

bool fltk::get_key_state(unsigned key) {
  open_display();
  XQueryKeymap(xdisplay, fl_key_vector);
  return event_key_state(key);
}
