//
// "$Id: forms.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Forms emulation header file for the Fast Light Tool Kit (FLTK).
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

#ifndef __FORMS_H__
#define __FORMS_H__

#include <fltk/Fl.h>
#include <fltk/Fl_Group.h>
#include <fltk/Fl_Window.h>
#include <fltk/fl_draw.h>

typedef Fl_Widget FL_OBJECT;
typedef Fl_Window FL_FORM;

////////////////////////////////////////////////////////////////
// Random constants & symbols defined by forms.h file:

#ifndef NULL
#define NULL 0
#endif
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#define FL_ON		1
#define FL_OK		1
#define FL_VALID	1
#define FL_PREEMPT	1
#define FL_AUTO		2
#define FL_WHEN_NEEDED	FL_AUTO
#define FL_OFF		0
#define FL_NONE		0
#define FL_CANCEL	0
#define FL_INVALID	0
#define FL_IGNORE	-1
#define FL_CLOSE	-2

#define FL_LCOL		FL_BLACK
#define FL_COL1		FL_GRAY
#define FL_MCOL		FL_LIGHT1
#define FL_LEFT_BCOL	FL_LIGHT3 // 53 is better match
#define FL_TOP_BCOL	FL_LIGHT2 // 51
#define FL_BOTTOM_BCOL	FL_DARK2  // 40
#define FL_RIGHT_BCOL	FL_DARK3  // 36
#define FL_INACTIVE	FL_INACTIVE_COLOR
#define FL_INACTIVE_COL	FL_INACTIVE_COLOR
#define FL_FREE_COL1	FL_FREE_COLOR
#define FL_FREE_COL2	((Fl_Color)(FL_FREE_COLOR+1))
#define FL_FREE_COL3	((Fl_Color)(FL_FREE_COLOR+2))
#define FL_FREE_COL4	((Fl_Color)(FL_FREE_COLOR+3))
#define FL_FREE_COL5	((Fl_Color)(FL_FREE_COLOR+4))
#define FL_FREE_COL6	((Fl_Color)(FL_FREE_COLOR+5))
#define FL_FREE_COL7	((Fl_Color)(FL_FREE_COLOR+6))
#define FL_FREE_COL8	((Fl_Color)(FL_FREE_COLOR+7))
#define FL_FREE_COL9	((Fl_Color)(FL_FREE_COLOR+8))
#define FL_FREE_COL10	((Fl_Color)(FL_FREE_COLOR+9))
#define FL_FREE_COL11	((Fl_Color)(FL_FREE_COLOR+10))
#define FL_FREE_COL12	((Fl_Color)(FL_FREE_COLOR+11))
#define FL_FREE_COL13	((Fl_Color)(FL_FREE_COLOR+12))
#define FL_FREE_COL14	((Fl_Color)(FL_FREE_COLOR+13))
#define FL_FREE_COL15	((Fl_Color)(FL_FREE_COLOR+14))
#define FL_FREE_COL16	((Fl_Color)(FL_FREE_COLOR+15))
#define FL_TOMATO	((Fl_Color)(131))
#define FL_INDIANRED	((Fl_Color)(164))
#define FL_SLATEBLUE	((Fl_Color)(195))
#define FL_DARKGOLD	((Fl_Color)(84))
#define FL_PALEGREEN	((Fl_Color)(157))
#define FL_ORCHID	((Fl_Color)(203))
#define FL_DARKCYAN	((Fl_Color)(189))
#define FL_DARKTOMATO	((Fl_Color)(113))
#define FL_WHEAT	((Fl_Color)(174))

#define FL_ALIGN_BESIDE	FL_ALIGN_INSIDE

#define FL_PUP_TOGGLE	2 // FL_MENU_TOGGLE
#define FL_PUP_INACTIVE 1 // FL_MENU_INACTIVE
#define FL_NO_FRAME	FL_NO_BOX
#define FL_ROUNDED3D_UPBOX 	FL_ROUND_UP_BOX
#define FL_ROUNDED3D_DOWNBOX	FL_ROUND_DOWN_BOX
#define FL_OVAL3D_UPBOX		FL_ROUND_UP_BOX
#define FL_OVAL3D_DOWNBOX	FL_ROUND_DOWN_BOX

#define FL_MBUTTON1	1
#define FL_LEFTMOUSE	1
#define FL_MBUTTON2	2
#define FL_MIDDLEMOUSE	2
#define FL_MBUTTON3	3
#define FL_RIGHTMOUSE	3
#define FL_MBUTTON4	4
#define FL_MBUTTON5	5

#define FL_INVALID_STYLE 255
#define FL_NORMAL_STYLE	0
#define FL_BOLD_STYLE	1
#define FL_ITALIC_STYLE	2
#define FL_BOLDITALIC_STYLE 3
#define FL_FIXED_STYLE	4
#define FL_FIXEDBOLD_STYLE 5
#define FL_FIXEDITALIC_STYLE 6
#define FL_FIXEDBOLDITALIC_STYLE 7
#define FL_TIMES_STYLE	8
#define FL_TIMESBOLD_STYLE 9
#define FL_TIMESITALIC_STYLE 10
#define FL_TIMESBOLDITALIC_STYLE 11

// hacks to change the labeltype() when passed to fl_set_object_lstyle():
#define FL_SHADOW_STYLE		0x100
#define FL_ENGRAVED_STYLE	0x200
#define FL_EMBOSSED_STYLE	0x300

