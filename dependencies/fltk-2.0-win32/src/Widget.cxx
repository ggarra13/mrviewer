//
// "$Id: Widget.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Base widget class for the Fast Light Tool Kit (FLTK).
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
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/Group.h>
#include <fltk/run.h>
#include <fltk/string.h> // for newstring
#include <stdlib.h> // for free
#include <config.h>

using namespace fltk;

/*! \class fltk::Widget

  The base class for all widgets in FLTK. The basic Widget draws an
  empty box() and the label(), and ignores all events. This can be
  useful for making decorations or providing areas that pop up a
  tooltip().
*/

/*! This is the initial value for callback(). It does set_changed()
  on the widget, thus recording the fact that the callback was done.
  Do not set the callback to zero, use this if you want no action.
*/
void Widget::default_callback(Widget* w, void*) {w->set_changed();}

/*! Standard constructor for a widget.
  The default constructor takes a value for x(), y(), w(), and h(),
  and an optional value for label(). All subclasses must provide an
  identical constructor in order to work with Fluid. They may also
  provide alternative constructors.

  If Group::begin() has been called, this widget is added as a new
  child of that group, and parent() is set to the group. If
  Group::begin() has not been called, or Group::end() has been called,
  or Group::current(0), then the parent() is set to null. In this case
  you must add the widget yourself in order to see it.
*/
Widget::Widget(int X, int Y, int W, int H, const char* L) :
  Rectangle(X,Y,W,H)
{
  style_	= default_style;
  parent_	= 0;
  callback_	= default_callback;
  user_data_	= 0;
  label_	= L;
  image_	= 0;
  tooltip_	= 0;
#if CLICK_MOVES_FOCUS
  flags_	= CLICK_TO_FOCUS | TAB_TO_FOCUS;
#else
  flags_	= TAB_TO_FOCUS;
#endif
  type_		= 0;
  damage_	= DAMAGE_ALL;
  layout_damage_= LAYOUT_DAMAGE;
  when_		= WHEN_RELEASE;
  if (Group::current()) Group::current()->add(this);
}

extern void delete_associations_for(Widget* widget); // in WidgetAssociation.cxx

/*! The destructor is virtual. The base class removes itself from the
  parent widget (if any), and destroys any label made with copy_label().
*/
Widget::~Widget() {
  remove_timeout();
  if (parent_) parent_->remove(this);
  throw_focus();
  delete_associations_for(this);
  if (style_->dynamic()) {
    // When a widget is destroyed it can destroy unique styles:
    delete (Style*)style_; // cast away const
  }
  if (flags_&COPIED_LABEL) delete[] const_cast<char*>( label_ );
}

/*! \fn Group* Widget::parent() const
  Returns a pointer to the parent widget.  Usually this is a
  fltk::Group or fltk::Window.  Returns NULL if none.
*/

/*! Returns true if \a b is a child of this widget, or is equal to
  this widget. Returns false if \a b is NULL. */
bool Widget::contains(const Widget* b) const {
  for (; b; b = b->parent_) if (b == this) return true;
  return false;
}

/*! Fills the Rectangle pointed to by \a rect with the widget's
  rectangle expressed in absolute (i.e. screen) coordinates.
*/
void Widget::get_absolute_rect( Rectangle *rect ) const
{
  if ( rect ) {
    int ax = x();
    int ay = y();
    Widget *widget = parent_;
    while ( widget ) {
      ax += widget->x();
      ay += widget->y();
      widget = widget->parent_;
    }
    rect->x( ax );
    rect->y( ay );
    rect->w( w() );
    rect->h( h() );
  }
}

/*! bool Widget::inside(const Widget* a) const
  Returns true if this is a child of a, or is equal to a. Returns
  false if a is NULL. */

/*! Sets the label directly to a string.
  The label is printed somewhere on the widget or next to it. The
  string passed to label() is \e not copied, instead the pointer to
  the string is stored. If copy_label()
  was called earlier the old string's memory is freed.
*/
void Widget::label(const char* s) {
  if (label_ == s) return; // Avoid problems if label(label()) is called
  if (flags_&COPIED_LABEL) {
    delete[] const_cast<char*>( label_ );
    flags_ &= ~COPIED_LABEL;
  }
  label_ = s;
}

