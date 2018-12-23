//
// "$Id: Enumerations.H 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Enumerations for the Fast Light Tool Kit (FLTK).
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

#ifndef Fl_Enumerations_H
#define Fl_Enumerations_H

# include <fltk/FL_API.h>
# include <fltk/FL_VERSION.h>

enum Fl_Event {	// these should match fltk/events.h
  FL_NO_EVENT		= 0,
  FL_PUSH		= 1,
  FL_RELEASE		= 2,
  FL_ENTER		= 3,
  FL_LEAVE		= 4,
  FL_DRAG		= 5,
  FL_FOCUS		= 6,
  FL_UNFOCUS		= 7,
  FL_KEYDOWN		= 8,
  FL_KEYBOARD		= FL_KEYDOWN,
  FL_KEYUP		= 9,
  //FL_CLOSE		= 10, // not in fltk2.0
  FL_MOVE		= 11,
  FL_SHORTCUT		= 12,
  FL_DEACTIVATE		= 13,
  FL_ACTIVATE		= 14,
  FL_HIDE		= 15,
  FL_SHOW		= 16,
  FL_PASTE		= 17,
  //FL_SELECTIONCLEAR	= 18, // not in fltk2.0
  FL_MOUSEWHEEL		= 19,
  FL_DND_ENTER		= 20,
  FL_DND_DRAG		= 21,
  FL_DND_LEAVE		= 22,
  FL_DND_RELEASE	= 23
};

enum Fl_When { // These should match values in fltk/Widget.h
  FL_WHEN_NEVER		= 0,
  FL_WHEN_CHANGED	= 1,
  FL_WHEN_RELEASE	= 4,
  FL_WHEN_RELEASE_ALWAYS= 6,
  FL_WHEN_ENTER_KEY	= 8,
  FL_WHEN_ENTER_KEY_ALWAYS=10,
  FL_WHEN_ENTER_KEY_CHANGED=11,
  FL_WHEN_NOT_CHANGED	= 2 // modifier bit to disable changed() test
};

// These should match symbols in fltk/events.h:
#define FL_Button	0 // old value was 0xfee8
#define FL_LEFT_MOUSE	1
#define FL_MIDDLE_MOUSE	2
#define FL_RIGHT_MOUSE	3
#define FL_BackSpace	0xff08
#define FL_Tab		0xff09
#define FL_Enter	0xff0d
#define FL_Pause	0xff13
#define FL_Scroll_Lock	0xff14
#define FL_Escape	0xff1b
#define FL_Home		0xff50
#define FL_Left		0xff51
#define FL_Up		0xff52
#define FL_Right	0xff53
#define FL_Down		0xff54
#define FL_Page_Up	0xff55
#define FL_Page_Down	0xff56
#define FL_End		0xff57
#define FL_Print	0xff61
#define FL_Insert	0xff63
#define FL_Menu		0xff67 // the "menu/apps" key on XFree86
#define FL_Help		0xff68 // the 'help' key on Mac keyboards
#define FL_Num_Lock	0xff7f
#define FL_KP		0xff80 // use FL_KP+'x' for 'x' on numeric keypad
#define FL_KP_Enter	0xff8d // same as Fl_KP+'\r'
#define FL_KP_Last	0xffbd // use to range-check keypad
#define FL_F		0xffbd // use FL_F+n for function key n
#define FL_F_Last	0xffe0 // use to range-check function keys
#define FL_Shift_L	0xffe1
#define FL_Shift_R	0xffe2
#define FL_Control_L	0xffe3
#define FL_Control_R	0xffe4
#define FL_Caps_Lock	0xffe5
#define FL_Meta_L	0xffe7
#define FL_Meta_R	0xffe8
#define FL_Alt_L	0xffe9
#define FL_Alt_R	0xffea
#define FL_Delete	0xffff

#define FL_SHIFT	0x00010000
#define FL_CAPS_LOCK	0x00020000
#define FL_CTRL		0x00040000
#define FL_ALT		0x00080000
#define FL_NUM_LOCK	0x00100000 // most X servers do this?
#define FL_META		0x00400000
#define FL_SCROLL_LOCK	0x00800000
#define FL_BUTTON1	0x01000000
#define FL_BUTTON2	0x02000000
#define FL_BUTTON3	0x04000000
#define FL_BUTTONS	0x7f000000 // All possible buttons
#define FL_BUTTON(n)	(0x00800000<<(n))

#ifdef __APPLE__
#  define FL_COMMAND	FL_META
#else
#  define FL_COMMAND	FL_CTRL
#endif // __APPLE__

