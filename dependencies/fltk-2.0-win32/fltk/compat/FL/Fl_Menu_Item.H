//
// "$Id: Fl_Menu_Item.H 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Back-compatability menu items for FLTK.  The new fltk::Menu class
// converts these tables into child fltk::Item and fltk::ItemGroup widgets.
// You should not use this for new programs.
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

#ifndef Fl_Menu_Item_h
#define Fl_Menu_Item_h

#include <fltk/Widget.h>

enum { // values for flags:
  FL_MENU_INACTIVE = 1,
  FL_MENU_TOGGLE= 2,
  FL_MENU_VALUE = 4,
  FL_MENU_RADIO = 8,
  FL_MENU_INVISIBLE = 0x10,
  FL_SUBMENU_POINTER = 0x20,
  FL_SUBMENU = 0x40,
  FL_MENU_DIVIDER = 0x80,
  FL_MENU_HORIZONTAL = 0x100 // not used!
};

namespace fltk {
  class FL_API Menu;
}

struct FL_API Fl_Menu_Item {
  const char *text;
  int shortcut_;
  fltk::Callback *callback_;
  void *user_data_;
  int flags;
  fltk::LabelType* labeltype_;
  fltk::Font* labelfont_;
  unsigned labelsize_;
  fltk::Color labelcolor_;

  // Used to add or set an fltk::Menu widget:
  void add_to(fltk::Menu*, void* data = 0) const;

  // popup menus without using an fltk::Menu widget:
  // Warning: this is now quite expensive, as it creates a temporary
  // fltk::Menu and child widgets!  These also do not match the fltk 1.0
  // calls, if there is any callback it is called directly (with the
  // dummy fltk::Menu as an argument).
  const Fl_Menu_Item*
	pulldown(int X, int Y, int W, int H,
		 const Fl_Menu_Item* picked = 0,
		 const char* title = 0) const;
  const Fl_Menu_Item*
	popup(int X, int Y, const char* title=0,
	      const Fl_Menu_Item* picked = 0) const {
    return pulldown(X, Y, 0, 0, picked, title);}
  const Fl_Menu_Item*
	test_shortcut() const;

  // return offset of terminating null item:
  int size() const ;

  // advance a pointer by N items, skipping submenus:
  const Fl_Menu_Item *next(int=1) const;
  Fl_Menu_Item *next(int i=1) {
    return (Fl_Menu_Item*)(((const Fl_Menu_Item*)this)->next(i));}

  const char* label() const {return text;}
  fltk::LabelType* labeltype() const {return labeltype_;}
  fltk::Color labelcolor() const {return labelcolor_;}
  fltk::Font* labelfont() const {return labelfont_;}
  unsigned labelsize() const {return labelsize_;}
  fltk::Callback_p callback() const {return callback_;}
  void* user_data() const {return user_data_;}
  long argument() const {return (long)user_data_;}
  int shortcut() const {return shortcut_;}
  int submenu() const {return flags&(FL_SUBMENU|FL_SUBMENU_POINTER);}
  int checkbox() const {return flags&FL_MENU_TOGGLE;}
  int radio() const {return flags&FL_MENU_RADIO;}
  int value() const {return flags&FL_MENU_VALUE;}
  int visible() const {return !(flags&FL_MENU_INVISIBLE);}
  int active() const {return !(flags&FL_MENU_INACTIVE);}
  int activevisible() const {return !(flags&0x11);}

  void label(const char* a) {text=a;}
  void label(fltk::LabelType* a,const char* b) {labeltype_ = a; text = b;}
  void labeltype(fltk::LabelType* a) {labeltype_ = a;}
  void labelcolor(uchar a) {labelcolor_ = a;}
  void labelfont(fltk::Font* a) {labelfont_ = a;}
  void labelsize(uchar a) {labelsize_ = a;}
  void callback(fltk::Callback* c, void* p) {callback_=c; user_data_=p;}
  void callback(fltk::Callback* c) {callback_=c;}
  void callback(fltk::Callback0*c) {callback_=(fltk::Callback*)c;}
  void callback(fltk::Callback1*c, long p=0) {callback_=(fltk::Callback*)c; user_data_=(void*)p;}
  void user_data(void* v) {user_data_ = v;}
  void argument(long v) {user_data_ = (void*)v;}
  void shortcut(int s) {shortcut_ = s;}
  void set() {flags |= FL_MENU_VALUE;}
  void clear() {flags &= ~FL_MENU_VALUE;}
  void setonly();
  void show() {flags &= ~FL_MENU_INVISIBLE;}
  void hide() {flags |= FL_MENU_INVISIBLE;}
  void activate() {flags &= ~FL_MENU_INACTIVE;}
  void deactivate() {flags |= FL_MENU_INACTIVE;}

  const Fl_Menu_Item* find_shortcut(int *ip=0) const;

  void do_callback(fltk::Widget* o) const {callback_(o, user_data_);}
  void do_callback(fltk::Widget* o,void* arg) const {callback_(o, arg);}
  void do_callback(fltk::Widget* o,long arg) const {callback_(o, (void*)arg);}

  // back-compatability, do not use:
  int checked() const {return flags&FL_MENU_VALUE;}
  void check() {flags |= FL_MENU_VALUE;}
  void uncheck() {flags &= ~FL_MENU_VALUE;}

#if 0
  int add(const char*, int shortcut, fltk::Callback*, void* =0, int = 0);
  int add(const char*a, const char* b, fltk::Callback* c,
	  void* d = 0, int e = 0) {
    return add(a,fltk::old_shortcut(b),c,d,e);}
#endif
};

typedef Fl_Menu_Item Fl_Menu; // back compatability with fltk < 1.0

enum {	// back-compatability enum:
  FL_PUP_NONE	= 0,
  FL_PUP_GREY	= FL_MENU_INACTIVE,
  FL_PUP_GRAY	= FL_MENU_INACTIVE,
  FL_MENU_BOX	= FL_MENU_TOGGLE,
  FL_PUP_BOX	= FL_MENU_TOGGLE,
  FL_MENU_CHECK	= FL_MENU_VALUE,
  FL_PUP_CHECK	= FL_MENU_VALUE,
  FL_PUP_RADIO	= FL_MENU_RADIO,
  FL_PUP_INVISIBLE = FL_MENU_INVISIBLE,
  FL_PUP_SUBMENU = FL_SUBMENU_POINTER
};

#endif

//
// End of "$Id: Fl_Menu_Item.H 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