/*! Sets the label to a copy of the string.
  The passed string is copied to private storage and used to set the
  label(). The memory will be freed when the widget is destroyed or when
  copy_label() is called again, or label(const char*) is called.

  Passing NULL will set label() to NULL.
*/
void Widget::copy_label(const char* s) {
  if (label_ == s) return; // Avoid problems if label(label()) is called
  if (flags_&COPIED_LABEL) delete[] const_cast<char*>( label_ );
  label_ = newstring(s);
  flags_ |= COPIED_LABEL;
}

/*! void Widget::image(Image*)
  Sets the image. The image is drawn as part of the label, usually to
  the left of the text. This is designed for icons on menu items. If
  you want to replace the entire background of the widget with a picture
  you should set box() instead. Notice that you can also get images into
  labels by putting '@' commands into the label().
*/

/*! void Widget::tooltip(const char*)
  Set the string used as the pop-up tooltip. The pointer to the passed string
  is stored, it is not copied! Passing null indicates that the tooltip
  of the parent() should be used (or no
  tooltip if no parent has one). If you want to disable the tooltip but
  let the parent have one, set this tooltip to <tt>""</tt>.

  Putting '@' commands in to bring up Symbol objects will allow a lot
  of interesting things to be put into the tooltip.
*/

/*! \fn void Widget::callback(fltk::Callback*, void*)
  Each widget has a single callback.  You can set it or examine it with
  these methods. The callback is called with the widget as the first
  argument and the void* as the second argument. It is called in response
  to user events, but exactly whe depends on the widget. For instance a
  button calls it when the button is released.
*/

/*! \fn void Widget::user_data(void*)
  Set the second argument to the callback.
*/

/*! \fn void Widget::callback(fltk::Callback1*, long = 0)
  For convenience you can also define the callback as taking a long
  integer argument.  This is implemented by casting the function to a
  fltk::Callback and casting the <tt>long</tt> to a <tt>void*</tt> and
  may not be portable to some machines.
*/

/*! \fn void Widget::argument(long)
  Sets the second argument to the callback to a number. This is done
  by casting the long to a void*.
*/

/*! \fn void Widget::callback(fltk::Callback0*)
  For convenience you can also define the callback as taking only the
  Widget as an argument.  This is implemented by casting this to a
  fltk::Callback and may not be portable to some machines.
*/

/*! \fn void Widget::do_callback()
  You can cause a widget to do its callback at any time. The callback
  function is called with this and user_data() as arguments.
*/

/*! \fn void Widget::do_callback(Widget*, void*)
  You can also call the callback function with arbitrary arguments. */

/*! \fn void Widget::do_callback(Widget*, long)
  You can also call the callback function with arbitrary arguments. */

/*! \fn uchar Widget::type() const

  8-bit identifier that controls how widget works.
  This value had to be provided for Forms compatibility, but
  you can use it for any purpose you want (mostly for "bad object
  oriented programming" where you insert some subclass functionality
  into the base class). Widget subclasses may store values in the
  range 0-99 here (larger values are reserved for use by FLTK).

  The fltk::PackedGroup widget uses the low bit of the type() of
  each child to indicate HORIZONTAL (1) or VERTICAL (0).

  For portability FLTK does not use RTTI (Run Time Typing Infomation)
  internally (you are free to use it, though). If you don't have RTTI you can
  use the clumsy FLTK mechanisim, by having type() use a unique
  value.  These unique values must be greater than the symbol
  Widget::RESERVED_TYPE (which is 100).  Look through the header
  files for Widget::RESERVED_TYPE to find an unused number.  If you
  make a subclass of fltk::Window you must use Widget::WINDOW_TYPE+n
  (\e n must be in the range 1 to 7) so that is_window() will work, if you
  make a subclass of fltk::Group you must use Widget::GROUP_TYPE+n
  (\e n must be in the range 1 to 7) so that is_group() will work.  */

/*! \fn bool Widget::is_group() const
  Returns true for subclasses of fltk::Group. If so you can cast it to
  a group with <tt>(fltk::Group*)(widget)</tt>. This is done by using
  type(), but if your compiler supports RTTI you may want to safer
  <tt>dynamic_cast&lt;fltk::Group*&gt;(widget)</tt>.
*/

/*! \fn bool Widget::is_window() const
  Returns true for subclasses of fltk::Window. If so you can cast it
  to a window with <tt>(fltk::Window*)(widget)</tt>. This is done by
  using type(), but if your compiler supports RTTI you may want to safer
  <tt>dynamic_cast&lt;fltk::Window*&gt;(widget)</tt>. If this is true,
  is_group() is also true.
*/