// size values are different from XForms, match older Forms:
#define FL_TINY_SIZE	8
#define FL_SMALL_SIZE	11 // 10
#undef FL_NORMAL_SIZE
#define FL_NORMAL_SIZE	14 // 12
#define FL_MEDIUM_SIZE	18 // 14
#define FL_LARGE_SIZE	24 // 18
#define FL_HUGE_SIZE	32 // 24
#define FL_DEFAULT_SIZE	FL_SMALL_SIZE
#define FL_TINY_FONT	FL_TINY_SIZE
#define FL_SMALL_FONT	FL_SMALL_SIZE
#define FL_NORMAL_FONT	FL_NORMAL_SIZE
#define FL_MEDIUM_FONT	FL_MEDIUM_SIZE
#define FL_LARGE_FONT	FL_LARGE_SIZE
#define FL_HUGE_FONT	FL_HUGE_SIZE
#define FL_NORMAL_FONT1	FL_SMALL_FONT
#define FL_NORMAL_FONT2	FL_NORMAL_FONT
#define FL_DEFAULT_FONT	FL_SMALL_FONT

#define FL_RETURN_END_CHANGED	FL_WHEN_RELEASE
#define FL_RETURN_CHANGED	FL_WHEN_CHANGED
#define FL_RETURN_END		FL_WHEN_RELEASE_ALWAYS
#define FL_RETURN_ALWAYS	(FL_WHEN_CHANGED|FL_WHEN_NOT_CHANGED)

#define FL_BOUND_WIDTH	3

typedef int FL_Coord;
typedef int FL_COLOR;

////////////////////////////////////////////////////////////////
// fltk interaction:

#define FL_CMD_OPT void
extern FL_FORMS_API void fl_initialize(int*, char*[], const char*, FL_CMD_OPT*, int);
inline void fl_finish() {}

typedef void (*FL_IO_CALLBACK) (int, void*);
inline void fl_add_io_callback(int fd, short w, FL_IO_CALLBACK cb, void* v) {
  Fl::add_fd(fd,w,cb,v);}
inline void fl_remove_io_callback(int fd, short, FL_IO_CALLBACK) {
  Fl::remove_fd(fd);} // removes all the callbacks!

// type of callback is different and no "id" number is returned:
inline void fl_add_timeout(long msec, void (*cb)(void*), void* v) {
  Fl::add_timeout(msec*.001, (Fl_Timeout_Handler)cb, v);}
inline void fl_remove_timeout(int) {}

// type of callback is different!
inline void fl_set_idle_callback(void (*cb)()) {Fl::set_idle(cb);}

FL_FORMS_API Fl_Widget* fl_do_forms(void);
FL_FORMS_API Fl_Widget* fl_check_forms();
inline Fl_Widget* fl_do_only_forms(void) {return fl_do_forms();}
inline Fl_Widget* fl_check_only_forms(void) {return fl_check_forms();}

// because of new redraw behavior, these are no-ops:
inline void fl_freeze_object(Fl_Widget*) {}
inline void fl_unfreeze_object(Fl_Widget*) {}
inline void fl_freeze_form(Fl_Window*) {}
inline void fl_unfreeze_form(Fl_Window*) {}
inline void fl_freeze_all_forms() {}
inline void fl_unfreeze_all_forms() {}

inline void fl_set_focus_object(Fl_Window*, Fl_Widget* o) {Fl::focus(o);}
inline void fl_reset_focus_object(Fl_Widget* o) {Fl::focus(o);}
#define fl_set_object_focus fl_set_focus_object

// void fl_set_form_atclose(Fl_Window*w,int (*cb)(Fl_Window*,void*),void* v)
// void fl_set_atclose(int (*cb)(Fl_Window*,void*),void*)
// fl_set_form_atactivate/atdeactivate not implemented!

////////////////////////////////////////////////////////////////
// Fl_Widget:

inline void fl_set_object_boxtype(Fl_Widget* o, Fl_Boxtype a) {o->box(a);}
inline void fl_set_object_lsize(Fl_Widget* o,int s) {o->label_size(s);}
FL_FORMS_API void fl_set_object_lstyle(Fl_Widget* o,int a);
inline void fl_set_object_lcol(Fl_Widget* o, unsigned a) {o->label_color((Fl_Color)a);}
#define fl_set_object_lcolor  fl_set_object_lcol
inline void fl_set_object_lalign(Fl_Widget* o, Fl_Align a) { o->clear_flag(FL_ALIGN_MASK); o->set_flag(a);}
#define fl_set_object_align fl_set_object_lalign
inline void fl_set_object_color(Fl_Widget* o,unsigned a,unsigned b) {o->color((Fl_Color)a); o->selection_color((Fl_Color)b);}
inline void fl_set_object_label(Fl_Widget* o, const char* a) {o->label(a); o->redraw();}
inline void fl_set_object_position(Fl_Widget*o,int x,int y) {o->position(x,y);}
inline void fl_set_object_size(Fl_Widget* o, int w, int h) {o->size(w,h);}
inline void fl_set_object_geometry(Fl_Widget* o,int x,int y,int w,int h) {o->resize(x,y,w,h);}

inline void fl_get_object_geometry(Fl_Widget* o,int*x,int*y,int*w,int*h) {
  *x = o->x(); *y = o->y(); *w = o->w(); *h = o->h();}
inline void fl_get_object_position(Fl_Widget* o,int*x,int*y) {
  *x = o->x(); *y = o->y();}

typedef void (*Forms_CB)(Fl_Widget*, long);
inline void fl_set_object_callback(Fl_Widget*o,Forms_CB c,long a) {o->callback(c,a);}
#define fl_set_call_back      fl_set_object_callback
inline void fl_call_object_callback(Fl_Widget* o) {o->do_callback();}
inline void fl_trigger_object(Fl_Widget* o) {o->do_callback();}
inline void fl_set_object_return(Fl_Widget* o, int v) {
  o->when(v|FL_WHEN_RELEASE);}

