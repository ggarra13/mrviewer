// "$Id: Tooltip.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

// Try to keep this code from bloating up, as it is always linked in.

/** \class fltk::Tooltip

  This is the class of window that pops up to display a tooltip
  (with the yellow color and 1-pixel border).

  In addition Toolip provides number of static functions for
  managing a single private instance, and making this pop up
  when the mouse is pointing at various rectangles in the GUI.
  FLTK calls these as the mouse moves between widgets, and a
  widget's handle() method can call these to present tooltips
  for various parts of itself.

  You can change the appearance of the tooltips by modifying the
  contents of Tooltip::default_style.
*/

#include <config.h>
#include <fltk/Tooltip.h>
#include <fltk/MenuWindow.h>
#include <fltk/Monitor.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/draw.h>
#include <fltk/Box.h>
using namespace fltk;

float		Tooltip::delay_ = 1.0f;
bool		Tooltip::enabled_ = true;

static const int MAX_WIDTH = 400;
static bool recent_tooltip;

Widget*			Tooltip::current_widget_;
Tooltip::Generator	Tooltip::current_generator_;
void*			Tooltip::current_data_;
Rectangle		Tooltip::current_rectangle_;
Tooltip*		Tooltip::instance_;

Tooltip::Tooltip() : MenuWindow(0, 0) {
  style(Tooltip::default_style);
  flags(ALIGN_LEFT|ALIGN_WRAP|ALIGN_INSIDE);
}

/**
  Measures the label() and resizes itself to fit, and positions itself to
  line up with the rectangle passed to the last enter().
*/
void Tooltip::layout() {
  setfont(textfont(), textsize());
  int ww=MAX_WIDTH, hh=0;
  measure(label(), ww, hh, flags());
  ww += 7; hh += 6;

  // We may want to skip some of this is this!=instance_

  // find position on the screen of the rectangle that popped up the tooltip:
  Rectangle r(current_rectangle_);
  for (Widget* p = current_widget_; p; p = p->parent()) r.move(p->x(), p->y());

  const Monitor& monitor = Monitor::find(event_x_root(),event_y_root());

  // figure out the corner of the tooltip:
  int ox = event_x_root();
  if (ox+ww > monitor.r()) ox = monitor.r()-ww;
  if (ox < monitor.x()) ox = monitor.x();

  int oy = event_y_root()+16;
  if (r.h() < 30) {
    if (oy < r.b()) oy = r.b();
    if (oy+hh > monitor.b()) oy = r.y()-hh;
  } else {
    if (oy+hh > monitor.b()) oy = event_y_root()-hh;
  }
  if (oy < monitor.y()) oy = monitor.y();

  resize(ox, oy, ww, hh);
  MenuWindow::layout();
}

void Tooltip::draw() {
  drawstyle(style(),flags());
  draw_box();
  Rectangle r(w(),h()); //box()->inset(r);
  draw_label(r, flags());
}

static void recent_timeout(void*) {
  recent_tooltip = false;
}

static bool recursion;

// Both Win32 and OSX don't like trying to reuse the window for the next
// tooltip. It's possible this is the right thing to do on X11 as well,
// but this is how we have always been doing it:
static inline void hide_tooltip() {
  if (Tooltip::instance())
#if USE_X11
    Tooltip::instance()->hide();
#else
    Tooltip::instance()->destroy();
#endif
}

void Tooltip::tooltip_timeout(void*) {
  if (recursion) return;
  recursion = true;
  const char* tip = current_generator_(current_widget_, current_data_);
  if (!tip || !*tip) {
    hide_tooltip();
  } else {
    //if (grab()) return;
    if (!instance_) instance_ = new Tooltip;
    // this cast bypasses the normal Window label() code:
    instance_->Widget::label(tip);
    instance_->layout();
    instance_->redraw();
    instance_->show();
  }
  fltk::remove_timeout(recent_timeout);
  recent_tooltip = true;
  recursion = false;
}

/**
  Get ready to display a tooltip. The \a widget and the \a rectangle inside
  it define an area the tooltip is for. The mouse is assummed to currently
  be inside this rectangle.

  After a timeout period a tooltip is created and it will appear at
  the edge of the rectangle.  If enter() with a different \a widget,
  \a generator, or \a data is called, or exit() is called, the
  tooltip will not appear.

  The \a generator callback is for deferring the calculation of the
  text until it is actually needed.  Just before popping up the
  tooltip \a generator is called with the widget and \a data as
  arguments, and it should return a pointer to a static buffer
  containing the tooltip text. You can use '@' commands to insert
  arbitrary fltk::Symbol graphics into it. If \a generator returns
  NULL no tooltip will be popped up.
*/
void Tooltip::enter(Widget* widget, const Rectangle& rectangle,
		    Tooltip::Generator generator, void* data)
{
  if (recursion) return;
  // act like exit() if nothing:
  if (!enabled() || !widget) { exit(); return; }
  // do nothing if it is the same:
  if (widget == current_widget_ &&
      generator == current_generator_ &&
      data == current_data_) return;
  fltk::remove_timeout(tooltip_timeout);
  fltk::remove_timeout(recent_timeout);
  // remember it:
  current_widget_ = widget;
  current_rectangle_ = rectangle;
  current_generator_ = generator;
  current_data_ = data;
  // popup the tooltip immediately if it was recently up:
  if (recent_tooltip || Tooltip::delay() < .1) {
#if !USE_X11
    // On Win32 and OSX trying to resize the existing tooltip does not work
    hide_tooltip();
#endif
    tooltip_timeout(0);
  } else {
    hide_tooltip();
    fltk::add_timeout(delay(), tooltip_timeout);
  }
}