/*! \fn Flags Widget::flags() const
  Each Widget, and most drawing functions, take a bitmask of
  flags that indicate the current state and exactly how to draw
  things. See \link Flags.h <fltk/Flags.h> \endlink for values.
  This is for copying the flag values, use flag(c) to test them.
*/

/*! \fn void Widget::flags(Flags f)
  Replace flags(). This is for constructors, don't use it elsewhere.
*/

/*! \fn void Widget::set_flag(unsigned f)
  Make flag(f) return true by turning on that bit. You can set
  more than one by or'ing them together.
*/

/*! \fn void Widget::clear_flag(unsigned f)
  Make flag(f) return false by turning off that bit. You can turn
  off multiple bits by or'ing them toegher.
*/

/*! \fn void Widget::set_flag(unsigned f,bool b)
  Make flag(f) return \a b. Same as b ? set_flag(f) : clear_flag(f).
*/

/*! \fn void Widget::invert_flag(unsigned f)
  Flip the result of flag(f) if \a f is a single bit. If you or
  together bits it will flip them all.
*/

/*! \fn bool Widget::flag(unsigned f) const
  Returns true if the bit for \a f is on.
*/

/*! \fn bool Widget::any_of(unsigned f) const
  Returns true if \e any of the bits in \a f are on. Actually this
  is the same function as flag(f) but using this may make the code
  more readable.
*/

/*! \fn bool Widget::all_of(unsigned f) const
  Returns true if \e all of the bits in \a f are on. \a f
  should be several bits or'd together, otherwise this is the
  same as flag(f).
*/

/*! \fn void Widget::align(Flags);
  Forces the values of all the fltk::ALIGN_* flags to the passed
  value. This determines how the label is printed next to or inside
  the widget. The default value is fltk::ALIGN_CENTER, which centers
  the label. The value can be any of the ALIGN flags or'd together.
*/

/*! \fn void Widget::when(uchar)

  Flags indicating when to do the callback(). This field is in the
  base class so that you can scan a panel and do_callback() on all the
  ones that don't do their own callbacks in response to an "OK"
  button.

  The following constants can be used, their exact meaning depends on
  the widget's implementation:
  - fltk::WHEN_NEVER - Never call the callback (0).
  - fltk::WHEN_CHANGED - Do the callback each time the widget's value
  is changed by the user (many callbacks may be done as the user drags
  the mouse)
  - fltk::WHEN_RELEASE - Each keystroke that modifies the value, or
  when the mouse is released and the value has changed, causes the
  callback (some widgets do not implement this and act like
  fltk::WHEN_CHANGED)
  - fltk::WHEN_RELEASE_ALWAYS - Each recognized keystroke and the
  mouse being released will cause the callback, even if the value did
  not change. (some widgets do not implement this and act like
  fltk::WHEN_RELEASE)
  - fltk::WHEN_ENTER_KEY - Do the callback when the user presses the
  ENTER key and the value has chagned (used by fltk::Input and
  fltk::Browser.
  - fltk::WHEN_ENTER_KEY_ALWAYS - Do the callback when the user
  presses the ENTER key, even if the value has not changed.
  -fltk::WHEN_ENTER_KEY_CHANGED - Do the callback when the user
  presses the ENTER key and each time the value changes.

*/

////////////////////////////////////////////////////////////////
// LAYOUT

/**
  Virtual function to respond to layout_damage(), it should
  calculate the correct size of this widget and all it's children.
  This function is called by fltk or by the layout() method in other
  widgets. User programs should not call it.

  A widget is allowed to alter it's own size in a layout() method, to
  indicate a size that the data will fit in. A parent widget is then
  expected to rearrange itself to accomodate the new size. This may
  mean it will move the widget and thus layout() will be called again.

  You can look at layout_damage() to find out why this is being called.

  The base class redraws the widget.
*/
void Widget::layout() {
  if (layout_damage_&~LAYOUT_CHILD) redraw();
}

/*! Change the size or position of the widget. Nothing is done if the
  passed size and position are the same as before. If there is a
  change then relayout() is called so that the virtual function
  layout() is called before the next draw().  */
bool Widget::resize(int x,int y,int w,int h) {
  uchar flags = 0;
  if (x != this->x()) flags = LAYOUT_X;
  if (y != this->y()) flags |= LAYOUT_Y;
  if (w != this->w()) flags |= LAYOUT_W;
  if (h != this->h()) flags |= LAYOUT_H;
  if (flags) {
    Rectangle::set(x,y,w,h);
    // parent must get LAYOUT_DAMAGE as well as LAYOUT_CHILD:
    if (parent()) {
      layout_damage_ |= flags;
      parent()->relayout(LAYOUT_DAMAGE|LAYOUT_CHILD);
    } else {
      relayout(flags);
    }
    return true;
  } else {
    return false;
  }
}