inline void fl_redraw_object(Fl_Widget* o) {o->redraw();}
inline void fl_show_object(Fl_Widget* o) {o->show();}
inline void fl_hide_object(Fl_Widget* o) {o->hide();}
inline void fl_free_object(Fl_Widget* x) {delete x;}
inline void fl_delete_object(Fl_Widget* o) {((Fl_Group*)(o->parent()))->remove(*o);}
inline void fl_activate_object(Fl_Widget* o) {o->activate();}
inline void fl_deactivate_object(Fl_Widget* o) {o->deactivate();}

inline void fl_add_object(Fl_Window* f, Fl_Widget* x) {f->add(x);}
inline void fl_insert_object(Fl_Widget* o, Fl_Widget* b) {
    ((Fl_Group*)(b->parent()))->insert(*o,b);}

inline Fl_Window* FL_ObjWin(Fl_Widget* o) {return o->window();}

////////////////////////////////////////////////////////////////
// things that appered in the demos a lot that I don't emulate, but
// I did not want to edit out of all the demos...

inline int fl_get_border_width() {return 3;}
inline void fl_set_border_width(int) {}
inline void fl_set_object_dblbuffer(Fl_Widget*, int) {}
inline void fl_set_form_dblbuffer(Fl_Window*, int) {}

////////////////////////////////////////////////////////////////
// Fl_Window:

inline void fl_free_form(Fl_Window* x) {delete x;}
inline void fl_redraw_form(Fl_Window* f) {f->redraw();}

inline Fl_Window* fl_bgn_form(Fl_Boxtype b,int w,int h) {
  Fl_Window* g = new Fl_Window(w,h,0);
  g->box(b);
  return g;
}
inline void fl_addto_form(Fl_Window* f) {f->begin();}
inline Fl_Group* fl_bgn_group() {return new Fl_Group(0,0,0,0,0);}
inline void fl_addto_group(Fl_Widget* o) {((Fl_Group* )o)->begin();}
FL_FORMS_API void fl_end_group();
FL_FORMS_API void fl_end_form();
#define resizebox _ddfdesign_kludge()

inline void fl_scale_form(Fl_Window* f, double x, double y) {
  f->resizable(f); f->size(int(f->w()*x),int(f->h()*y));}
inline void fl_set_form_position(Fl_Window* f,int x,int y) {f->position(x,y);}
inline void fl_set_form_size(Fl_Window* f, int w, int h) {f->size(w,h);}
inline void fl_set_form_geometry(Fl_Window* f,int x,int y,int w,int h) {
  f->resize(x,y,w,h);}
#define fl_set_initial_placement fl_set_form_geometry
inline void fl_adjust_form_size(Fl_Window*) {}

FL_FORMS_API void fl_show_form(Fl_Window* f,int p,int b,const char* n);
enum {	// "p" argument values:
  FL_PLACE_FREE = 0,	// make resizable
  FL_PLACE_MOUSE = 1,	// mouse centered on form
  FL_PLACE_CENTER = 2,	// center of the screen
  FL_PLACE_POSITION = 4,// fixed position, resizable
  FL_PLACE_SIZE = 8,	// fixed size, normal fltk behavior
  FL_PLACE_GEOMETRY =16,// fixed size and position
  FL_PLACE_ASPECT = 32,	// keep aspect ratio (ignored)
  FL_PLACE_FULLSCREEN=64,// fill screen
  FL_PLACE_HOTSPOT = 128,// enables hotspot
  FL_PLACE_ICONIC = 256,// iconic (ignored)
  FL_FREE_SIZE=(1<<14),	// force resizable
  FL_FIX_SIZE =(1<<15)	// force off resizable
};
#define FL_PLACE_FREE_CENTER (FL_PLACE_CENTER|FL_FREE_SIZE)
#define FL_PLACE_CENTERFREE  (FL_PLACE_CENTER|FL_FREE_SIZE)
enum {	// "b" arguement values:
  FL_NOBORDER = 0,
  FL_FULLBORDER,
  FL_TRANSIENT
//FL_MODAL = (1<<8)	// not implemented yet in Forms
};
inline void fl_set_form_hotspot(Fl_Window* w,int x,int y) {w->hotspot(x,y);}
inline void fl_set_form_hotobject(Fl_Window* w, Fl_Widget* o) {w->hotspot(o);}
extern FL_FORMS_API char fl_flip;	// in forms.C
inline void fl_flip_yorigin() {fl_flip = 1;}

#define fl_prepare_form_window fl_show_form
inline void fl_show_form_window(Fl_Window*) {}

inline void fl_raise_form(Fl_Window* f) {f->show();}

inline void fl_hide_form(Fl_Window* f) {f->hide();}
inline void fl_pop_form(Fl_Window* f) {f->show();}

extern FL_FORMS_API char fl_modal_next; // in forms.C
inline void fl_activate_all_forms() {}
inline void fl_deactivate_all_forms() {fl_modal_next = 1;}
inline void fl_deactivate_form(Fl_Window*w) {w->deactivate();}
inline void fl_activate_form(Fl_Window*w) {w->activate();}

inline void fl_set_form_title(Fl_Window* f, const char* s) {f->label(s);}
inline void fl_title_form(Fl_Window* f, const char* s) {f->label(s);}

typedef void (*Forms_FormCB)(Fl_Widget*);
inline void fl_set_form_callback(Fl_Window* f,Forms_FormCB c) {f->callback(c);}
#define fl_set_form_call_back fl_set_form_callback

inline void fl_init() {}
inline void fl_set_graphics_mode(int r, int /*d*/) {
  Fl::visual(r ? FL_RGB : FL_INDEX);
  // d should add FL_DOUBLE, but that always fails in fltk 2.0
}

inline int fl_form_is_visible(Fl_Window* f) {return f->visible();}

inline int fl_mouse_button() {return Fl::event_button();}
#define fl_mousebutton fl_mouse_button