static const char* text_generator(Widget*, void* data) {return (const char*)data;}

/**
  Same as the previous enter() except the tooltip is defined by the
  string constant passed as \a text. The \a text must be in a static
  location that will still exist when the tooltip pops up!

  If \a widget or \a text is NULL then exit() is done instead and no tooltip
  appears.
*/
void Tooltip::enter(Widget* widget, const Rectangle& rectangle, const char* text) {
  if (!widget || !text) exit();
  else enter(widget, rectangle, text_generator, (void*)text);
}

/**
  Locates either \a widget or the closest parent with an non-null
  Widget::tooltip() value and then does an enter() using this widget's
  area and that widget's tooltip(). If \a widget is NULL or there is
  no tooltip found then exit() is done instead.

  This is automatically called by FLTK when Widget::below_mouse() is
  changed.
*/
void Tooltip::enter(Widget* w) {
  //if (w == instance) return;
  // find the enclosing group with a tooltip:
  Widget* tw = w;
  for (;;) {
    if (!tw) {exit(); return;}
    if (tw == current_widget_) return;
    if (tw->tooltip()) break;
    tw = tw->parent();
  }
  enter(w, Rectangle(w->w(), w->h()), tw->tooltip());
}

/**
  Acts as though enter(widget) was done but does \e not pop up a
  tooltip. However, this makes it ignore a subsequent call to enter()
  for this same widget. This is useful to prevent a tooltip from
  reappearing when a modal overlapping window is deleted. FLTK does
  this automatically when a popup window disappears.
*/
void Tooltip::current(Widget* w) {
  exit();
  // find the enclosing group with a tooltip:
  Widget* tw = w;
  for (;;) {
    if (!tw) return;
    if (tw->tooltip()) break;
    tw = tw->parent();
  }
  // act just like enter() except we can remember a zero:
  current_widget_ = w;
  current_generator_ = text_generator;
  current_data_ = (void*)tw->tooltip();
}

/**
  Indicate that the mouse is no longer pointing at a tooltip area.
  Makes any visible tooltip disappear and cancel any pending one. No
  tooltip will appear until after enter() is called again.
*/
void Tooltip::exit() {
  if (!current_widget_) return;
  current_widget_ = 0;
  fltk::remove_timeout(tooltip_timeout);
  fltk::remove_timeout(recent_timeout);
  hide_tooltip();
  if (recent_tooltip) {
    if (event_state() & ANY_BUTTON)
      recent_tooltip = 0;
    else
      fltk::add_timeout(.2f, recent_timeout);
  }
}

/** \fn void Tooltip::delay(float);
  Set the time (in seconds) before tooltips pop up after enter() is
  called. Any time less than .1 causes the tooltip to pop up instantly.
*/

/** \fn void Tooltip::enable(bool b)
  If set to false tooltips never appear. Setting delay() really large will
  do the same thing, but this function is provided as most people find it
  more clear.
*/

/** \fn void Tooltip::disable();
  Same as enable(false);
*/

/** \fn Widget* Tooltip::current_widget();
  Last widget passed to enter(), or NULL if exit() was called.
*/

/** \fn const Rectangle& Tooltip::current_rectangle();
  Last rectangle passed to enter()
*/

/** \fn Tooltip::Generator Tooltip::current_generator();
  Last generator function passed to enter();
*/

/** \fn void* Tooltip::current_data();
  Last data for the generator function passed to enter().
*/

/** \fn Tooltip* Tooltip::instance();
  The Tooltip window that is used to display tooltips in response to enter().
  This is NULL before the first tooltip is shown to the user.
*/

static void revert(Style* s) {
  s->box_ = BORDER_BOX;
  s->color_ = (Color)215;
  //s->textcolor_ = GRAY33;
  //s->labelcolor_ = BLACK;
}
static NamedStyle style("Tooltip", revert, &Tooltip::default_style);
/**
  The Tooltip::default_style controls how the automatic tooltip appears.
  It defaults to the yellow color() and a box() of BORDER_BOX. You may also
  want to set the textfont(), textsize(), textcolor().
*/
NamedStyle* Tooltip::default_style = &::style;

//
// End of "$Id: Tooltip.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
