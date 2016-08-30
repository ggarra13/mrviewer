//
// "$Id: events.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Event types and data. A Widget::handle() method needs this.
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

#ifndef fltk_events_h
#define fltk_events_h

#include "FL_API.h"

namespace fltk {

/// \name fltk/events.h
//@{

/*! Numbers passed to Widget::handle() and returned by event(). */
enum {
  NO_EVENT	= 0,
  PUSH		= 1,
  RELEASE	= 2,
  ENTER		= 3,
  LEAVE		= 4,
  DRAG		= 5,
  FOCUS		= 6,
  UNFOCUS	= 7,
  KEY		= 8,
  KEYUP		= 9,
  FOCUS_CHANGE	= 10,
  MOVE		= 11,
  SHORTCUT	= 12,
  DEACTIVATE	= 13,
  ACTIVATE	= 14,
  HIDE		= 15,
  SHOW		= 16,
  PASTE		= 17,
  TIMEOUT	= 18,
  MOUSEWHEEL	= 19,
  DND_ENTER	= 20,
  DND_DRAG	= 21,
  DND_LEAVE	= 22,
  DND_RELEASE	= 23,
  TOOLTIP	= 24
};

/*! Values returned by event_key(), passed to event_key_state() and
  get_key_state(), and used for the low 16 bits of add_shortcut().

  The actual values returned are based on X11 keysym values, though
  fltk always returns "unshifted" values much like Windows does. A
  given key always returns the same value no matter what shift keys
  are held down. Use event_text() to see the results of any shift
  keys.

  The lowercase letters 'a' through 'z' and the ascii symbols
  '`', '-', '=', '[', ']', '\\', ',', '.', '/', ';', '\'' and space
  are used to identify the keys in the main keyboard.

  On X systems unrecognized keys are returned unchanged as their
  X keysym value. If they have no keysym it uses the scan code
  or'd with 0x8000, this is what all those blue buttons on a
  Microsoft keyboard will do. I don't know how to get those
  buttons on Windows.
*/
enum {
  LeftButton	= 1,		/*!< PUSH/RELEASE sets event_key to this */
  MiddleButton	= 2,		/*!< PUSH/RELEASE sets event_key to this */
  RightButton	= 3,		/*!< PUSH/RELEASE sets event_key to this */
  WheelUp	= 4,		/*!< MOUSEWHEEL sets event_key to this */
  WheelDown	= 5,		/*!< MOUSEWHEEL sets event_key to this */
  WheelLeft	= 6,		/*!< MOUSEWHEEL sets event_key to this */
  WheelRight	= 7,		/*!< MOUSEWHEEL sets event_key to this */
  SpaceKey	= 32,		/*!< Same as ' ' or 32 */
  // 'a'-'z', and all punctuation go here in numerical order
  BackSpaceKey	= 0xff08,	/*!< Backspace */
  TabKey	= 0xff09,	/*!< Tab */
  ClearKey	= 0xff0b,	/*!< On some systems with NumLock off '5' produces this */
  ReturnKey	= 0xff0d,	/*!< Main Enter key, Windows and X documentation calls this "Return" */
  PauseKey	= 0xff13,	/*!< Pause + Break button */
  ScrollLockKey	= 0xff14,	/*!< Scroll Lock button */
  EscapeKey	= 0xff1b,	/*!< Esc */
  HomeKey	= 0xff50,	/*!< Home */
  LeftKey	= 0xff51,	/*!< Left */
  UpKey		= 0xff52,	/*!< Up arrow */
  RightKey	= 0xff53,	/*!< Right arrow */
  DownKey	= 0xff54,	/*!< Down arrow */
  PageUpKey	= 0xff55,	/*!< Page Up */
  PageDownKey	= 0xff56,	/*!< Page Down */
  EndKey	= 0xff57,	/*!< End */
  PrintKey	= 0xff61,	/*!< Print Scrn key + SysRq key */
  InsertKey	= 0xff63,	/*!< Insert */
  MenuKey	= 0xff67,	/*!< Key in lower-right with picture of popup menu */
  HelpKey	= 0xff68,	/*!< Help key on Macintosh keyboards */
  NumLockKey	= 0xff7f,	/*!< NumLock key */
  Keypad	= 0xff80,	/*!< Add ASCII to get keypad keys */
  KeypadEnter	= Keypad+'\r',	/*!< Keypad+'\\r' */
  MultiplyKey	= Keypad+'*',	/*!< Keypad+'*' */
  AddKey	= Keypad+'+',	/*!< Keypad+'+' */
  SubtractKey	= Keypad+'-',	/*!< Keypad+'-' */
  DecimalKey	= Keypad+'.',	/*!< Keypad+'.' */
  DivideKey	= Keypad+'/',	/*!< Keypad+'/' */
  Keypad0	= Keypad+'0',	/*!< Keypad+'0' */
  Keypad1	= Keypad+'1',	/*!< Keypad+'1' */
  Keypad2	= Keypad+'2',	/*!< Keypad+'2' */
  Keypad3	= Keypad+'3',	/*!< Keypad+'3' */
  Keypad4	= Keypad+'4',	/*!< Keypad+'4' */
  Keypad5	= Keypad+'5',	/*!< Keypad+'5' */
  Keypad6	= Keypad+'6',	/*!< Keypad+'6' */
  Keypad7	= Keypad+'7',	/*!< Keypad+'7' */
  Keypad8	= Keypad+'8',	/*!< Keypad+'8' */
  Keypad9	= Keypad+'9',	/*!< Keypad+'9' */
  KeypadLast	= 0xffbd,	/*!< Keypad+'=', largest legal keypad key */
  F0Key		= 0xffbd,	/*!< Add a number to get function key */
  F1Key		= F0Key+1,	/*!< F0Key+1 */
  F2Key		= F0Key+2,	/*!< F0Key+2 */
  F3Key		= F0Key+3,	/*!< F0Key+3 */
  F4Key		= F0Key+4,	/*!< F0Key+4 */
  F5Key		= F0Key+5,	/*!< F0Key+5 */
  F6Key		= F0Key+6,	/*!< F0Key+6 */
  F7Key		= F0Key+7,	/*!< F0Key+7 */
  F8Key		= F0Key+8,	/*!< F0Key+8 */
  F9Key		= F0Key+9,	/*!< F0Key+9 */
  F10Key	= F0Key+10,	/*!< F0Key+10 */
  F11Key	= F0Key+11,	/*!< F0Key+11 */
  F12Key	= F0Key+12,	/*!< F0Key+12 */
  // use F0Key+n to get function key n
  LastFunctionKey = F0Key+35,	/*!< F0Key+35, largest legal function key */
  LeftShiftKey	= 0xffe1,	/*!< Left-hand Shift */
  RightShiftKey	= 0xffe2,	/*!< Right-hand Shift */
  LeftCtrlKey	= 0xffe3,	/*!< Left-hand Ctrl */
  RightCtrlKey	= 0xffe4,	/*!< Right-hand Ctrl */
  CapsLockKey	= 0xffe5,	/*!< Caps Lock */
  LeftMetaKey	= 0xffe7,	/*!< The left "Windows" or "Apple" key */
  RightMetaKey	= 0xffe8,	/*!< The right "Windows" or "Apple" key */
  LeftAltKey	= 0xffe9,	/*!< Left-hand Alt (option on Mac) */
  RightAltKey	= 0xffea,	/*!< Right-hand Alt (option on Mac) */
  DeleteKey	= 0xffff,	/*!< Delete */
#if  defined(__APPLE__)
  LeftAccKey	= LeftCtrlKey,
  RightAccKey	= RightCtrlKey,
  LeftCmdKey	= LeftMetaKey,
  RightCmdKey	= RightMetaKey
#else
  LeftAccKey	= LeftAltKey,
  RightAccKey	= RightAltKey,
  LeftCmdKey	= LeftCtrlKey,
  RightCmdKey	= RightCtrlKey
#endif
};

/*! Flags returned by event_state(), and used as the high 16 bits
  of Widget::add_shortcut() values (the low 16 bits are all zero, so these
  may be or'd with key values).

  The inline function BUTTON(n) will turn n (1-8) into the flag for a
  mouse button.
*/
enum {
  SHIFT		= 0x00010000,	/*!< Either shift key held down */
  CAPSLOCK	= 0x00020000,	/*!< Caps lock is toggled on */
  CTRL		= 0x00040000,	/*!< Either ctrl key held down */
  ALT		= 0x00080000,	/*!< Either alt key held down */
  NUMLOCK	= 0x00100000,	/*!< Num Lock turned on */
  META		= 0x00400000,	/*!< "Windows" or the "Apple" keys held down */
  SCROLLLOCK	= 0x00800000,	/*!< Scroll Lock turned on */
  BUTTON1	= 0x01000000,	/*!< Left mouse button held down */
  BUTTON2	= 0x02000000,	/*!< Middle mouse button held down */
  BUTTON3	= 0x04000000,	/*!< Right mouse button held down */
  BUTTON4	= 0x08000000,	/*!< Mouse wheel scroll up */
  BUTTON5	= 0x10000000,	/*!< Mouse wheel scroll down */
  BUTTON6	= 0x20000000,	/*!< Mouse wheel scroll left */
  BUTTON7	= 0x40000000,	/*!< Mouse wheel scroll right */
  ANY_BUTTON	= 0xff000000, /*!< Any mouse button (up to 8) */
#if defined(__APPLE__)
  ACCELERATOR	= CTRL,
  OPTION	= ALT,
  COMMAND	= META
#else
  ACCELERATOR	= ALT, //!< ALT on Windows/Linux, CTRL on OS/X, use for menu accelerators
  COMMAND	= CTRL,	//!< CTRL on Windows/Linux, META on OS/X, use for menu shortcuts
  OPTION	= ALT|META //!< ALT|META on Windows/Linux, just ALT on OS/X, use as a drag modifier
#endif
};

inline unsigned BUTTON(int n) {return 0x00800000 << n;}

/*! Device identifier returned by event_device(). This enumeration
  is useful to get the device type that caused a PUSH, RELEASE,
  DRAG or MOVE event 
*/
enum {
  DEVICE_MOUSE    = 0, /*!< Event triggered by the system mouse */
  DEVICE_STYLUS   = 1, /*!< Event triggered by a pen on a tablet, givin pressure and tilt information */
  DEVICE_ERASER   = 2, /*!< Event triggered by an eraser on a tablet, givin pressure and tilt information */
  DEVICE_CURSOR   = 3, /*!< Event triggered by a puck style device on a tablet */
  DEVICE_AIRBRUSH = 4, /*!< Event triggered by an airbrush on a tablet, giving pressure and tilt information */
  DEVICE_TOUCH    = 5  /*!< Event triggered by touch a touch screen device */
};

class Rectangle;
class Widget;
class Window;

#ifndef DOXYGEN

////////////////////////////////////////////////////////////////
// Do not use these variables, they may not exist in future versions:

extern FL_API int e_type;
extern FL_API int e_x;
extern FL_API int e_y;
extern FL_API int e_dx;
extern FL_API int e_dy;
extern FL_API int e_x_root;
extern FL_API int e_y_root;
extern FL_API unsigned e_state;
extern FL_API int e_clicks;
extern FL_API unsigned e_is_click;
extern FL_API unsigned e_keysym;
extern FL_API unsigned e_length;
extern FL_API const char* e_text;
extern FL_API unsigned e_key_repeated;
extern FL_API float e_pressure;
extern FL_API float e_x_tilt;
extern FL_API float e_y_tilt;
extern FL_API int e_device;
extern FL_API int compose_state;
extern FL_API Widget* belowmouse_;
extern FL_API Widget* pushed_;
extern FL_API Widget* focus_;
extern FL_API Widget* modal_;
extern FL_API bool grab_;
extern FL_API bool exit_modal_;

////////////////////////////////////////////////////////////////
#endif

inline int  event()			{return e_type;}
inline int  event_x()			{return e_x;}
inline int  event_y()			{return e_y;}
inline int  event_dx()			{return e_dx;}
inline int  event_dy()			{return e_dy;}
inline int  event_x_root()		{return e_x_root;}
inline int  event_y_root()		{return e_y_root;}
inline int  event_clicks()		{return e_clicks;}
inline void event_clicks(int i)		{e_clicks = i;}
inline bool event_is_click()		{return e_is_click != 0;}
inline void event_is_click(bool)	{e_is_click = 0;} // only false works!
inline unsigned event_state()		{return e_state;}
inline bool event_state(unsigned i)	{return (e_state&i) != 0;}
inline unsigned event_key()		{return e_keysym;}
inline unsigned event_button()		{return e_keysym;}
FL_API bool event_key_state(unsigned);
inline const char* event_text() 	{return e_text;}
inline unsigned event_length() 		{return e_length;}
inline unsigned event_key_repeated()	{return e_key_repeated;}
inline float event_pressure()	 	{return e_pressure;}
inline float event_x_tilt()     	{return e_x_tilt;}
inline float event_y_tilt()     	{return e_y_tilt;}
inline int  event_device()      	{return e_device;}

// tests on current event:
FL_API bool event_inside(const Rectangle&);
FL_API bool compose(int &del);
inline void compose_reset()		{compose_state = 0;}

// shortcuts:
FL_API bool try_shortcut();
FL_API const char* key_name(unsigned key);
FL_API unsigned key(const char* name);

class FL_API ShortcutFunctor {
 public:
  virtual bool handle(const Widget*, unsigned key) = 0;
};
FL_API unsigned foreachShortcut(const Widget*, ShortcutFunctor&);
inline unsigned foreachShortcut(ShortcutFunctor& f) { return foreachShortcut(0,f); }

// get current information, not info from last event:
FL_API bool get_key_state(unsigned);
FL_API void get_mouse(int &,int &);
FL_API bool warp_mouse(int, int);

// event destinations:
FL_API bool handle(int, Window*);
FL_API void add_event_handler(int (*h)(int, Window*));
inline Widget* belowmouse() 		{return belowmouse_;}
FL_API void belowmouse(Widget*);
inline void belowmouse(Widget& w)	{belowmouse(&w);}
inline Widget* pushed()			{return pushed_;}
FL_API void pushed(Widget*);
inline void pushed(Widget& w)		{pushed(&w);}
inline Widget* focus()			{return focus_;}
FL_API void focus(Widget*);
inline void focus(Widget& w)		{focus(&w);}

// cut/paste/dnd:
FL_API void copy(const char* stuff, int len, bool clipboard = false);
FL_API void paste(Widget &receiver, bool clipboard = false);
FL_API bool dnd();

// Modal widgets (block events going to any other widgets):
FL_API void modal(Widget*, bool grab = false);
inline Widget* modal()			{return modal_;}
inline bool grab()			{return grab_;}
inline void exit_modal()		{exit_modal_ = true;}
inline bool exit_modal_flag()		{return exit_modal_;}

// for debugging purpose : 
const char  *event_name(int event); /// see STR #508

//@}

}

#endif

//
// $Id: events.h 8500 2011-03-03 09:20:46Z bgbnbigben $
//