#define fl_free       free
#define fl_malloc     malloc
#define fl_calloc     calloc
#define fl_realloc    realloc

////////////////////////////////////////////////////////////////
// Drawing functions.  Only usable inside an Fl_Free object?

#if 0
inline void fl_drw_box(Fl_Boxtype b,int x,int y,int w,int h,Fl_Color bgc,int=3) {
    b->draw(x,y,w,h,bgc);}
inline void fl_drw_frame(Fl_Boxtype b,int x,int y,int w,int h,Fl_Color bgc,int=3) {
    b->draw(x,y,w,h,bgc,FL_FRAME_ONLY);}
#endif

inline void fl_drw_text(Fl_Align align, int x, int y, int w, int h,
		  Fl_Color fgcolor, int size, Fl_Font style,
		  const char* s) {
  fl_font(style,size);
  fl_color(fgcolor);
  fl_draw(s,x,y,w,h,align);
}

// this does not work except for CENTER...
inline void fl_drw_text_beside(Fl_Align align, int x, int y, int w, int h,
		  Fl_Color fgcolor, int size, Fl_Font style,
		  const char* s) {
  fl_font(style,size);
  fl_color(fgcolor);
  fl_draw(s,x,y,w,h,align);
}

//inline void fl_set_font_name(Fl_Font n,const char* s) {fl_set_font(n,s);}

inline void fl_mapcolor(Fl_Color c, uchar r, uchar g, uchar b) {
  fl_set_color(c,fl_rgb(r,g,b));}
#define fl_set_clipping(x,y,w,h) fl_clip(x,y,w,h)
#define fl_unset_clipping() fl_pop_clip()

////////////////////////////////////////////////////////////////
// Forms classes:

inline Fl_Widget* fl_add_new(Fl_Widget* p) {return p;}
inline Fl_Widget* fl_add_new(uchar t,Fl_Widget* p) {p->type(t); return p;}

#define forms_constructor(type,name) \
inline type* name(uchar t,int x,int y,int w,int h,const char* l) { \
 return (type*)(fl_add_new(t, new type(x,y,w,h,l)));}
#define forms_constructort(type,name) \
inline type* name(uchar t,int x,int y,int w,int h,const char* l) { \
 return (type*)(fl_add_new(new type(t,x,y,w,h,l)));}
#define forms_constructorb(type,name) \
inline type* name(Fl_Boxtype t,int x,int y,int w,int h,const char* l) { \
 return (type*)(fl_add_new(new type(t,x,y,w,h,l)));}

#include "Fl_FormsBitmap.h"
#define FL_NORMAL_BITMAP FL_NO_BOX
forms_constructorb(Fl_FormsBitmap, fl_add_bitmap)
inline void fl_set_bitmap_data(Fl_Widget* o, int w, int h, const uchar* b) {
    ((Fl_FormsBitmap*)o)->set(w,h,b);
}

#include "Fl_FormsPixmap.h"
#define FL_NORMAL_PIXMAP FL_NO_BOX
forms_constructorb(Fl_FormsPixmap, fl_add_pixmap)
inline void fl_set_pixmap_data(Fl_Widget* o, char*const* b) {
    ((Fl_FormsPixmap*)o)->set(b);
}
//inline void fl_set_pixmap_file(Fl_Widget*, const char*);
inline void fl_set_pixmap_align(Fl_Widget* o,Fl_Align a,int,int) { o->clear_flag(FL_ALIGN_MASK); o->set_flag(a);}
//inline void fl_set_pixmap_colorcloseness(int, int, int);

#include <fltk/Fl_Box.h>
forms_constructorb(Fl_Box, fl_add_box)

#include <fltk/Fl_Browser.h>
forms_constructor(Fl_Browser, fl_add_browser)

inline void fl_clear_browser(Fl_Widget* o) {
    ((Fl_Browser*)o)->clear();}
inline void fl_add_browser_line(Fl_Widget* o, const char* s) {
    ((Fl_Browser*)o)->add(s);}
inline void fl_addto_browser(Fl_Widget* o, const char* s) {
    ((Fl_Browser*)o)->add(s);} /* should also scroll to bottom */
//inline void fl_addto_browser_chars(Fl_Widget*, const char*)
//#define fl_append_browser fl_addto_browser_chars
inline void fl_insert_browser_line(Fl_Widget* o, int n, const char* s) {
    ((Fl_Browser*)o)->insert(n,s);}
inline void fl_delete_browser_line(Fl_Widget* o, int n) {
    ((Fl_Browser*)o)->remove(n);}
inline void fl_replace_browser_line(Fl_Widget* o, int n, const char* s) {
    ((Fl_Browser*)o)->replace(n,s);}
inline char* fl_get_browser_line(Fl_Widget* o, int n) {
    return (char*)(((Fl_Browser*)o)->text(n));}
FL_FORMS_API int fl_load_browser(Fl_Widget* o, const char* f);
inline void fl_select_browser_line(Fl_Widget* o, int n) {
    ((Fl_Browser*)o)->select(n,1);}
inline void fl_deselect_browser_line(Fl_Widget* o, int n) {
    ((Fl_Browser*)o)->select(n,0);}
inline void fl_deselect_browser(Fl_Widget* o) {
    ((Fl_Browser*)o)->deselect();}
inline int fl_isselected_browser_line(Fl_Widget* o, int n) {
    return ((Fl_Browser*)o)->selected(n);}
inline int fl_get_browser_topline(Fl_Widget* o) {
    return ((Fl_Browser*)o)->topline();}
inline int fl_get_browser(Fl_Widget* o) {
    return ((Fl_Browser*)o)->value();}
inline int fl_get_browser_maxline(Fl_Widget* o) {
    return ((Fl_Browser*)o)->size();}
