// "$Id: Widget.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
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

#ifndef fltk_Widget_h
#define fltk_Widget_h

#include "Style.h"
#include "Rectangle.h"

namespace fltk {

class FL_API Widget;
class FL_API Window;
class FL_API Symbol;
class FL_API Group;
class FL_API AssociationType;
class FL_API AssociationFunctor;
struct Cursor;

typedef void (Callback )(Widget*, void*); /**< Callback is a function pointer to a fltk callback mechanism. It points to any function void foo(Widget*, void*) */
typedef Callback* Callback_p; /**< A pointer to a Callback. Needed for BORLAND */
typedef void (Callback0)(Widget*); /**< Function pointer to a callback with only one argument, the widget */
typedef void (Callback1)(Widget*, long); /**<Function pointer to a callback with a long argument instead of a void argument*/

#ifdef FLTK_1_WIDGET  // back-compatability section:
FL_API Font* font(int);
#endif

class FL_API Widget : public Rectangle {
  // disable the copy assignment/constructors:
  Widget & operator=(const Widget &);
  Widget(const Widget &);

public:

  Widget(int,int,int,int,const char* =0);
  virtual ~Widget();

  virtual void draw();
  virtual int handle(int);
  int	send(int event);
  virtual void layout();

  const Style* style() const { return style_; }
  void	style(const Style* s) { style_ = s; }
  void	style(const Style& s) { style_ = &s; }
  bool	copy_style(const Style* s);
  static NamedStyle* default_style;
  static Symbol* default_glyph;

  Group* parent() const	{ return parent_; }
  void	parent(Group* w)	{ parent_ = w; }
  Window* window() const	;

  enum WidgetVisualType {
    // Values for type() shared by Button and menu Item, and for fake RTTI:
    RESERVED_TYPE      = 0x64,
    TOGGLE    = RESERVED_TYPE+1,
    RADIO     = RESERVED_TYPE+2,
    GROUP_TYPE         = 0xe0,
    WINDOW_TYPE        = 0xf0
  };

  uchar	type() const		{ return type_; }
  void	type(uchar t)		{ type_ = t; }
  bool	is_group() const	{ return type_ >= GROUP_TYPE; }
  bool	is_window() const	{ return type_ >= WINDOW_TYPE; }

  bool  resize(int x,int y,int w,int h);
  bool	position(int x, int y)	;
  bool	resize(int w, int h)	;

  void  get_absolute_rect( Rectangle *rect ) const;

  const char* label() const	{ return label_; }
  void	label(const char* a);
  void	copy_label(const char* a);

  const Symbol* image() const	{ return image_; }
  void	image(const Symbol* a)	{ image_ = a; }
  void	image(const Symbol& a)	{ image_ = &a; }

  const char *tooltip() const	{ return tooltip_; }
  void	tooltip(const char *t)	{ tooltip_ = t; }

  unsigned shortcut() const	;
  void	shortcut(unsigned key)	;
  bool	add_shortcut(unsigned key);
  bool	remove_shortcut(unsigned key);
  void  remove_shortcuts()	;
  unsigned label_shortcut() const;
  bool	test_label_shortcut() const;
  bool	test_shortcut() const	;
  bool  test_shortcut(bool) const;

  Callback_p callback() const	{ return callback_; }
  void	callback(Callback* c, void* p) { callback_=c; user_data_=p; }
  void	callback(Callback* c)	{ callback_=c; }
  void	callback(Callback0*c)	{ callback_=(Callback*)c; }
  void	callback(Callback1*c, long p=0) { callback_=(Callback*)c; user_data_=(void*)p; }
  void*	user_data() const	{ return user_data_; }
  void	user_data(void* v)	{ user_data_ = v; }
  long	argument() const	{ return (long)user_data_; }
  void	argument(long v)	{ user_data_ = (void*)v; }
  uchar when() const		{ return when_; }
  void	when(uchar i)		{ when_ = i; }

  static void default_callback(Widget*, void*);
  void	do_callback()		{ callback_(this,user_data_); }
  void	do_callback(Widget* o,void* arg=0) { callback_(o,arg); }
  void	do_callback(Widget* o,long arg) { callback_(o,(void*)arg); }
  bool	contains(const Widget*) const;
  bool	inside(const Widget* o) const { return o && o->contains(this); }
  bool	pushed() const		;
  bool	focused() const		;
  bool	belowmouse() const	;

