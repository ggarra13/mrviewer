// "$Id: FluidType.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Base type of object constructed and manipualated by the
// Fast Light toolkit Interface Designer (fluid)
//
// Each object described by Fluid is one of these objects.  They
// are all stored in a double-linked list.
//
// There is also a single "factory" instance of each type of this.
// The method "make()" is called on this factory to create a new
// instance of this object.  It could also have a "copy()" function,
// but it was easier to implement this by using the file read/write
// that is needed to save the setup anyways.
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

#ifndef fltk_fluid_type_h
#define fltk_fluid_type_h

#include <fltk/Widget.h>
#include <fltk/Color.h>
#include <fltk/Preferences.h>
#include "Fluid_Plugins.h"
#include "Fluid_Image.h"

class FLUID_API FluidType {

  friend class Widget_Browser;
  friend fltk::Widget *make_type_browser(int,int,int,int,const char *l=0);
  friend class WindowType;
  virtual void setlabel(const char *); // virtual part of label(char*)

public:
  virtual void prefix(const char* p) {}
  virtual const char*  prefix() const {return 0;}
  virtual int pixmapID() {return -1;}

protected:

  FluidType();

  const char *name_;
  const char *label_;
  const char *tooltip_;
  const char *callback_;
  const char *user_data_;
  const char *user_data_type_;
  fltk::Widget *live_widget;

public:	// things that should not be public:

  FluidType* parent;
  FluidType* first_child;
  FluidType* next_brother;
  FluidType* previous_brother;

  FluidType* walk(const FluidType* top) const;
  FluidType* walk() const;

  static FluidType *first;

  FluidType *factory;
  const char *callback_name();

  bool open_;	// open/close state of this parent in browser

  static int selected_count() {return selected_count_;}
  bool selected() const {return selected_;}
  void selected(bool s) {
      if (s==selected_) return;
      if (s ) ++selected_count_; else --selected_count_; 
      selected_=s;
  }

  bool new_selected; // browser highlight
private:
  bool selected_; // copied here by selection_changed()
  static int selected_count_;

public:

  virtual ~FluidType();
  virtual FluidType *make() = 0;

  void add(FluidType *parent); // add as new child
  void insert(FluidType *n); // insert into list before n
  void remove(); // remove from list
  void move_before(FluidType*); // move before a sibling

  virtual const char *title(); // string for browser
  virtual const char *type_name() const = 0; // type for code output

  const char *name() const {return name_;}
  void name(const char *);
  const char *label() const {return label_;}
  void label(const char *);
  const char *tooltip() const {return tooltip_;}
  void tooltip(const char *);
  const char *callback() const {return callback_;}
  void callback(const char *);
  const char *user_data() const {return user_data_;}
  void user_data(const char *);
  const char *user_data_type() const {return user_data_type_;}
  void user_data_type(const char *);

  virtual FluidType* click_test(int,int);
  virtual void add_child(FluidType*, FluidType* beforethis);
  virtual void move_child(FluidType*, FluidType* beforethis);
  virtual void remove_child(FluidType*);

  static FluidType *current;  // most recently picked object
  virtual void open();	// what happens when you double-click

  // read and write data to a saved file:
  void write();
  virtual void write_properties();
  virtual void read_property(const char *);
  virtual int read_fdesign(const char*, const char*);

  // write code, these are called in order:
  virtual void write_static(); // write static stuff to .c file
  virtual void write_code();   // write .h and .c file

  // live mode
  virtual fltk::Widget *enter_live_mode(int top=0); // build widgets needed for live mode
  virtual void leave_live_mode(); // free allocated resources
  virtual void copy_properties(); // copy properties from this type into a potetial live object

  // fake rtti:
  virtual int is_parent() const; // makes it open/close in browser
  virtual int is_widget() const; // is an fltk::Widget
  virtual int is_button() const; // has true/false value()
  virtual int is_valuator() const; // double value(), range, step, etc
  virtual int is_menu_item() const; // can be put into menus
  virtual int is_group() const; // is an fltk::Group but not an fltk::Menu
  virtual int is_window() const; // is an outermost fltk::Window
  virtual int is_code_block() const;
  virtual int is_decl_block() const;
  virtual int is_comment() const;
  virtual int is_class() const;
  virtual int is_input() const; // has text() methods

  const char* member_of(bool need_nest = false) const;
  // codeview
  int code_line, header_line;
  int code_line_end, header_line_end;

  // constructor of a subclass by name:
  static FluidType* make(const char *type_name);
};

////////////////////////////////////////////////////////////////
// This header file also declares all the global functions in fluid:

// object list operations:
fltk::Widget *make_widget_browser(int x,int y,int w,int h);
void delete_all(int selected_only=0);
void selection_changed(FluidType* new_current);

// file operations:
FLUID_API void write_word(const char *);
FLUID_API void write_string(const char *,...);
FLUID_API int write_file(const char *, int selected_only = 0);
FLUID_API int write_code(const char *cfile, const char *hfile);

FLUID_API int write_declare(const char *, ...);
FLUID_API int is_id(char);
FLUID_API const char* unique_id(void* o, const char*, const char*, const char*);
FLUID_API void write_craw(const char* string);
FLUID_API void write_c(const char*, ...);
FLUID_API void write_h(const char*, ...);
FLUID_API void write_cstring(const char *);
FLUID_API void write_cstring(const char *,int length);
FLUID_API void write_carray(const char *);
FLUID_API void write_carray(const char *,int length);
FLUID_API void write_indent(int n);
FLUID_API void write_open(int);
FLUID_API void write_close(int n);
FLUID_API extern int write_number;
FLUID_API void write_public(int state); // writes pubic:/private: as needed
FLUID_API extern int indentation;
FLUID_API extern const char* indent();

FLUID_API int read_file(const char *, int merge);
FLUID_API const char *read_word(int wantbrace = 0);
FLUID_API void read_error(const char *format, ...);

extern double read_version;

// check legality of c code (sort of) and return error:
FLUID_API const char *c_check(const char *c, int type = 0);

// replace a string pointer with new value, strips leading/trailing blanks:
FLUID_API int storestring(const char *n, const char * & p, int nostrip=0);

FLUID_API extern bool include_H_from_C;
FLUID_API void select(FluidType* it, int value);
FLUID_API void select_only(FluidType *);
FLUID_API void refresh_browser_views();
FLUID_API void initialize_tab_colors();

extern int modflag;

#endif
//
// End of "$Id: FluidType.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