//linline int fl_get_browser_screenlines(Fl_Widget*);
inline void fl_set_browser_topline(Fl_Widget* o, int n) {
    ((Fl_Browser*)o)->topline(n);}
inline void fl_set_browser_fontsize(Fl_Widget* o, int s) {
    ((Fl_Browser*)o)->text_size(s);}
inline void fl_set_browser_fontstyle(Fl_Widget* o, int s) {
    ((Fl_Browser*)o)->text_font(fl_fonts+s);}
inline void fl_set_browser_specialkey(Fl_Widget* o, char c) {
    ((Fl_Browser*)o)->format_char(c);}
//inline void fl_set_browser_vscrollbar(Fl_Widget*, int);
//inline void fl_set_browser_hscrollbar(Fl_Widget*, int);
//inline void fl_set_browser_leftslider(Fl_Widget*, int);
//#define fl_set_browser_leftscrollbar fl_set_browser_leftslider
//inline void fl_set_browser_line_selectable(Fl_Widget*, int, int);
//inline void fl_get_browser_dimension(Fl_Widget*,int*,int*,int*,int*);
//inline void fl_set_browser_dblclick_callback(Fl_Widget*,FL_CALLBACKPTR,long);
//inline void fl_set_browser_xoffset(Fl_Widget*, FL_Coord);
//inline void fl_set_browser_scrollbarsize(Fl_Widget*, int, int);
inline void fl_setdisplayed_browser_line(Fl_Widget* o, int n, int i) {
    ((Fl_Browser*)o)->display(n,i);}
inline int fl_isdisplayed_browser_line(Fl_Widget* o, int n) {
    return ((Fl_Browser*)o)->displayed(n);}

#include <fltk/Fl_Button.h>

#define FL_NORMAL_BUTTON	0
//#define FL_HIDDEN_BUTTON
#define FL_TOUCH_BUTTON		4
#define FL_INOUT_BUTTON		5
#define FL_RETURN_BUTTON	6
#define FL_HIDDEN_RET_BUTTON	7
#define FL_PUSH_BUTTON		FL_TOGGLE_BUTTON
#define FL_MENU_BUTTON		9

FL_FORMS_API Fl_Button* fl_add_button(uchar t,int x,int y,int w,int h,const char* l);
inline int fl_get_button(Fl_Widget* b) {return ((Fl_Button*)b)->value();}
inline void fl_set_button(Fl_Widget* b, int v) {((Fl_Button*)b)->value(v);}
inline int fl_get_button_numb(Fl_Widget*) {return Fl::event_button();}
inline void fl_set_object_shortcut(Fl_Widget* b, const char* s, int=0) {
  b->shortcut(fltk::key(s));}
#define fl_set_button_shortcut fl_set_object_shortcut

#include <fltk/Fl_Light_Button.h>
forms_constructor(Fl_Light_Button, fl_add_lightbutton)

#include <fltk/Fl_Round_Button.h>
forms_constructor(Fl_Round_Button, fl_add_roundbutton)
forms_constructor(Fl_Round_Button, fl_add_round3dbutton)

#include <fltk/Fl_Check_Button.h>
forms_constructor(Fl_Check_Button, fl_add_checkbutton)

inline Fl_Widget* fl_add_bitmapbutton(int t,int x,int y,int w,int h,const char* l) {Fl_Widget* o = fl_add_button(t,x,y,w,h,l); return o;}
inline void fl_set_bitmapbutton_data(Fl_Widget* o,int a,int b,uchar* c) {
  (new Fl_Bitmap(c,a,b))->label(o);}  // does not delete old Fl_Bitmap!

inline Fl_Widget* fl_add_pixmapbutton(int t,int x,int y,int w,int h,const char* l) {Fl_Widget* o = fl_add_button(t,x,y,w,h,l); return o;}
inline void fl_set_pixmapbutton_data(Fl_Widget* o, const char*const* c) {
  (new Fl_Pixmap(c))->label(o);}  // does not delete old Fl_Pixmap!

// Fl_Canvas object not yet implemented!

#include "Fl_Chart.h"

forms_constructor(Fl_Chart, fl_add_chart)
inline void fl_clear_chart(Fl_Widget* o) {
  ((Fl_Chart*)o)->clear();}
inline void fl_add_chart_value(Fl_Widget* o,double v,const char* s,uchar c){
  ((Fl_Chart*)o)->add(v,s,c);}
inline void fl_insert_chart_value(Fl_Widget* o, int i, double v, const char* s, uchar c) {
  ((Fl_Chart*)o)->insert(i,v,s,c);}
inline void fl_replace_chart_value(Fl_Widget* o, int i, double v, const char* s, uchar c) {
  ((Fl_Chart*)o)->replace(i,v,s,c);}
inline void fl_set_chart_bounds(Fl_Widget* o, double a, double b) {
  ((Fl_Chart*)o)->bounds(a,b);}
inline void fl_set_chart_maxnumb(Fl_Widget* o, int v) {
  ((Fl_Chart*)o)->maxsize(v);}
inline void fl_set_chart_autosize(Fl_Widget* o, int v) {
  ((Fl_Chart*)o)->autosize(v);}
inline void fl_set_chart_lstyle(Fl_Widget* o, Fl_Font v) {
  ((Fl_Chart*)o)->text_font(v);}
inline void fl_set_chart_lsize(Fl_Widget* o, int v) {
  ((Fl_Chart*)o)->text_size(v);}
inline void fl_set_chart_lcolor(Fl_Widget* o, unsigned v) {
  ((Fl_Chart*)o)->text_color((Fl_Color)v);}
#define fl_set_chart_lcol   fl_set_chart_lcolor

#include <fltk/Fl_Choice.h>