  Flags	flags() const		{ return flags_; }
  void	flags(Flags f)  	{ flags_ = f; }
  void	set_flag(unsigned f)	{ flags_ |= f; }
  void	clear_flag(unsigned f)	{ flags_ &= ~f; }
  void	invert_flag(unsigned f)	{ flags_ ^= f; }
  void	set_flag(unsigned f,bool b) { flags_ = (flags_&~f)|(b?f:0); }
  bool	flag(unsigned f) const	{ return (flags_ & f) != 0; }
  bool	any_of(unsigned f) const{ return (flags_ & f) != 0; }
  bool	all_of(unsigned f) const{ return (flags_ & f) == f; }

  bool	state() const 		{ return flag(STATE); }
  bool  state(bool);
  bool	set()			{ return state(true); }
  bool	clear()			{ return state(false); }
  void	setonly();

  Flags	align() const		{ return flags_&ALIGN_MASK; }
  void	align(unsigned a)	{ flags_ = (flags_ & (~ALIGN_MASK)) | a; }
  bool	visible() const		{ return !flag(INVISIBLE); }
  bool	visible_r() const	;
  void	show()			;
  void	hide()			;
  void	set_visible()		{ clear_flag(INVISIBLE); }
  void	clear_visible()		{ set_flag(INVISIBLE); }
  bool	active() const		{ return !flag(INACTIVE); }
  bool	active_r() const	;
  void	activate()		;
  void	activate(int b)		{ if (b) activate(); else deactivate(); }
  void	deactivate()		;
  bool	output() const		{ return flag(OUTPUT); }
  void	set_output()		{ set_flag(OUTPUT); }
  void	clear_output()		{ clear_flag(OUTPUT); }
  bool	takesevents() const	{ return !any_of(OUTPUT|INVISIBLE|INACTIVE); }
  bool	changed() const		{ return flag(CHANGED); }
  void	set_changed()		{ set_flag(CHANGED); }
  void	clear_changed()		{ clear_flag(CHANGED); }
  bool	selected() const	{ return flag(SELECTED); }
  void	set_selected()		{ set_flag(SELECTED); }
  void	clear_selected()	{ clear_flag(SELECTED); }
  bool	click_to_focus()	{ return flag(CLICK_TO_FOCUS); }
  void  set_click_to_focus()	{ set_flag(CLICK_TO_FOCUS); }
  void	clear_click_to_focus()	{ clear_flag(CLICK_TO_FOCUS); }
  bool	tab_to_focus()		{ return flag(TAB_TO_FOCUS); }
  void  set_tab_to_focus()	{ set_flag(TAB_TO_FOCUS); }
  void	clear_tab_to_focus()	{ clear_flag(TAB_TO_FOCUS|CLICK_TO_FOCUS); }
  bool  horizontal() const	{ return !flag(LAYOUT_VERTICAL);}
  bool	vertical() const	{ return flag(LAYOUT_VERTICAL);}
  void	set_horizontal()	{ clear_flag(LAYOUT_VERTICAL); }
  void	set_vertical()		{ set_flag(LAYOUT_VERTICAL); }

  bool	take_focus()		;
  void	throw_focus()		;

  void	redraw()		;
  void	redraw(uchar c)		;
  void	redraw_label()		;
  void	redraw_highlight()	;
  void	redraw(const Rectangle&);
  uchar	damage() const		{ return damage_; }
  void	set_damage(uchar c)	{ damage_ = c; } // should be called damage(c)

  void  relayout()		;
  void	relayout(uchar damage)	;
  uchar layout_damage() const	{ return layout_damage_; }
  void	layout_damage(uchar c)	{ layout_damage_ = c; }

  void	add_timeout(float)	;
  void	repeat_timeout(float)	;
  void  remove_timeout()	;

  void	make_current() const	;
  void	draw_background() const	;
  void  draw_frame() const	;
  void  draw_box() const	;
  void  draw_box(const Rectangle& r) const ; // multiple boxes drawing for a single Widget
  void	draw_label() const	;
  void  draw_label(const Rectangle&, Flags) const ;
  void  draw_glyph(int, const Rectangle&) const ;
  void	cursor(Cursor*) const	;

  void	measure_label(int&, int&) const ;

