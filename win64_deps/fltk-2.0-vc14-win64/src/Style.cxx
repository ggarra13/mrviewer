//
// "$Id: Style.cxx 8662 2011-05-15 06:04:24Z bgbnbigben $"
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

#include <fltk/Widget.h>
#include <fltk/string.h>
#include <fltk/math.h>
#include <fltk/run.h>
#include <fltk/draw.h>
#include <stdlib.h>
#include <config.h>
#include <ctype.h>

using namespace fltk;

/*! \class fltk::Style

  Each widget has a pointer to an instance of Style. Usually
  many widgets share pointers to the same one. Styles are linked
  into a hierarchy tree by the parent_ pointers.

  When you look up a value from a style (such as when box() is called)
  it looks at that style and each parent up until it finds a non-zero
  value to return, or until there are no more parents, in which case
  zero is returned. Thus changing a parent style can make global
  changes as long as widgets do not have local values set.

  When you "set" a style (such as by calling box(UP_BOX)) then the
  local member variable is set. Notice that by setting a zero value
  you will indicate that it should return the parent value.

  The method Widget::box(BOX) will create a dynamic() style for that
  widget, which is a child of the original style, and set the box in
  that style. This "unique" style is reused for any other changes to
  that widget and it is deleted when the widget is deleted. Thus
  changes to a single widget do not affect other widgets, but the
  majority of widgets all share a Style structure.

  Occasionally it is useful to see if a field has been set. To do this
  you can directly access the local member variables using names like box_.

*/

/*! \fn bool Style::dynamic() const
  True if this Style is unique to the Widget that owns it, and is not
  shared with other Widgets.
*/

// Do not change the contents of this ever.  The themes depend on getting
// a known state initially. Make sure the documentation below matches the
// default values!
static void revert(Style* s) {
  s->parent_		= 0;	// this is the topmost style always
  s->box_		= DOWN_BOX;
  s->buttonbox_		= UP_BOX;
  s->focusbox_		= DEFAULT_FOCUS_BOX;
  s->glyph_		= Widget::default_glyph;
  s->labelfont_		= HELVETICA;
  s->textfont_		= HELVETICA;
  s->labeltype_		= NORMAL_LABEL;
  s->color_		= WHITE; // GRAY99?
  s->alt_color_ 	= 0xE0E0E000;	
  s->textcolor_		= BLACK;
  s->selection_color_	= WINDOWS_BLUE;
  s->selection_textcolor_= WHITE;
  s->buttoncolor_	= GRAY75;
  s->labelcolor_	= BLACK;
  s->highlight_color_	= NO_COLOR;
  s->highlight_textcolor_= NO_COLOR;
  s->labelsize_		= 12;
  s->textsize_		= 12;
  s->leading_		= 2;
  s->scrollbar_width_	= 15;
  s->scrollbar_align_	= ALIGN_RIGHT|ALIGN_BOTTOM;
}

bool Style::hide_underscore_ = true;
bool Style::draw_boxes_inactive_ = true;
int Style::wheel_scroll_lines_ = 3;

/*! \fn Box* Style::box() const
  The type of box to draw around the outer edge of the widget (for the
  majority of widgets, some classes ignore this or use it to draw only
  text fields inside the widget). The default is fltk::DOWN_BOX.
*/

/*! \fn Box* Style::buttonbox()	const
  The type of box to draw buttons internal the widget (notice that
  fltk::Button uses box, however). The default is fltk::UP_BOX.
*/

/*! \fn Box* Style::focusbox() const;
  The type of box to draw when a widget takes focus. The default is
  fltk::DEFAULT_FOCUS_BOX
*/

/*! \fn Symbol* Style::glyph() const;

  A small image that some Widgets use to draw part of themselves.
  For instance the fltk::CheckButton class has this set to a Symbol
  that draws the white box and the checkmark if VALUE is true.

  Im most cases the fltk::drawflags() are examined to decide between
  different symbols. The default value draws empty squares and arrow
  buttons if ALIGN flags are on, see Widget::default_glpyh.

*/