#define FL_NORMAL_CHOICE	0
#define FL_NORMAL_CHOICE2	0
#define FL_DROPLIST_CHOICE	0

forms_constructor(Fl_Choice, fl_add_choice)
inline void fl_clear_choice(Fl_Widget* o) {
    ((Fl_Choice*)o)->clear();}
inline void fl_addto_choice(Fl_Widget* o, const char* s) {
    ((Fl_Choice*)o)->add(s);}
inline void fl_replace_choice(Fl_Widget* o, int i, const char* s) {
    ((Fl_Choice*)o)->replace(i-1,s);}
inline void fl_delete_choice(Fl_Widget* o, int i) {
    ((Fl_Choice*)o)->remove(i-1);}
inline void fl_set_choice(Fl_Widget* o, int i) {
    ((Fl_Choice*)o)->value(i-1);}
// inline void fl_set_choice_text(Fl_Widget*, const char*);
inline int fl_get_choice(Fl_Widget* o) {
    return ((Fl_Choice*)o)->value()+1;}
// inline const char* fl_get_choice_item_text(Fl_Widget*, int);
// inline int fl_get_choice_maxitems(Fl_Widget*);
inline const char* fl_get_choice_text(Fl_Widget* o) {
    return ((Fl_Choice*)o)->text();}
inline void fl_set_choice_fontsize(Fl_Widget* o, int x) {
    ((Fl_Choice*)o)->text_size(x);}
inline void fl_set_choice_fontstyle(Fl_Widget* o, Fl_Font x) {
    ((Fl_Choice*)o)->text_font(x);}
// inline void fl_set_choice_item_mode(Fl_Widget*, int, unsigned);
// inline void fl_set_choice_item_shortcut(Fl_Widget*, int, const char*);

#include <fltk/Fl_Clock.h>
forms_constructor(Fl_Clock, fl_add_clock)
inline void fl_get_clock(Fl_Widget* o, int* h, int* m, int* s) {
    *h = ((Fl_Clock*)o)->hour();
    *m = ((Fl_Clock*)o)->minute();
    *s = ((Fl_Clock*)o)->second();
}

#include <fltk/Fl_Counter.h>
forms_constructor(Fl_Counter, fl_add_counter)
inline void fl_set_counter_value(Fl_Widget* o, double v) {
    ((Fl_Counter*)o)->value(v);}
inline void fl_set_counter_bounds(Fl_Widget* o, double a, double b) {
    ((Fl_Counter*)o)->range(a,b);}
inline void fl_set_counter_step(Fl_Widget* o, double a, double b) {
    ((Fl_Counter*)o)->step(a / b);}
inline void fl_set_counter_precision(Fl_Widget* o, int v) {
//    ((Fl_Counter*)o)->precision(v);}
    ((Fl_Counter*)o)->step(1/(10^v));}
inline void fl_set_counter_return(Fl_Widget* o, int v) {
    ((Fl_Counter*)o)->when(v|FL_WHEN_RELEASE);}
inline double fl_get_counter_value(Fl_Widget* o) {
    return ((Fl_Counter*)o)->value();}
inline void fl_get_counter_bounds(Fl_Widget* o, float* a, float* b) {
  *a = float(((Fl_Counter*)o)->minimum());
  *b = float(((Fl_Counter*)o)->maximum());
}
//inline void fl_set_counter_filter(Fl_Widget*,const char* (*)(Fl_Widget*,double,int));

// Cursor stuff cannot be emulated because it uses X stuff
inline void fl_set_cursor(Fl_Window* w, Fl_Cursor c) {w->cursor(c);}
#define FL_INVISIBLE_CURSOR FL_CURSOR_NONE
#define FL_DEFAULT_CURSOR FL_CURSOR_DEFAULT

#include <fltk/Fl_Dial.h>

#define FL_DIAL_COL1 FL_GRAY
#define FL_DIAL_COL2 37

forms_constructor(Fl_Dial, fl_add_dial)
inline void fl_set_dial_value(Fl_Widget* o, double v) {
  ((Fl_Dial*)o)->value(v);}
inline double fl_get_dial_value(Fl_Widget* o) {
  return ((Fl_Dial*)o)->value();}
inline void fl_set_dial_bounds(Fl_Widget* o, double a, double b) {
  ((Fl_Dial*)o)->range(a, b);}
inline void fl_get_dial_bounds(Fl_Widget* o, float* a, float* b) {
  *a = float(((Fl_Dial*)o)->minimum());
  *b = float(((Fl_Dial*)o)->maximum());
}
inline void fl_set_dial_return(Fl_Widget* o, int i) {
  ((Fl_Dial*)o)->when(i|FL_WHEN_RELEASE);}
inline void fl_set_dial_angles(Fl_Widget* o, int a, int b) {
  ((Fl_Dial*)o)->angles(a, b);}
//inline void fl_set_dial_cross(Fl_Widget* o, int);
// inline void fl_set_dial_direction(Fl_Widget* o, uchar d) {
//   ((Fl_Dial*)o)->direction(d);}
inline void fl_set_dial_step(Fl_Widget* o, double v) {
  ((Fl_Dial*)o)->step(v);}

// Frames:

inline Fl_Widget* fl_add_frame(Fl_Boxtype i,int x,int y,int w,int h,const char* l) {
  return fl_add_box(i,x-3,y-3,w+6,h+6,l);}

// labelframe nyi
inline Fl_Widget* fl_add_labelframe(Fl_Boxtype i,int x,int y,int w,int h,const char* l) {
  Fl_Widget* o = fl_add_box(i,x-3,y-3,w+6,h+6,l);
  o->clear_flag(FL_ALIGN_MASK); 
  o->set_flag(FL_ALIGN_TOP | FL_ALIGN_LEFT);
  return o;
}