  Box*	box()			const;
  Box*	buttonbox()		const;
  Box*  focusbox()		const;
  Symbol* glyph()		const;
  Font*	labelfont()		const;
  Font*	textfont()		const;
  LabelType* labeltype()	const;
  Color	color()			const;
  Color	textcolor()		const;
  Color	selection_color()	const;
  Color	selection_textcolor()	const;
  Color	buttoncolor()		const;
  Color	labelcolor()		const;
  Color	highlight_color()	const;
  Color	highlight_textcolor()	const;
  float labelsize()		const;
  float textsize()		const;
  float leading()		const;
  unsigned char scrollbar_align() const;
  unsigned char scrollbar_width() const;

  void box(Box*)		;
  void buttonbox(Box*)		;
  void focusbox(Box*)		;
  void glyph(Symbol*)		;
  void labelfont(Font*)		;
  void textfont(Font*)		;
  void labeltype(LabelType*)	;
  void color(Color)		;
  void textcolor(Color a)	;
  void selection_color(Color)	;
  void selection_textcolor(Color);
  void buttoncolor(Color)	;
  void labelcolor(Color)	;
  void highlight_color(Color)	;
  void highlight_textcolor(Color);
  void labelsize(float a)	;
  void textsize(float a)	;
  void leading(float a)		;
  void scrollbar_align(unsigned char);
  void scrollbar_width(unsigned char);

  void  add(const AssociationType&, void* data);
  void  set(const AssociationType&, void* data);
  void* get(const AssociationType&) const;
  void* foreach(const AssociationType&, AssociationFunctor&) const;
  bool  remove(const AssociationType&, void* data);
  bool  find(const AssociationType&, void* data) const;

#ifdef FLTK_1_WIDGET  // back-compatability section:

  Box*	down_box()		const { return box(); }
  Box*	slider()		const { return buttonbox(); }
  Box*	box2()			const { return box(); }
  Box*	fly_box()		const { return box(); }
  Color	color2()		const { return selection_color(); }
  Color	color3()		const { return buttoncolor(); }
  Color	down_labelcolor()	const { return selection_textcolor(); }
  Color	fly_color()		const { return highlight_color(); }
  Color	selected_textcolor()	const { return selection_textcolor(); }
  Color	cursor_color()		const { return selection_color(); }
  float text_size()		const { return textsize(); }
  float label_size()		const { return labelsize(); }

  void down_box(Box* a)		{ box(a); }
  void slider(Box* a)		{ buttonbox(a); }
  void fly_box(Box*)		{ }
  void color(Color a, Color b)	{ color(a); selection_color(b); }
  void color2(Color a)		{ selection_color(a); }
  void color3(Color a)		{ buttoncolor(a); }
  void down_labelcolor(Color a)	{ selection_textcolor(a); }
  void labelfont(unsigned a)	{ labelfont(font(a)); }
  void fly_color(Color a)	{ highlight_color(a); }
  void textfont(unsigned a)	{ textfont(font(a)); }
  void selected_textcolor(Color a) { selection_textcolor(a); }
  void cursor_color(Color a)	{ selection_color(a); }
  void text_size(float n)	{ textsize(n); }
  void label_size(float n)	{ labelsize(n); }

#endif

private:

  const char*		label_;
  const Symbol*		image_;
  unsigned		flags_;
  const Style*		style_;
  Callback*		callback_;
  void*			user_data_;
  const char*		tooltip_; // make this into another widget?
  Group*		parent_;
  uchar			type_;
  uchar			damage_;
  uchar			layout_damage_;
  uchar			when_;

};

/**Widget::when() values*/
enum {
  WHEN_NEVER		= 0, /**< Never triggers the callback*/
  WHEN_CHANGED		= 1, /**< Triggers the callback when the data has changed */
  WHEN_RELEASE		= 4, /**< Triggers the callback when the data has changed and the mouse button has been released */
  WHEN_RELEASE_ALWAYS	= 6, /**< Triggers the callback when the mouse button has been released, regardless of the data's state*/
  WHEN_ENTER_KEY	= 8, /**< Triggers the callback when the user hits the Enter key and the data has changed*/
  WHEN_ENTER_KEY_ALWAYS	=10, /**< Triggers the callback when the user hits the Enter key, regardless of the data's state */
  WHEN_ENTER_KEY_CHANGED=11, /**< Triggers the callback either when the data has changed or the user hits the Enter key */
  WHEN_NOT_CHANGED	= 2  /**< Modifier bit to disable changed() test */
};

}

#endif

//
// End of "$Id: Widget.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