/*! Same as resize() to x, y, w(), h() */
bool Widget::position(int x, int y) {
  return resize(x, y, w(), h());
}

/*! Same as resize() to x(), y(), w, h */
bool Widget::resize(int w, int h) {
  return resize(x(), y(), w, h);
}

/*! Same as relayout(LAYOUT_DAMAGE), indicates that data inside the
  widget may have changed, but the size did not change. This flag is
  also on when the widget is initially created.
*/
void Widget::relayout() {
  relayout(LAYOUT_DAMAGE);
}

/* Cause layout() to be called later. Turns on the specified flags in
   layout_damage(), and turns on LAYOUT_CHILD in all parents of this
   widget. \a flags cannot be zero, the maaning of the flags is listed
   under \link layout.h <fltk/layout.h> \endlink.
*/
void Widget::relayout(uchar flags) {
  //if (!(flags & ~layout_damage_)) return;
  layout_damage_ |= flags;
  for (Widget* w = this->parent(); w; w = w->parent())
    w->layout_damage_ |= LAYOUT_CHILD;
  fltk::damage(1); // make flush() do something
}

/*! \fn uchar Widget::layout_damage() const

  The 'or' of all the calls to relayout() or resize() done since the
  last time layout() was called.

  A typical layout function does not care about the widget moving, an
  easy way to skip it is as follows:

\code
MyClass::layout() {
  if (!(layout_damage() & ~LAYOUT_XY)) return;
  do_expensive_layout();
  redraw();
}
\endcode
*/

/*! \fn void Widget::layout_damage(uchar c)
  Directly change the value returned by layout_damage(). */

/*! \fn bool Widget::horizontal() const
  Return true if this widget has a horizontal orientation and fltk::Pack
  will position it against the top or bottom edge. This is the default.
*/

/*! \fn bool Widget::vertical() const
  Same as !horizontal().
*/

/*! \fn void Widget::set_vertical()
  Makes vertical() return true. This will affect how a surrounding
  fltk::Pack (or similar group) will place the widget, but you must
  call relayout() to indicate that this must be recalculated.

  Some widgets classes such as fltk::MenuBar or fltk::Slider will draw
  differently if this is turned on, in a vertical arrangement.
*/

/*! \fn void Widget::set_horizontal()
  Undoes set_vertical() and makes horizontal() return true. This will
  affect how a surrounding fltk::Pack (or similar group) will place
  the widget, but you must call relayout() to indicate that this must
  be recalculated.
*/

////////////////////////////////////////////////////////////////
// REDRAWING

/*! \var fltk::DAMAGE_CHILD
  A child of this group widget needs to be
  redrawn (non-group widgets can use this bit for their own purposes).
*/
/*! \var fltk::DAMAGE_CHILD_LABEL
  An outside label of this widget needs
  to be redrawn. This is handled (and this bit is cleared) by the
  parent group.

  Because anti-aliasing cannot be redrawn atop itself, this is not
  used anymore. Instead if an outside label needs to change the entire
  parent widget is redrawn.
*/
/*! \var fltk::DAMAGE_EXPOSE
  Damage caused by damage() or by expose
  events from the operating system. If this and fltk::DAMAGE_ALL is on
  the widget should draw every pixel inside it's region.
*/
/*! \var fltk::DAMAGE_ALL
  This bit is set by redraw() and indicates that
  all of the widget (but not "holes" where the background shows
  through) needs to be redraw.
*/
/*! \var fltk::DAMAGE_VALUE
  A widget may use this to indicate that the displayed value has changed. */
/*! \var fltk::DAMAGE_PUSHED
  A widget may use this to indicate that the user has pushed or released
  a button. */
/*! \var fltk::DAMAGE_SCROLL
  A widget may use this to indicate that
  the displayed data has scrolled moved horizontally and/or vertically. */
/*! \var fltk::DAMAGE_OVERLAY
  Same value as fltk::DAMAGE_SCROLL. */
/*! \var fltk::DAMAGE_HIGHLIGHT
  A widget may use this to indicate that
  the mouse has entered/exited part of the widget. */