#include <fltk/Style.h>

typedef fltk::Box* Fl_Boxtype;

// Box types. Many of the FRAME types are deleted in fltk2.0, these have
// been replaced with the equivalent BOX which draws it's inner area:
#define FL_NO_BOX		fltk::NO_BOX
#define FL_FLAT_BOX		fltk::FLAT_BOX
#define FL_UP_BOX		fltk::UP_BOX
#define FL_DOWN_BOX		fltk::DOWN_BOX
#define FL_UP_FRAME		fltk::UP_BOX //
#define FL_DOWN_FRAME		fltk::DOWN_BOX //
#define FL_THIN_UP_BOX		fltk::THIN_UP_BOX
#define FL_THIN_DOWN_BOX	fltk::THIN_DOWN_BOX
#define FL_THIN_UP_FRAME	fltk::THIN_UP_BOX //
#define FL_THIN_DOWN_FRAME	fltk::THIN_DOWN_BOX //
#define FL_ENGRAVED_BOX		fltk::ENGRAVED_BOX
#define FL_EMBOSSED_BOX		fltk::EMBOSSED_BOX
#define FL_ENGRAVED_FRAME	fltk::ENGRAVED_BOX //
#define FL_EMBOSSED_FRAME	fltk::EMBOSSED_BOX //
#define FL_BORDER_BOX		fltk::BORDER_BOX
#define FL_SHADOW_BOX		fltk::SHADOW_BOX
#define FL_BORDER_FRAME		fltk::BORDER_FRAME
#define FL_SHADOW_FRAME		fltk::SHADOW_BOX //
#define FL_ROUNDED_BOX		fltk::ROUNDED_BOX
#define FL_RSHADOW_BOX		fltk::RSHADOW_BOX
#define FL_ROUNDED_FRAME	fltk::ROUNDED_BOX //
#define FL_RFLAT_BOX		fltk::RFLAT_BOX
#define FL_ROUND_UP_BOX		fltk::ROUND_UP_BOX
#define FL_ROUND_DOWN_BOX	fltk::ROUND_DOWN_BOX
#define FL_DIAMOND_UP_BOX	fltk::DIAMOND_UP_BOX
#define FL_DIAMOND_DOWN_BOX	fltk::DIAMOND_DOWN_BOX
#define FL_OVAL_BOX		fltk::OVAL_BOX
#define FL_OSHADOW_BOX		fltk::OSHADOW_BOX
#define FL_OVAL_FRAME		fltk::OVAL_BOX //
#define FL_OFLAT_BOX		fltk::OFLAT_BOX
#define FL_PLASTIC_UP_BOX	fltk::PLASTIC_UP_BOX
#define FL_PLASTIC_DOWN_BOX	fltk::PLASTIC_DOWN_BOX
#define FL_PLASTIC_UP_FRAME	fltk::PLASTIC_UP_BOX //
#define FL_PLASTIC_DOWN_FRAME	fltk::PLASTIC_DOWN_BOX //
//#define FL_FREE_BOXTYPE not used in fltk 2.0

// conversions of box types to other boxtypes:
//inline Fl_Boxtype fl_down(Fl_Boxtype b) // not implemented
//inline Fl_Boxtype fl_frame(Fl_Boxtype b)

// back-compatability box types:
#define FL_FRAME		FL_ENGRAVED_FRAME
#define FL_FRAME_BOX		FL_ENGRAVED_BOX
#define FL_CIRCLE_BOX		FL_ROUND_DOWN_BOX
#define FL_DIAMOND_BOX		FL_DIAMOND_DOWN_BOX

// labeltypes:
#define FL_NO_LABEL		fltk::NO_LABEL
#define FL_NORMAL_LABEL		fltk::NORMAL_LABEL
#define FL_SHADOW_LABEL		fltk::SHADOW_LABEL
#define FL_ENGRAVED_LABEL	fltk::ENGRAVED_LABEL
#define FL_EMBOSSED_LABEL	fltk::EMBOSSED_LABEL
//#define FL_FREE_LABELTYPE not used in fltk2.0
#define FL_SYMBOL_LABEL		FL_NORMAL_LABEL