#include "Fl_Free.h"
inline Fl_Free*
fl_add_free(int t,double x,double y,double w,double h,const char* l,
	    FL_HANDLEPTR hdl) {
 return (Fl_Free*)(fl_add_new(
   new Fl_Free(t,int(x),int(y),int(w),int(h),l,hdl)));
}

#include <fltk/fl_ask.h>
#include <fltk/fl_show_colormap.h>

inline int fl_show_question(const char* c, int = 0) {return fl_ask(c);}
FL_FORMS_API void fl_show_message(const char *,const char *,const char *);
FL_FORMS_API void fl_show_alert(const char *,const char *,const char *,int=0);
FL_FORMS_API int fl_show_question(const char *,const char *,const char *);
inline const char *fl_show_input(const char *l,const char*d=0) {return fl_input(l,d);}
/*const*/ char *fl_show_simple_input(const char *label, const char *deflt = 0);
int fl_show_choice(
    const char *m1,
    const char *m2,
    const char *m3,
    int numb,
    const char *b0,
    const char *b1,
    const char *b2);

inline void fl_set_goodies_font(int a, unsigned b) {
  fl_message_style->label_font = fl_fonts+a;
  fl_message_style->label_size = b;
}
#define fl_show_messages fl_message
inline int fl_show_choices(const char* c,int n,const char* b1,const char* b2,
			   const char* b3, int) {
  return fl_show_choice(0,c,0,n,b1,b2,b3);
}

#include <fltk/filename.h>
#include <fltk/fl_file_chooser.h>
inline int do_matching(char* a, const char* b) {return filename_match(a,b);}

// Forms-compatable file chooser (implementation in fselect.C):
FL_FORMS_API char* fl_show_file_selector(const char* message,const char* dir,
			    const char* pat,const char* fname);
FL_FORMS_API char*	fl_get_directory();
FL_FORMS_API char*	fl_get_pattern();
FL_FORMS_API char*	fl_get_filename();

#include <fltk/Fl_Input.h>
forms_constructor(Fl_Input, fl_add_input)
inline void fl_set_input(Fl_Widget* o, const char* v) {
    ((Fl_Input*)o)->text(v);}
inline void fl_set_input_return(Fl_Widget* o, int x) {
    ((Fl_Input*)o)->when(x | FL_WHEN_RELEASE);}
inline void fl_set_input_color(Fl_Widget* o, unsigned a, unsigned /*b*/) {
    ((Fl_Input*)o)->text_color((Fl_Color)a);
//  ((Fl_Input*)o)->cursor_color((Fl_Color)b);
}
// inline void fl_set_input_scroll(Fl_Widget*, int);
inline void fl_set_input_cursorpos(Fl_Widget* o, int x, int /*y*/) {
  ((Fl_Input*)o)->position(x);}
// inline void fl_set_input_selected(Fl_Widget*, int);
// inline void fl_set_input_selected_range(Fl_Widget*, int, int);
// inline void fl_set_input_maxchars(Fl_Widget*, int);
// inline void fl_set_input_format(Fl_Widget*, int, int);
// inline void fl_set_input_hscrollbar(Fl_Widget*, int);
// inline void fl_set_input_vscrollbar(Fl_Widget*, int);
// inline void fl_set_input_xoffset(Fl_Widget*, int);
// inline void fl_set_input_topline(Fl_Widget*, int);
// inline void fl_set_input_scrollbarsize(Fl_Widget*, int, int);
// inline int fl_get_input_topline(Fl_Widget*);
// inline int fl_get_input_screenlines(Fl_Widget*);
inline int fl_get_input_cursorpos(Fl_Widget* o, int*x, int*y) {
  *x = ((Fl_Input*)o)->position(); *y = 0; return *x;}
// inline int fl_get_input_numberoflines(Fl_Widget*);
// inline void fl_get_input_format(Fl_Widget*, int*, int*);
inline const char* fl_get_input(Fl_Widget* o) {return ((Fl_Input*)o)->text();}

#include <fltk/Fl_Menu_Button.h>

// types are not implemented, they all act like FL_PUSH_MENU:
#define FL_TOUCH_MENU		0
#define FL_PUSH_MENU		1
#define FL_PULLDOWN_MENU	2
forms_constructor(Fl_Menu_Button, fl_add_menu)

inline void fl_clear_menu(Fl_Widget* o) {
    ((Fl_Menu_Button*)o)->clear();}
inline void fl_set_menu(Fl_Widget* o, const char* s) {
    ((Fl_Menu_Button*)o)->clear(); ((Fl_Menu_Button*)o)->add(s);}
inline void fl_addto_menu(Fl_Widget* o, const char* s) {
    ((Fl_Menu_Button*)o)->add(s);}
inline void fl_replace_menu_item(Fl_Widget* o, int i, const char* s) {
    ((Fl_Menu_Button*)o)->replace(i-1,s);}
inline void fl_delete_menu_item(Fl_Widget* o, int i) {
    ((Fl_Menu_Button*)o)->remove(i-1);}
inline void fl_set_menu_item_shortcut(Fl_Widget* o, int i, const char* s) {
    ((Fl_Menu_Button*)o)->shortcut(i-1,fltk::key(s));}
// inline void fl_set_menu_item_mode(Fl_Widget* o, int i, long x) {
//     ((Fl_Menu_Button*)o)->mode(i-1,x);}
inline void fl_show_menu_symbol(Fl_Widget*, int ) {
/*    ((Fl_Menu_Button*)o)->show_menu_symbol(i); */}
// inline void fl_set_menu_popup(Fl_Widget*, int);
inline int fl_get_menu(Fl_Widget* o) {
    return ((Fl_Menu_Button*)o)->value()+1;}