/*! \var fltk::DAMAGE_CONTENTS
  Same as fltk::DAMAGE_EXPOSE but if fltk::DAMAGE_ALL is off a widget can
  use this for it's own purposes. */

/*! \fn uchar Widget::damage() const

  The 'or' of all the calls to redraw() done since the last
  draw(). Cleared to zero after draw() is called.
*/

/*! \fn void Widget::set_damage(uchar c)

  Directly change the value returned by damage(). Note that this
  \e replaces the value, it does not turn bits on. Use redraw()
  to turn bits on.
*/

/*

New scheme to reduce the expense of calling redraw() on widgets.
The loss here is that if a widget is changed and also gets exposed, it
will be drawn twice. The gain is that region calculations are not done
on every redraw(), restoring the speed we had in earlier fltk.

I renamed damage(n) to redraw(n) to match fltk method style. damage(n)
is reserved for changing damage_ but for now use set_damage(n) for this.

Normal redraw(n) will turn on those damage bits in the widget and turn
on DAMAGE_CHILD in all the parents. Groups should call redraw_child()
on each child when DAMAGE_CHILD is on.

Expose events accumulates a region but does not turn on any damage
bits.  The flush() method on a window must cause this area to be
updated. This may be done by copying a back buffer. The normal window
calls draw() again with the clip region set and damage() set to
DAMAGE_EXPOSE. All widgets should redraw every pixel in the clip
region if DAMAGE_EXPOSE is turned on. Exposure that covers the
entire window is detected and changed into DAMAGE_ALL, so only one
redraw is done.

The redraw(x,y,w,h) function does the same thing as expose events but
it sets DAMAGE_EXPOSE. This is so flush() for double-buffered
windows can tell these apart from normal expose events.

*/

/*! Same as redraw(DAMAGE_ALL). This bit is used by most widgets to
  indicate that they should not attempt any incremental update, and
  should instead completely draw themselves.
*/
void Widget::redraw() {
  redraw(DAMAGE_ALL);
}

/*! Indicates that draw() should be called, and turns on the given
  bits in damage(). At least these bits, and possibly others, will
  still be on when draw() is called. */
void Widget::redraw(uchar flags) {
  if (!(flags & ~damage_)) return;
  damage_ |= flags;
  for (Widget* widget = parent(); widget; widget = widget->parent())
    widget->damage_ |= DAMAGE_CHILD;
  fltk::damage(1); // make flush() do something
}

/*! Indicates that the label() should be redrawn. This does nothing
  if there is no label. If it is an outside label (see align()) then
  the parent() is told to redraw it. Otherwise redraw() is called.
*/
void Widget::redraw_label() {
  if (!label() && !image()) return;
  // inside label redraws the widget:
  if (!(flags()&15) || flag(ALIGN_INSIDE)) redraw();
#if 0
  // outside label requires a marker flag and damage to parent:
  // This does not work for antialiased labels...
  else redraw(DAMAGE_CHILD_LABEL);
#else
  // Find the parent with a box and redraw it...
  else {
    Widget* window = (Widget*)(this->window());
    if (window) window->redraw();
  }
#endif
}

/** Causes a redraw if highlighting changes.

    Calls redraw(DAMAGE_HIGHLIGHT) if this widget
    has a non-zero highlight_color(). This is designed to be
    called in response to ENTER and EXIT events and not redraw
    the widget if the no highlight color is being used.
*/
void Widget::redraw_highlight() {
  if (active() && highlight_color() && box()!=NO_BOX)
    redraw(DAMAGE_HIGHLIGHT);
}

// If a draw() method sets this then the calling group assummes it
// modified the current clip region to clip out the area covered by
// the widget.
extern Widget* fl_did_clipping;

/*!
  Fltk calls this virtual function to draw the widget, after setting
  up the graphics (current window, xy translation, etc) so that any
  drawing functions will go into this widget.

  User code should not call this! You probably want to call redraw().

  The default version calls draw_box() and draw_label(), thus drawing
  the box() to fill the widget and putting the label() and image()
  inside it to fill it, unless the align() flags are set to put it
  outside.

  Information on how to write your own version is <a
  href=subclassing.html#draw>here</a>.
*/
void Widget::draw()
{
  clear_flag(HIGHLIGHT);
  if (box() == NO_BOX) {
    // check for completely blank widgets. We must not clip to their
    // area because it will break lots of programs that assumme these
    // can overlap any other widgets:
    if (!image() &&
	(!label() || (align() != ALIGN_CENTER && !(align()&ALIGN_INSIDE)))) {
      fl_did_clipping = this;
      return;
    }
    // draw the background behind the invisible widget:
    draw_background();
  } else {
    draw_box();
  }
  draw_label();
}