#define Fl_Align		fltk::Flags
enum {
  FL_ALIGN_CENTER		= 0,
  FL_ALIGN_TOP			= fltk::ALIGN_TOP,
  FL_ALIGN_BOTTOM		= fltk::ALIGN_BOTTOM,
  FL_ALIGN_LEFT			= fltk::ALIGN_LEFT,
  FL_ALIGN_RIGHT		= fltk::ALIGN_RIGHT,
  FL_ALIGN_INSIDE		= fltk::ALIGN_INSIDE,
  FL_ALIGN_TEXT_OVER_IMAGE	= fltk::ALIGN_BOTTOM|fltk::ALIGN_INSIDE, //
  FL_ALIGN_IMAGE_OVER_TEXT	= 0,
  FL_ALIGN_CLIP			= fltk::ALIGN_CLIP,
  FL_ALIGN_WRAP			= fltk::ALIGN_WRAP,
  FL_ALIGN_MASK			= fltk::ALIGN_MASK,
  FL_ALIGN_TOP_LEFT		= FL_ALIGN_TOP | FL_ALIGN_LEFT,
  FL_ALIGN_TOP_RIGHT		= FL_ALIGN_TOP | FL_ALIGN_RIGHT,
  FL_ALIGN_BOTTOM_LEFT		= FL_ALIGN_BOTTOM | FL_ALIGN_LEFT,
  FL_ALIGN_BOTTOM_RIGHT		= FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT,
  FL_ALIGN_LEFT_TOP		= FL_ALIGN_TOP_LEFT,
  FL_ALIGN_RIGHT_TOP		= FL_ALIGN_TOP_RIGHT,
  FL_ALIGN_LEFT_BOTTOM		= FL_ALIGN_BOTTOM_LEFT,
  FL_ALIGN_RIGHT_BOTTOM		= FL_ALIGN_BOTTOM_RIGHT,
  FL_ALIGN_NOWRAP		= 0
};

#define FL_HELVETICA		fltk::HELVETICA
#define FL_HELVETICA_BOLD	fltk::HELVETICA_BOLD
#define FL_HELVETICA_ITALIC	fltk::HELVETICA_ITALIC
#define FL_HELVETICA_BOLD_ITALIC fltk::HELVETICA_BOLD_ITALIC
#define FL_COURIER		fltk::COURIER
#define FL_COURIER_BOLD		fltk::COURIER_BOLD
#define FL_COURIER_ITALIC	fltk::COURIER_ITALIC
#define FL_COURIER_BOLD_ITALIC	fltk::COURIER_BOLD_ITALIC
#define FL_TIMES		fltk::TIMES
#define FL_TIMES_BOLD		fltk::TIMES_BOLD
#define FL_TIMES_ITALIC		fltk::TIMES_ITALIC
#define FL_TIMES_BOLD_ITALIC	fltk::TIMES_BOLD_ITALIC
#define FL_SYMBOL		fltk::SYMBOL_FONT
#define FL_SCREEN		fltk::SCREEN_FONT
#define FL_SCREEN_BOLD		fltk::SCREEN_BOLD_FONT
#define FL_ZAPF_DINGBATS	fltk::ZAPF_DINGBATS
//#define FL_FREE_FONT
// These used to be added to the above fonts but were sometimes used alone
// because FL_HELVETICA was zero. Only using these by themselves works now:
#define FL_BOLD			fltk::HELVETICA_BOLD
#define FL_ITALIC		fltk::HELVETICA_ITALIC

#define FL_NORMAL_SIZE (fltk::Widget::default_style->labelsize_)
#define FL_FOREGROUND_COLOR (fltk::Widget::default_style->labelcolor_)
#define FL_BACKGROUND2_COLOR (fltk::Widget::default_style->color_)
#define FL_SELECTION_COLOR (fltk::Widget::default_style->selection_color_)

#define Fl_Color	fltk::Color
enum {
  FL_GRAY0		= fltk::GRAY00,
  FL_DARK3		= fltk::GRAY33,
  FL_DARK2		= fltk::GRAY60,
  FL_DARK1		= fltk::GRAY66,
  FL_GRAY		= fltk::GRAY75,
  FL_BACKGROUND_COLOR	= fltk::GRAY75,
  FL_LIGHT1		= fltk::GRAY85,
  FL_LIGHT2		= fltk::GRAY90,
  FL_LIGHT3		= fltk::GRAY99,

  FL_BLACK		= fltk::BLACK,
  FL_RED		= fltk::RED,
  FL_GREEN		= fltk::GREEN,
  FL_YELLOW		= fltk::YELLOW,
  FL_BLUE		= fltk::BLUE,
  FL_MAGENTA		= fltk::MAGENTA,
  FL_CYAN		= fltk::CYAN,
  FL_DARK_RED		= 72,