inline const char* fl_get_menu_item_text(Fl_Widget* o, int i) {
    return ((Fl_Menu_Button*)o)->text(i);}
inline int fl_get_menu_maxitems(Fl_Widget* o) {
    return ((Fl_Menu_Button*)o)->size();}
inline int fl_get_menu_item_mode(Fl_Widget* o, int i) {
    return ((Fl_Menu_Button*)o)->mode(i);}
inline const char* fl_get_menu_text(Fl_Widget* o) {
    return ((Fl_Menu_Button*)o)->text();}

#include "Fl_Positioner.h"
#define FL_NORMAL_POSITIONER	0
forms_constructor(Fl_Positioner, fl_add_positioner)
inline void fl_set_positioner_xvalue(Fl_Widget* o, double v) {
    ((Fl_Positioner*)o)->xvalue(v);}
inline double fl_get_positioner_xvalue(Fl_Widget* o) {
    return ((Fl_Positioner*)o)->xvalue();}
inline void fl_set_positioner_xbounds(Fl_Widget* o, double a, double b) {
    ((Fl_Positioner*)o)->xbounds(a,b);}
inline void fl_get_positioner_xbounds(Fl_Widget* o, float* a, float* b) {
  *a = float(((Fl_Positioner*)o)->xminimum());
  *b = float(((Fl_Positioner*)o)->xmaximum());
}
inline void fl_set_positioner_yvalue(Fl_Widget* o, double v) {
    ((Fl_Positioner*)o)->yvalue(v);}
inline double fl_get_positioner_yvalue(Fl_Widget* o) {
    return ((Fl_Positioner*)o)->yvalue();}
inline void fl_set_positioner_ybounds(Fl_Widget* o, double a, double b) {
    ((Fl_Positioner*)o)->ybounds(a,b);}
inline void fl_get_positioner_ybounds(Fl_Widget* o, float* a, float* b) {
  *a = float(((Fl_Positioner*)o)->yminimum());
  *b = float(((Fl_Positioner*)o)->ymaximum());
}
inline void fl_set_positioner_xstep(Fl_Widget* o, double v) {
    ((Fl_Positioner*)o)->xstep(v);}
inline void fl_set_positioner_ystep(Fl_Widget* o, double v) {
    ((Fl_Positioner*)o)->ystep(v);}
inline void fl_set_positioner_return(Fl_Widget* o, int v) {
    ((Fl_Positioner*)o)->when(v|FL_WHEN_RELEASE);}

#include <fltk/Fl_Slider.h>

#define FL_HOR_BROWSER_SLIDER FL_HOR_SLIDER
#define FL_VERT_BROWSER_SLIDER FL_VERT_SLIDER

forms_constructor(Fl_Slider, fl_add_slider)
#define FL_SLIDER_COL1 FL_GRAY
inline void fl_set_slider_value(Fl_Widget* o, double v) {
    ((Fl_Slider*)o)->value(v);}
inline double fl_get_slider_value(Fl_Widget* o) {
    return ((Fl_Slider*)o)->value();}
inline void fl_set_slider_bounds(Fl_Widget* o, double a, double b) {
    ((Fl_Slider*)o)->range(a, b);}
inline void fl_get_slider_bounds(Fl_Widget* o, float* a, float* b) {
  *a = float(((Fl_Slider*)o)->minimum());
  *b = float(((Fl_Slider*)o)->maximum());
}
inline void fl_set_slider_return(Fl_Widget* o, int i) {
    ((Fl_Slider*)o)->when(i|FL_WHEN_RELEASE);}
inline void fl_set_slider_step(Fl_Widget* o, double v) {
    ((Fl_Slider*)o)->step(v);}
// inline void fl_set_slider_increment(Fl_Widget* o, double v, double);
inline void fl_set_slider_size(Fl_Widget* o, double v) {
    ((Fl_Slider*)o)->slider_size(v);}

#include <fltk/Fl_Value_Slider.h>
forms_constructor(Fl_Value_Slider, fl_add_valslider)

inline void fl_set_slider_precision(Fl_Widget* o, int i) {
  double v = 1.0;
  while (i--) v /= 10.0;
  ((Fl_Value_Slider*)o)->step(v);
}

// The forms text object was the same as an Fl_Box except it inverted the
// meaning of FL_ALIGN_INSIDE.  Implementation in forms.C
class FL_FORMS_API Fl_FormsText : public Fl_Widget {
protected:
    void draw();
public:
    Fl_FormsText(Fl_Boxtype b, int x, int y, int w, int h, const char* l=0)
	: Fl_Widget(x,y,w,h,l) {box(b); clear_flag(FL_ALIGN_MASK); set_flag(FL_ALIGN_LEFT);}
};
#define FL_NORMAL_TEXT FL_NO_BOX
forms_constructorb(Fl_FormsText, fl_add_text)

#include "Fl_Timer.h"
forms_constructort(Fl_Timer, fl_add_timer)
inline void fl_set_timer(Fl_Widget* o, double v) {((Fl_Timer*)o)->value(v);}
inline double fl_get_timer(Fl_Widget* o) {return ((Fl_Timer*)o)->value();}
inline void fl_suspend_timer(Fl_Widget* o) {((Fl_Timer*)o)->suspended(1);}
inline void fl_resume_timer(Fl_Widget* o) {((Fl_Timer*)o)->suspended(0);}
inline void fl_set_timer_countup(Fl_Widget* o,char d) {((Fl_Timer*)o)->direction(d);}
FL_FORMS_API void fl_gettime(long* sec, long* usec);

// Fl_XYPlot nyi


// stuff from DDForms:

inline int fl_double_click() {return Fl::event_clicks();}
inline void fl_draw() {Fl::flush();}

#endif	/* define __FORMS_H__ */

//
// End of "$Id: forms.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