////////////////////////////////////////////////////////////////

/**
  Handle an event. Returns non-zero if the widget understood and used
  the event.

  The event numbers are listed in \link events.h <fltk/events.h>
  \endlink.  All other information about the current event (like mouse
  position) is accessed by various functions listed in the same header
  file.

  The default version returns true for fltk::ENTER and fltk::MOVE
  events, this is done so you can put tooltips on the base widget. All
  other events return zero.

  If you want to send an event to a widget you probably want to call
  send(), not handle(). Send will do extra work with each event before
  calling this, such as turning HIGHLIGHT and FOCUSED flags on/off.
*/
int Widget::handle(int event) {
  switch (event) {
  case ENTER:
    redraw_highlight();
    // fall through to MOVE:
  case MOVE:
    // This calls belowmouse() directly do this works as the fallback
    // for Group::handle(), and also so it somewhat works if called
    // directly rather than from send():
    fltk::belowmouse(this);
    return true;
  case DND_DRAG:
    fltk::belowmouse(this);
    return false;
  case LEAVE:
    redraw_highlight();
    return true;
  default:
    return 0;
  }
}

/*! Wrapper for handle(). This should be called to send
  events. It does a few things:
  - It calculates event_x()/event_y() to be relative to the widget.
    The previous values are restored before this returns.
  - It makes sure the widget is active and/or visible if the event
    requres this.
  - If this is not the fltk::belowmouse() widget then it changes
    fltk::MOVE into fltk::ENTER and turns fltk::DND_DRAG into
    fltk::DND_ENTER. If this \e is the fltk::belowmouse() widget then
    the opposite conversion is done.
  - For move, focus, and push events if handle() returns true it sets
    the fltk::belowmouse() or fltk::focus() or fltk::pushed() widget to
    reflect this.
*/
int Widget::send(int event) {

  int dx = x(); int dy = y();

  for (Widget* p = parent(); p; p = p->parent()) {
    // we may want to ignore hiearchy parents in a browser. Not figured
    // out how to do this yet.
    dx += p->x();
    dy += p->y();
  }
  int save_x = e_x;
  int save_y = e_y;
  e_x = e_x_root-dx;
  e_y = e_y_root-dy;

  int ret = 0;
  switch (event) {

  case FOCUS:
    if (!takesevents()) break;
    ret = handle(event);
    if (ret) {
      set_flag(FOCUSED);
      // If it returns true then this is the focus() widget, but only
      // set this if handle() did not pass this on to a child:
      if (!contains(fltk::focus())) fltk::focus(this);
    }
    break;

  case ENTER:
  case MOVE:
    if (!visible()) break;
    if (active()) set_flag(HIGHLIGHT);
    // figure out correct type of event:
    event = (contains(fltk::belowmouse())) ? MOVE : ENTER;
    ret = handle(event);
    // Set belowmouse only if handle() did not set it to some child:
    if (!contains(fltk::belowmouse())) fltk::belowmouse(this);
    break;

  case LEAVE:
  case DND_LEAVE:
    clear_flag(HIGHLIGHT);
    ret = handle(event);
    break;

  case DND_ENTER:
  case DND_DRAG:
    if (!takesevents()) break;
    set_flag(HIGHLIGHT);
    // figure out correct type of event:
    event = (contains(fltk::belowmouse())) ? DND_DRAG : DND_ENTER;
    // see if it wants the event:
    ret = handle(event);
    // Set belowmouse only if handle() did not set it to some child:
    if (!contains(fltk::belowmouse())) fltk::belowmouse(this);
    break;

  case PUSH:
    if (!takesevents()) {
      // inactive (but not invisible/output) widgets eat the mouse clicks:
      if (!(flags_&(OUTPUT|INVISIBLE))) ret = 1;
      break;
    }
    // see if it wants the event:
    ret = handle(event);
    if (ret) {
      // If it returns true then this is the pushed() widget. But we
      // only set this if handle() did not pass this on to a child,
      // and if the mouse is still down:
      // Widgets with click_to_focus flag on will get the focus if
      // they accept the mouse push.
      if (event_state(0x0f000000) && !contains(fltk::pushed())) {
	fltk::pushed(this);
	if (click_to_focus()) take_focus();
      }
    }
    break;

  case SHOW:
    if (visible()) handle(event);
    break;

  case HIDE:
    if (visible()) {throw_focus(); handle(event);}
    break;

  case ACTIVATE:
    if (active()) {clear_flag(INACTIVE_R); handle(event);}
    break;

  case DEACTIVATE:
    if (active()) {throw_focus(); set_flag(INACTIVE_R); handle(event);}
    break;

  case SHORTCUT:
    // This is sent even if the child is invisible
    if (active()) ret = handle(event);
    break;

  default:
    if (takesevents()) ret = handle(event);
    break;
  }

  e_x = save_x; e_y = save_y;
  return ret;
}