  FL_DARK_GREEN		= 60,
  FL_DARK_YELLOW	= 76,
  FL_DARK_BLUE		= 136,
  FL_DARK_MAGENTA	= 152,
  FL_DARK_CYAN		= 140,

  FL_WHITE		= fltk::WHITE,

  FL_FREE_COLOR		= 16,
  FL_NUM_FREE_COLOR	= 16,
  FL_GRAY_RAMP		= fltk::GRAY00,
  FL_NUM_GRAY		= 24,
  FL_COLOR_CUBE		= fltk::BLACK,
  FL_NUM_RED		= 5,
  FL_NUM_GREEN		= 8,
  FL_NUM_BLUE		= 5
};

#define fl_inactive(c)		fltk::inactive(c)
#define fl_contrast(a,b)	fltk::contrast(a,b)
#define fl_color_average(a,b,c)	fltk::lerp(b,a,c)
#define fl_lighter(c)		fltk::lerp(fltk::WHITE,c,.67f)
#define fl_darker(c)		fltk::lerp(fltk::BLACK,c,.67f)
#define fl_rgb_color		fltk::color
#define fl_gray_ramp(i)		(fltk::GRAY00+(i))
#define fl_color_cube(r,g,b)	(((b)*5+(r))*8+(g)+fltk::BLACK)

#include <fltk/Cursor.h>
#define Fl_Cursor		fltk::Cursor*
#define FL_CURSOR_DEFAULT	fltk::CURSOR_DEFAULT
#define FL_CURSOR_ARROW		fltk::CURSOR_ARROW
#define FL_CURSOR_CROSS		fltk::CURSOR_CROSS
#define FL_CURSOR_WAIT		fltk::CURSOR_WAIT
#define FL_CURSOR_INSERT	fltk::CURSOR_INSERT
#define FL_CURSOR_HAND		fltk::CURSOR_HAND
#define FL_CURSOR_HELP		fltk::CURSOR_HELP
#define FL_CURSOR_MOVE		fltk::CURSOR_MOVE
#define FL_CURSOR_NS		fltk::CURSOR_NS
#define FL_CURSOR_WE		fltk::CURSOR_WE
#define FL_CURSOR_NWSE		fltk::CURSOR_NWSE
#define FL_CURSOR_NESW		fltk::CURSOR_NESW
#define FL_CURSOR_NO		fltk::CURSOR_NO
#define FL_CURSOR_NONE		fltk::CURSOR_NONE
#define FL_CURSOR_N		fltk::CURSOR_NS
#define FL_CURSOR_NE		fltk::CURSOR_NESW
#define FL_CURSOR_E		fltk::CURSOR_WE
#define FL_CURSOR_SE		fltk::CURSOR_NWSE
#define FL_CURSOR_S		fltk::CURSOR_NS
#define FL_CURSOR_SW		fltk::CURSOR_NESW
#define FL_CURSOR_W		fltk::CURSOR_WE
#define FL_CURSOR_NW		fltk::CURSOR_NWSE

enum { // must match values in fltk/run.h
  FL_READ = 1,
  FL_WRITE = 4,
  FL_EXCEPT = 8
};

enum Fl_Mode { // must match values in fltk/visual.h
  FL_RGB	= 0,
  FL_INDEX	= 1,
  FL_SINGLE	= 0,
  FL_DOUBLE	= 2,
  FL_ACCUM	= 4,
  FL_ALPHA	= 8,
  FL_DEPTH	= 16,
  FL_STENCIL	= 32,
  FL_RGB8	= 64,
  FL_MULTISAMPLE= 128,
  FL_STEREO     = 256,
  FL_FAKE_SINGLE = 512	// Fake single buffered windows using double-buffer
};

#include <fltk/damage.h>
enum Fl_Damage {
  FL_DAMAGE_CHILD    = fltk::DAMAGE_CHILD,
  FL_DAMAGE_EXPOSE   = fltk::DAMAGE_EXPOSE,
  FL_DAMAGE_SCROLL   = fltk::DAMAGE_SCROLL,
  FL_DAMAGE_OVERLAY  = fltk::DAMAGE_OVERLAY,
  FL_DAMAGE_ALL      = fltk::DAMAGE_ALL
};

// FLTK 1.0.x compatibility definitions...
#  ifdef FLTK_1_0_COMPAT
#    define contrast	fl_contrast
#    define down	fl_down
#    define frame	fl_frame
#    define inactive	fl_inactive
#  endif // FLTK_1_0_COMPAT

#endif

//
// End of "$Id: Enumerations.H 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