/*! \fn Font* Style::labelfont() const;
  The font used to draw the label. Default is fltk::HELVETICA. 
*/

/*! \fn float Style::labelsize() const;
  Size of labelfont(). Default is 12. 
*/

/*! \fn Font* Style::textfont() const;
  Font to use to draw information inside the widget, such as the text
  in a text editor or menu or browser. Default is fltk::HELVETICA. 
*/

/*! \fn float Style::textsize() const;
  Size of textfont(). This is also used by many Widgets to control
  the size they draw the glyph(). Default is 12.
*/

/*! \fn float Style::leading() const;
  Extra spacing added between text lines or other things that are
  stacked vertically. The default is 2. The function fltk::drawtext() 
  will use the value from Widget::default_style, but text editors
  and browsers and menus and similar widgets will use the local value.
*/

/*! \fn LabelType* Style::labeltype() const;
  How to draw the label. This provides things like inset, shadow, and
  the symbols. fltk::NORMAL_LABEL.
*/

/*! \fn Color Style::color() const;

  Color of the widgets. The default is fltk::WHITE. You may think most
  widgets are gray, but this is because Group and Window have their
  own Style with this set to fltk::GRAY75, and also because many parts
  of widgets are drawn with the buttoncolor().

  If you want to change the overall color of all the gray parts of the
  interface you want to call fltk::set_background(color) instead, as this
  will set the entry for fltk::GRAY75 and also set the "gray ramp" so
  that the edges of buttons are the same color.
*/

/*! \fn Color Style::textcolor() const;
  Color to draw text inside the widget. Default is black. This is also
  used by many widgets to control the color when they draw the glyph(),
  thus it can control the color of checkmarks in fltk::CheckButton.
*/

/*! \fn Color Style::selection_color() const;
  Color drawn behind selected text in inputs, or selected browser or
  menu items, or lit light buttons. The default is fltk::WINDOWS_BLUE.
*/

/*! \fn Color Style::selection_textcolor() const;
  The color to draw text atop the selection_color. The default of zero
  indicates that fltk will choose a contrasting color (either the same
  as the original color or white or black). I recommend you use the
  default if possible.
*/

/*! \fn Color Style::highlight_color() const;

  The color to draw the widget when the mouse is over it (for
  scrollbars and sliders this is used to color the buttons). Depending
  on the widget this will either recolor the buttons that are normally
  colored with buttoncolor(), or will recolor the main area that is
  normally colored with color().

  The default value is zero, which indicates that highlighting is disabled.  */

/*! \fn Color Style::labelcolor() const;
  Color used to draw labels. Default is fltk::BLACK.  */

/*! \fn Color Style::highlight_textcolor() const;
  Color used to draw the labels or text when the background is drawn
  in the highlight_color. The default of zero indicates that fltk will
  choose a contrasting color (either the same as the original color or
  white or black). I recommend you use the default if possible.
*/

/*! \fn Color Style::buttoncolor() const;
  Color used when drawing buttons. Default is fltk::GRAY75.  */

/*! \fn unsigned char Style::scrollbar_align() const;
  Where to place scrollbars around a Browser or other scrolling widget.
  The default is fltk::ALIGN_RIGHT|fltk::ALIGN_BOTTOM.
*/

/*! \fn unsigned char Style::scrollbar_width() const;
  How wide the scrollbars are around a Browser or other scrolling widget.
  The default is 15.
*/

/*! \fn bool Style::hide_underscore() const;
  If false, draw &x in labels as an underscore. If true (the default)
  then the underscores are not drawn. In this case you should limit
  your &x hotkeys to menubar items, as underscores in buttons are not
  visible. The menubar will show them when Alt is held down.
*/

/*! \fn bool Style::draw_boxes_inactive() const
  If false then most of the built-in box types draw the same even if
  fltk::INACTIVE_R is passed to them. This repliates Windows appearance.
  If true (the default) then the boxes themselves gray out.
*/

/*! \fn int Style::wheel_scroll_lines() const
  How many lines to move for one click of a mouse wheel. The default
  is 3.
*/

NamedStyle* NamedStyle::first;