static void widget_timeout(void* data) {
  ((Widget*)data)->handle(TIMEOUT);
}

/*! Call handle(TIMEOUT) at the given time in the future. This will
  happen exactly once. To make it happen repeatedly, call repeat_timeout()
  from inside handle(TIMEOUT).
*/
void Widget::add_timeout(float time) {
  fltk::add_timeout(time, widget_timeout, this);
}
/*! Call handle(TIMEOUT) at the given time interval since the last timeout.
  This will produce much more accurate time intervals than add_timeout().
*/
void Widget::repeat_timeout(float time) {
  fltk::repeat_timeout(time, widget_timeout, this);
}
/*! Cancel any and all pending handle(TIMEOUT) callbacks. */
void Widget::remove_timeout() {
  fltk::remove_timeout(widget_timeout, this);
}

////////////////////////////////////////////////////////////////

/*! \fn bool Widget::active() const
  Returns true if deactivate() has not been called, or activate()
  has been called since then.

  Parents may also be deactivated, in which case this widget will
  not get events even if this is true. You can test for this with
  !active_r().
*/

/*! Returns true if active() is true for this and all parent widgets.
  This is actually the INACTIVE_R bit in flags(), fltk keeps this
  up to date as widgets are deactivated and/or added to inactive
  parents.
*/
bool Widget::active_r() const {
#if 1
  return !any_of(INACTIVE|INACTIVE_R);
#else
  // this version will work if INACTIVE_R is not kept up to date
  for (const Widget* o = this; o; o = o->parent())
    if (!o->active()) return false;
  return true;
#endif
}

/*! If active() is false, this turns it on. If active_r() is now true
  send() an fltk::ACTIVATE event. */
void Widget::activate() {
  if (!active()) {
    clear_flag(INACTIVE);
    if (!parent() || parent()->active_r()) {
      clear_flag(INACTIVE_R);
      handle(ACTIVATE);
      if (inside(focus())) focus()->take_focus();
      redraw_label(); redraw();
    }
  }
}

/*! If active() is true, this turns it off. If active_r() was true
  send() an fltk::DEACTIVATE event. */
void Widget::deactivate() {
  if (active_r()) {
    set_flag(INACTIVE|INACTIVE_R);
    throw_focus();
    redraw_label(); redraw();
    handle(DEACTIVATE);
  } else {
    set_flag(INACTIVE|INACTIVE_R);
  }
}

/*! \fn bool Widget::output() const
  This flag is similar to !active() except it does not change how the
  widget is drawn. The widget will not recieve any events. This is
  useful for making scrollbars or buttons that work as displays rather
  than input devices.

  Set or clear this flag with set_output() and clear_output().
*/

/*! bool Widget::visible() const
  Returns true if the widget is visible (flag(INVISIBLE) is false)
*/

/*! Returns true if the widget and all of its parents are visible. Only
  if this is true can the user see the widget. */
bool Widget::visible_r() const {
  for (const Widget* o = this; o; o = o->parent())
    if (!o->visible()) return false;
  return true;
}

/*! If visible() is false, turn it on. If visible_r() is then true, send()
  a fltk::SHOW event. */
void Widget::show() {
  if (!visible()) {
    clear_flag(INVISIBLE);
    if (visible_r()) {
      redraw_label(); redraw();
      relayout();
      handle(SHOW);
    }
  }
}

/*! If visible() is true, turn it off. If visible_r() was true then
  send() a fltk::HIDE event, and redraw() the parent if necessary. */
void Widget::hide() {
  if (visible_r()) {
    set_flag(INVISIBLE);
    throw_focus();
    relayout();
    // we must redraw the enclosing group that has an opaque box:
    for (Widget *p = parent(); p; p = p->parent()) {
      if (p->box() != NO_BOX || !p->parent()) {p->redraw(); break;}
    }
    handle(HIDE);
  } else {
    set_flag(INVISIBLE);
  }
}

/*! bool Widget::takesevents() const
  This is the same as (active() && visible() && !output()) but faster.
  send() uses this to decide whether or not to call handle() for most
  events.
*/

/*! bool Widget::click_to_focus() const
  If true then clicking on this widget will give it the focus (the
  handle() method must also return non-zero for fltk::PUSH and for
  fltk::FOCUS events).

  By default fltk only turns this on on certain widgets such as
  fltk::Input. Turning this on on all widgets will make the user
  interface match Windows more closely.
*/

/*! bool Widget::tab_to_focus() const
  If true then this widget can be given focus by keyboard navigation.
  (the handle() method must also return non-zero for fltk::FOCUS events).

  Turning this off with clear_tab_to_focus() will also turn off
  the click_to_focus() flag. This is for compatability in case we
  change the default to a more Windows-like style where all widgets
  get the focus on clicks.

  For historical reasons this flag is true on many more widgets than
  it should be, and FLTK relies on handle() returing 0 for
  FOCUS. This may change in the future so that more widgets have
  this flag off.
*/

/*! bool Widget::changed() const

  The default callback() turns this flag on. This can be used to find
  what widgets have had their value changed by the user, for instance
  in response to an "OK" button.

  Most widgets turn this flag off when they do the callback, and when
  the program sets the stored value.
*/

/*! bool Widget::value() const
  A true/false flag used by fltk::Button to indicate the current state
  and by "parent" items in a hierarchial fltk::Browser to indicate if
  they are open. Many widgets will draw pushed-in or otherwise
  indicate that this flag is on.
*/

/*! bool Widget::selected() const
  A true/false flag used to mark widgets currently selected in
  fltk::Menu and fltk::Browser widgets. Some widgets will draw with
  much different colors if this is on.  */

/*!  Returns true if this is equal to fltk::pushed(), meaning it has
  responded to an fltk::PUSH event and the mouse is still held
  down. Using this function avoids the need to include the <fltk/Fl.h>
  header file. */
bool Widget::pushed() const {return this == fltk::pushed();}

/*!  Returns true if this is equal to fltk::focus(), meaning it has
  the keyboard focus and fltk::KEY events will be sent to this
  widget. Using this function avoids the need to include the
  <fltk/Fl.h> header file. */
bool Widget::focused() const {return this == fltk::focus();}

/*!  Returns true if this is equal to fltk::belowmouse(), meaning it
  has the keyboard focus and fltk::MOVE or fltk::PUSH events will be
  sent to this widget. Using this function avoids the need to include
  the <fltk/Fl.h> header file. */
bool Widget::belowmouse() const {return this == fltk::belowmouse();}

////////////////////////////////////////////////////////////////

/*! \fn bool Widget::state() const
  Widgets have space in them to store a single true/false value
  (put into the STATE bit of flags()). This is used by buttons and
  checkmarks and radio menu items.
*/

/*! \fn bool Widget::set()
  Same as state(true). If you know the widget will already be
  redrawn, or it is not displayed, it is faster to call the
  inline set_flag(STATE) function.
*/

/*! \fn bool Widget::clear()
  Same as state(false). If you know the widget will already be
  redrawn, or it is not displayed, it is faster to call the
  inline clear_flag(STATE) function.
*/

/*!
  Change the state(). Also calls clear_changed(). If the state is
  different, redraw(DAMAGE_VALUE) is called and true is returned. If
  the state is the same then false is returned and the widget is not
  redrawn.
*/
bool Widget::state(bool v) {
  clear_changed();
  unsigned f = v ? (flags_|STATE) : (flags_&~STATE);
  if (f != flags_) {
    flags_ = f;
    redraw(DAMAGE_VALUE);
    return true;
  }
  return false;
}

/*!
  Calls set() on this widget and calls clear() on all adjacent widgets in
  the same parent Group that have the type() set to RADIO.
*/
void Widget::setonly() {
  set();
  Group* g = parent();
  int my_index = g->find( this );
  int i;
  for(i = my_index-1; i >= 0; --i ) {
    Widget* c = g->child(i);
    if( RADIO != c->type() ) break;
    c->clear();
  }
  for(i = my_index+1; i < g->children(); ++i ) {
    Widget* c = g->child(i);
    if( RADIO != c->type() ) break;
    c->clear();
  }
}

//
// End of "$Id: Widget.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