static NamedStyle default_named_style("default", ::revert, &Widget::default_style);
NamedStyle* Widget::default_style = &default_named_style;

/*! Copy the Style from another widget. Copying a style pointer from
  another widget is not safe if that
  style is dynamic() because it may change or be deleted.  This makes
  another dynamic() copy if necessary. For non-dynamic styles the
  pointer is copied.  */
bool Widget::copy_style(const Style* t) {
  if (style_ == t) return false;
  if (style_ && style_->dynamic()) delete (Style*)style_;
  if (!t->dynamic()) {style_ = t; return false;}
  Style* newstyle = new Style;
  newstyle->parent_ = (Style*)t;
  style_ = newstyle;
  return true;
}

// Widgets set their own attributes by (possibly) creating a unique copy
// of their current style and setting it there.  Because this copy does
// not have any children the recursive search is not needed:

static Style* unique_style(const Style* & pointer) {
  if (pointer->dynamic()) return (Style*)pointer;
  Style* newstyle = new Style();
  newstyle->dynamic_ = true;
  newstyle->parent_ = (Style*)pointer;
  pointer = newstyle;
  return newstyle;
}

// Retrieve/set values from a style, using parent's value if not in child:

#ifndef DOXYGEN
#define style_functions(TYPE,FIELD)	\
TYPE Widget::FIELD() const {return style()->FIELD();} \
TYPE Style::FIELD() const {		\
  for (const Style* s = this;;) {	\
    if (s->FIELD##_) return s->FIELD##_;\
    s = s->parent_;			\
    if (!s) return 0;			\
  }					\
}					\
void Widget::FIELD(TYPE v) {		\
  unique_style(style_)->FIELD##_ = v;	\
}

style_functions(Box*,	box		)
style_functions(Box*,	buttonbox	)
style_functions(Box*,	focusbox	)
style_functions(Symbol*,	glyph		)
style_functions(Font*,		labelfont	)
style_functions(Font*,		textfont	)
style_functions(LabelType*,	labeltype	)
style_functions(Color,		color		)
style_functions(Color,		textcolor	)
style_functions(Color,		selection_color	)
style_functions(Color,		selection_textcolor)
style_functions(Color,		buttoncolor	)
style_functions(Color,		labelcolor	)
style_functions(Color,		highlight_color	)
style_functions(Color,		highlight_textcolor)
style_functions(float,		labelsize	)
style_functions(float,		textsize	)
style_functions(float,		leading		)
style_functions(unsigned char,	scrollbar_align	)
style_functions(unsigned char,	scrollbar_width	)
#endif // !DOXYGEN

#if 0
/** Color to draw text atop the selection_color.

    This searches the styles for the first one that has selection_textcolor,
    textcolor, or selection_color set. If selection_textcolor is set it is
    returned.
    Otherwise if textcolor() is considered to "contrast" with
    selection_color() then return that.
    Otherwise return white or black, chosen to contrast with
    selection_color().
*/
Color Widget::selection_textcolor() const
{
  for (const Style* s = style_;;) {
    if (s->selection_textcolor) return s->selection_textcolor;
    if (s->textcolor) return contrast(s->textcolor, selection_color());
    if (s->selection_color) return contrast(textcolor(), s->selection_color);
    s = s->parent;
    if (!s) return 0;
  }
}
/** Set the value returned selection_textcolor(). This will create a
    new style() if necessary and set the field there.
*/
void Widget::selection_textcolor(Color v) {
  unique_style(style_)->selection_textcolor = v;
}

/** Color to draw text atop the highlight_color.

    This searches the styles for the first one that has highlight_textcolor
    or highlight_color set. If hightlight_textcolor is set it is returned.
    Otherwise if textcolor() is considered to "contrast" with
    highlight_color() then return that.
    Otherwise return white or black, chosen to contrast with
    highlight_color().
*/
Color Widget::highlight_textcolor() const
{
  for (const Style* s = style_;;) {
    if (s->highlight_textcolor) return s->highlight_textcolor;
    if (s->highlight_color) return contrast(textcolor(), s->highlight_color);
    s = s->parent;
    if (!s) return textcolor();
  }
}
/** Set the value returned highlight_textcolor(). This will create a
    new style() if necessary and set the field there.
*/
void Widget::highlight_textcolor(Color v) {
  unique_style(style_)->highlight_textcolor = v;
}
#endif

const Style* fltk::drawstyle_ = Widget::default_style;

/** \fn const Style* fltk::drawstyle()
  Return the last style sent to drawstyle(s,f). Some drawing functions
  (such as glyphs) look in this for box types. If this has not been
  called it is Widget::default_style.
*/

/**
  Draw using this style.  Set drawstyle() to this, drawflags() to \a
  flags, calls setcolor() and setbgcolor() with appropriate colors for
  this style and the given flags, and calls setfont().  This is called
  by the draw() methods on most fltk widgets. The calling Widget picks
  what flags to pass to the Symbols so that when they call this they
  get the correct colors for each part of the widget.

    Flags that are understood:
    - HIGHLIGHT: if highlight_color() is non-zero, set bg to
      highlight_color() and fg to highlight_textcolor().
    - OUTPUT: Normally color(), textcolor(), textfont(), and textsize()
      are used. If this flag is set buttoncolor(), labelcolor(),
      labelfont(), and labelsize() are used. Widgets will set
      this true for any internal buttons, but false for the
      main area.
    - INACTIVE_R: Change the fg to a gray color.

    It then further modifies fg so that it contrasts with the bg.
*/
void fltk::drawstyle(const Style* style, Flags flags) {
  drawstyle_ = style;
  drawflags_ = flags;
  // this is not correct! It should search the styles in order and
  // decide what to do as it searches. For instance highlight_textcolor
  // should only be used if set before or at the highlight_color style.
  Color bg, fg;
  if ((flags & HIGHLIGHT) && (bg = style->highlight_color())) {
    fg = contrast(style->highlight_textcolor(), bg);
  } else {
    if (flags & OUTPUT) {bg = style->buttoncolor(); fg = style->labelcolor();}
    else {bg = style->color(); fg = style->textcolor();}
    // fg = contrast(fg, bg);this messes up things
  }
  if (flags & INACTIVE_R) fg = inactive(fg,bg);
  setcolor(fg);
  setbgcolor(bg);
  if (flags & OUTPUT) setfont(style->labelfont(), style->labelsize());
  else setfont(style->textfont(), style->textsize());
}

/*! \class fltk::NamedStyle

  Typically a widget class will define a single NamedStyle that is
  used by all instances of that widget. A "theme" can locate this
  structure by looking it up by name (using the find() method) and
  then change it to change the appearance of all widgets of
  that class.

  The reason a string name is used, rather than making the style be
  a public static data member of the class, is so that a theme can
  modify a large number of types of widgets without having them
  all linked into a program. If find() returns null it should just skip
  that setting code because that widget is not used by this program.

  The "revert" function is mostly provided to make it easy to initialize
  the fields even though C++ does not allow a structure constant.
  It is also used to undo theme changes when fltk::reset_theme()
  is called.  */

static void plainrevert(Style*) {}

NamedStyle::NamedStyle(const char* n, void (*revert)(Style*), NamedStyle** pds) {
  memset((void*)this, 0, sizeof(*this));
  parent_ = Widget::default_style; // revert may want to change this
  if (revert) { revertfunc = revert; revert(this); }
  else revertfunc = plainrevert;
  next = NamedStyle::first;
  NamedStyle::first = this;
  back_pointer = pds;
  name = n;
}

/*! The constructor clears the style to entirely zeros, including the
  parent_ pointer. You probably want to set the parent to
  Widget::default_style in order to inherit the global settings.
*/
Style::Style() {
  memset((void*)this, 0, sizeof(*this));
}

Style* Style::find(const char* name) {
  for (NamedStyle* p = NamedStyle::first; p; p = p->next) {
    const char* a = p->name;
    const char* b = name;
    if (!a) continue;
    for (;;) {
      if (*a == '_') {
	if (*b == ' ' || *b == '_');
	else {a++; continue;}
      } else if (tolower(*a) != tolower(*b)) break;
      if (!*a && !*b) return p;
      a++;
      b++;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////
// Themes:

//extern "C" FL_API bool fltk_theme();

/*! \typedef Theme

  A Theme is a function called by fltk just before it shows the first
  window, and also whenever it receives a signal from the operating
  system that the user's preferences have changed.

  The Theme's job is to set all the NamedStyle structures to the
  correct values for the appearance selected by the user and
  operating system.

  The return value is ignored but you should return true for future
  compatability.

  This pointer is declared as a "C" function to make it easier to load
  the correct function by name from a plugin, if you would like to
  write a scheme where the appearance is controlled by plugins.  Fltk
  provides a convienence function to portably load plugins called
  fltk::load_plugin() that you may want to use if you are writing such
  a system.
*/

/*! \fn fltk::theme()
  Returns the current Theme function. By default this points at
  fltk_theme().
*/

/*! \fn fltk::theme(Theme)

  Change what function fltk should call to set the appearance. If you
  change this after any windows may have been shown, you should call
  reload_theme().
*/

Theme fltk::theme_ = fltk_theme;

Color fl_bg_switch = 0; // set by -bg in arg.cxx

static char theme_loaded;

/*!
  Call the theme() function if it has not already been called.
  Normally FLTK calls this just before the first
  Window::show() is done. You need to call this earlier to
  execute code such as measuring labels that may depend on the theme.
*/
void fltk::load_theme() {
  if (theme_loaded) return;
  theme_loaded = 2; // signal reset_theme to do nothing
  theme_();
  if (fl_bg_switch) set_background(fl_bg_switch);
  theme_loaded = 1;
}

/*!
  Does nothing if load_theme() has not been called yet. If
  load_theme() has been called, this calls the theme() function again
  and then call redraw(). If the theme function is written correctly,
  this should change the display to the new theme.  You should call
  this if you change the theme() or if external information changes
  such that the result of your theme() function changes.

  FLTK will call this automatically when it gets a message from the
  system indicating the user's preferences have changed.
*/
void fltk::reload_theme() {
  if (theme_loaded != 1) return;
  reset_theme();
  theme_();
  if (fl_bg_switch) set_background(fl_bg_switch);
  fltk::redraw();
}

/*! Change the theme to the compiled-in default by calling the revert
  function of all NamedStyle structures. A theme() function may want
  to call this to clear the previous settings.
*/
bool fltk::reset_theme() {
  // Don't waste time if the themes are just starting up:
  if (theme_loaded==2) return false;
  // revert to compiled defaults:
  // set_background((Color)0xc0c0c000); // not necessary?
  Style::draw_boxes_inactive_ = 1;
  for (NamedStyle* p = NamedStyle::first; p; p = p->next) {
    if (p->name) {
      const Style* savep = p->parent_;
      memset((void*)p, 0, sizeof(Style));
      p->parent_ = savep;
      p->revertfunc(p);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////

/*!
  fltk::GRAY75 is replaced with the passed color, and all the other
  fltk::GRAY* colors are replaced with a color ramp (or sometimes
  a straight line) so that using them for highlighted edges of
  raised buttons looks correct.
*/
void fltk::set_background(Color c) {
  uchar r, g, b;
  split_color( c, r, g, b );
  int i;
  int R, G, B;
  for (i = GRAY00; i <= GRAY99; i++) {
    if (i <= GRAY75) {
      R = r*(i-GRAY00)/(GRAY75-GRAY00);
      G = g*(i-GRAY00)/(GRAY75-GRAY00);
      B = b*(i-GRAY00)/(GRAY75-GRAY00);
    } else {
      const int DELTA = ((0xff-0xe0)*(i-GRAY75))/(GRAY99-GRAY75);
      R = r+DELTA; if (R > 255) R = 255;
      G = g+DELTA; if (G > 255) G = 255;
      B = b+DELTA; if (B > 255) B = 255;
    }
    set_color_index(Color(i), color(R,G,B));
  }
}

//
// End of "$Id: Style.cxx 8662 2011-05-15 06:04:24Z bgbnbigben $".
//
