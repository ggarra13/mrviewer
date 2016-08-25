//
// "$Id: Window.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Window widget class for the Fast Light Tool Kit (FLTK).
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

/*! \class fltk::Window

This widget produces an actual window. This can either be a main
window, with a border and title and all the window management
controls, or a "subwindow" inside a window. This is controlled by
whether or not the window has a parent(). Internally there are now
significant differences between "main" windows and "subwindows" and
these really should be different classes, they are the same for
historic fltk reasons.

Once you create a window, you usually add children Widgets to it by
using add(child) or by using begin() and then constructing the
children. See fltk::Group for more information on how to add and
remove children.

There are several subclasses of fltk::Window that provide
double-buffering, overlay, menu, and OpenGL support.

The window's callback is done if the user tries to close a window
using the window manager and fltk::modal() is zero or equal to the
window. Window has a default callback that calls Window::hide() and
calls exit(0) if this is the last top-level window.

You can set the shortcut() and then that key will call the callback.
If you don't change it then that key will close the window.

*/

// This is the system-independent portions.  The huge amount of
// crap you need to do to communicate with X is in x.C, the
// equivalent (but totally different) crap for MSWindows is in win32.C

#include <config.h>
#include <fltk/Window.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/layout.h>
#include <fltk/run.h>
#include <fltk/x.h>
using namespace fltk;

/*! Return a pointer to the fltk::Window this widget is in.
  (it will skip any and all parent widgets between this and
  the window).  Returns NULL if none.  Note: for an
  fltk::Window, this returns the \e parent window (if any),
  not \e this window.
*/
Window *Widget::window() const {
  for (Widget *o = parent(); o; o = o->parent())
    if (o->is_window()) return (Window*)o;
  return 0;
}

// WAS: anything with cairo contexts should be put into make_current()
void Window::draw() {Group::draw();}

/*! Sets the window title, which is drawn in the titlebar by the system. */
void Window::label(const char *name) {label(name, iconlabel());}

/*! Sets the text displayed below the icon (or in the taskbar). If
  you don't set this it defaults to the label() but if that appears
  to be a filename, it defaults to the last portion after the last
  '/' character. */
void Window::iconlabel(const char *iname) {label(label(), iname);}

void Window::default_callback(Window* window, void*) {
  window->hide();
  // if there are no visible windows we exit:
  // Not anymore, it has been restored to fltk 1.0 behavior. Instead
  // run() checks after each loop to see if all windows are closed.
  // if (!first()) exit(0);
}

static void revert(fltk::Style* s) {
  s->color_ = GRAY75;
  s->box_ = FLAT_BOX;
}
static NamedStyle style("Window", revert, &Window::default_style);
/*! By default a window has box() set to \c FLAT_BOX, and the color()
  set to \c GRAY75, which is a special color cell that is altered by
  fltk::set_background().

  If you plan to turn the border() off you may want to change
  the box() to \c UP_BOX. You can also produce something that looks
  like an arbitrary shape (though really it is showing the original
  screen contents in the "outside" area, so the window had better
  be temporary and the user cannot move it) by setting the box()
  to \c NO_BOX and making draw() only draw the opaque part.
*/
NamedStyle* Window::default_style = &::style;

void Window::_Window() {
  style(default_style);
  type(WINDOW_TYPE);
  i = 0;
  icon_ = 0;
  iconlabel_ = 0;
  resizable(0);
  minw = minh = maxw = maxh = 0;
  size_range_set = 0;
  child_of_ = 0;
  shortcut(EscapeKey);
  callback((Callback*)default_callback);
  set_flag(DOUBLE);
}

/*! This constructor is for \e child windows. You should use the
  constructor with just W and H for normal parent windows. This
  constructor leaves visible() true, so the child window will appear
  when the parent window has show() called. WARNING: this is misleading
  if this is *not* a child window, call clear_visible() for top-level
  ones.
*/
Window::Window(int X,int Y,int W, int H, const char *l, bool begin)
: Group(X, Y, W, H, l, begin) {
  _Window();
  // if (!parent()) clear_visible(); // this breaks the popup menus
}

/*! This form of the constructor should be used for a "top-level"
  window (that is, one that is not inside another window). It
  correctly sets visible() to false and parent() to NULL. By not
  specifying the position of the window, the window system will pick a
  place to show the window (some older X window managers will allow
  the user to place the window by hand). If you want to force a
  position you should call position(x,y) or hotspot() before calling
  show().
*/
Window::Window(int W, int H, const char *l)
// fix common user error of a missing end() with current(0):
  : Group((Group::current(0),USEDEFAULT), USEDEFAULT, W, H, l) {
  _Window();
  clear_visible();
}

/*! \fn void Window::size_range(int minw, int minh, int maxw, int maxh, int dw, int dh)

  Set the allowable range the user can resize this window to. This
  only works for top-level windows.
  - \a minw and \a minh are the smallest the window can be.
  - \a maxw and \a maxh are the largest the window can be. If either is
  equal to the minimum then you cannot resize in that direction. If
  either is zero then FLTK picks a maximum size in that direction such
  that the window will fill the screen.
  - \a dw and \a dh are size increments. The window will be constrained to
  widths of minw+N*dw, where N is any non-negative integer. If
  these are less or equal to 1 they are ignored. (this is ignored on
  WIN32)

  It is undefined what happens if the current size does not fit in the
  constraints passed to size_range().

  If this function is not called, FLTK tries to figure out the range
  from the setting of resizeable():
  - If resizeable() is NULL (this is the default) then the window
  cannot be resized.
  - If either dimension of resizeable() is less than 100, then that is
  considered the minimum size. Otherwise the resizeable() has a
  minimum size of 100.
  - If either dimension of resizeable() is zero, then that is also the
  maximum size (so the window cannot resize in that direction).

*/

/*! void Window::resize(int,int,int,int)

  Change the size and position of the window. If shown() is true,
  these changes are communicated to the window server (which may
  refuse that size and cause a further resize). If shown() is false,
  the size and position are used when show() is called. See
  fltk::Group for the effect of resizing on the child widgets.

  The special value fltk::USEDEFAULT may be used for x and y indicate
  that the system should choose the window's position. This will only
  work before show() is called.

*/

////////////////////////////////////////////////////////////////
// show() and hide()...
// SHOW events will normally create and map the window, HIDE will
// unmap.  On both X and Win32 creating a window requires a lot of ugly
// cruft, some of it is here and much of it is in the machine-specific
// code like Fl_x.cxx.  There are also static variables (!) used to
// modify how the window is created, such as to create it iconized or
// to create it with a parent.

/*! \fn bool Window::shown() const
  Returns non-zero if show() has been called, but destroy() has not
  been called. Note that this returns true if hide() was called or
  if the user has iconized the window.
*/

// This is set by arg to argv[0], or the user can set it.
// It is used by X to look up stuff in the X resource database:
const char* Window::xclass_ = "fltk";

bool fl_show_iconic; // set by iconize() or by -i arg switch

#if defined(_WIN32)

// Windows will directly call the WndProc inside of system calls that
// manipulate windows. This causes all kinds of bugs because there is
// lots of code that assummes any side effects of draw() or handle()
// will not happen when they destroy or hide the window. To avoid this
// all the calls are delayed by putting this in this queue and are
// executed when fltk calls GetMessage next. This will delay the callback
// to WndProc to the time programs expect it (inside fltk::wait()).

enum DeferredCallType {
  NOTHING, SHOW_WINDOW, KEEP_ACTIVE, OPEN_ICON, RAISE_WINDOW, DESTROY_WINDOW
};
struct DeferredCall {DeferredCallType what; HWND window; int argument;};
static DeferredCall* deferred_queue;
static int deferred_queue_size = 0;
static int deferred_queue_alloc = 0;
static void deferred_call(DeferredCallType what, HWND window, int arg=0) {
  if (deferred_queue_size >= deferred_queue_alloc) {
    deferred_queue_alloc = deferred_queue_alloc ? deferred_queue_alloc*2 : 32;
    DeferredCall* newqueue = new DeferredCall[deferred_queue_alloc];
    memcpy(newqueue, deferred_queue, deferred_queue_size*sizeof(DeferredCall));
    delete[] deferred_queue;
    deferred_queue = newqueue;
  }
  deferred_queue[deferred_queue_size].what = what;
  deferred_queue[deferred_queue_size].window = window;
  deferred_queue[deferred_queue_size].argument = arg;
  ++deferred_queue_size;
}

extern HWND ignore_size_change_window;

// Programs that never call wait() (because they are using another GUI
// toolkit along with fltk) may need to call this occasionally:
void fl_do_deferred_calls() {
  static bool recurse = false;
  if (recurse) return;
  recurse = true;
  // Notice that WndProc may be called and put *more* things on the queue.
  // Be sure to handle this. Yuck.
  HWND keep_active = 0;
  for (int n = 0; n < deferred_queue_size; n++) {
    DeferredCall& c = deferred_queue[n];
    switch (c.what) {
    case SHOW_WINDOW:
      if (c.argument == SW_RESTORE)
        ignore_size_change_window = c.window;
      ShowWindow(c.window, c.argument);
      keep_active = 0;
      break;
    case KEEP_ACTIVE:
      keep_active = c.window;
      break;
    case OPEN_ICON:
      ignore_size_change_window = c.window;
      OpenIcon(c.window);
      keep_active = 0;
      break;
    case RAISE_WINDOW:
      BringWindowToTop(c.window);
      keep_active = 0;
      break;
    case DESTROY_WINDOW:
      DestroyWindow(c.window);
      break;
    case NOTHING: break; /* Make GCC happy; Eliminate warning */
    }
  }
  deferred_queue_size = 0;
  // This attemts to fix a Windoze bug where dismissing the child-of
  // a child-of window deactivates the app. This bug has been in every
  // version of Windows for years and affects IE and other Microsoft
  // programs:
  if (keep_active && !grab()) {
    //BringWindowToTop(keep_active);
    SetActiveWindow(keep_active);
  }
  recurse = false;
}

void fl_prune_deferred_calls(HWND window) {
  for (int n = 0; n < deferred_queue_size; n++)
    if (deferred_queue[n].window == window)
      deferred_queue[n].what = NOTHING;
}
#endif

extern Window* fl_actual_window; // in Fl.cxx

bool Window::get_size_range( int *min_w, int *min_h, int *max_w, int *max_h )
{
  if ( min_w ) { *min_w = minw; }
  if ( min_h ) { *min_h = minh; }
  if ( max_w ) { *max_w = maxw; }
  if ( max_h ) { *max_h = maxh; }
  return size_range_set != 0;
}

//+++ verify port to FLTK2
int Window::handle(int event) {
  switch (event) {
  case SHOW:
    // create the window, which will recursively call this:
    if (!shown()) {show(); return 1;}
    Group::handle(event); // make the child windows map first
#if USE_X11
    XMapWindow(xdisplay, i->xid);
#elif defined(_WIN32)
    deferred_call(SHOW_WINDOW, i->xid, SW_RESTORE);
#elif USE_QUARTZ
    if (parent()) ; // needs to update clip and redraw...
    else ShowWindow(i->xid);
#else
#error
#endif
    return 1;

  case HIDE:
    if (flag(MODAL)) modal(0, false);
#if USE_X11
    if (i) {
      if (!parent()) XWithdrawWindow(xdisplay, i->xid, xscreen);
      XUnmapWindow(xdisplay, i->xid);
    }
#elif defined(_WIN32)
    if (i) {
      deferred_call(SHOW_WINDOW, i->xid, SW_HIDE);
      deferred_call(KEEP_ACTIVE, i->xid);
    }
#elif USE_QUARTZ
    if (i) HideWindow(i->xid);
#else
#error
#endif

    break;

  case PUSH:
    // If we are in a modal state, see if the user is clicking on
    // another window. If so, just raise this (the modal) window.
    if (modal()==this && fl_actual_window != this) {
      show();
      return 0;
    }
    break;
  }

  int ret = Group::handle(event); if (ret) return ret;

  // unused events can close windows or raise them:
  if (!parent()) switch (event) {
  case KEY:
  case SHORTCUT:
    // See if the user hit Escape (or whatever the shortcut is)
    // and close the window:
    if (test_shortcut()) {
      if (!event_key_repeated())// make repeating key not close everything
	do_callback();
      return 1;
    }
    break;
#if USE_X11
  case PUSH:
    // Unused clicks should raise the window. Windows unfortunatly
    // raises the window on *all* clicks so we don't bother except on X.
    if (shown()) XMapRaised(xdisplay, i->xid);
#endif
  }
  return 0;
}

/*! Cause the window to become visible. It is harmless to call this
    multiple times.

    For subwindows (with a parent()) this just causes the window
    to appear. Currently no guarantee about stacking order is made.

    For a outer window (one with no parent()) this causes the window
    to appear on the screen, be de-iconized, and be raised to the top.
    Depending on child_of() settings of this window and of windows
    pointing to it, and on system and window manager settings, this
    may cause other windows to also be deiconized and raised, or if this
    window is a child_of() then this window \e may remain iconized.

    <i>Window::show() is not a virtual override of Widget::show()</i>.
    You can call either one. The \e only difference is that if an outer
    window has had show() called already, Window::show() will raise
    and deiconize it, while Widget::show() will only un-hide() it,
    making it appear in the same stacking order as before but not
    changing the iconization state (on some X window managers it will
    deiconize anyway).

    The first time this is called is when the actual "system" window
    (ie the X window) is created. Before that an fltk window is simply
    an internal data structure and is not visible outside your
    program. To return to the non-system-window state call destroy()
    or ~Window(). hide() will "unmap" the system window.

    The first time show() is called on any window is when fltk will
    call fltk::open_display() and fltk::load_theme(), unless you have
    already called them. This allows these expensive operations to be
    deferred as long as possible, and allows fltk programs to be
    written that will run without an X server as long as they don't
    actually show a window.
*/
//+++ verify port to FLTK2
void Window::show() {
  // get rid of very common user bug: forgot end():
  Group::current(0);
  // Emulate the undocumented back-compatability modal() stuff:
  if (any_of(MODAL|NON_MODAL)) {
    child_of(first()); // this may unmap window if it changes
    if (flag(MODAL)) modal(this, false);
  }

#if USE_QUARTZ
  if (!parent()) {
    // bring the application to the front
    ProcessSerialNumber psn;
    OSErr err = GetCurrentProcess( &psn );
    if ( err==noErr ) SetFrontProcess( &psn );
  }
#endif

  if (!shown()) {

    // open the display:
    load_theme();
    open_display();
    layout();

    // Ignore show() calls on children of un-shown parent windows:
    if (parent()) {
      Window* pw = window();
      if (!pw || !pw->shown()) {Widget::show(); return;}
    }

    // back-compatability automatic size_range() based on resizable():
    if (!parent() && !size_range_set) {
      if (resizable()) {
	// find the innermost nested resizable():
	Widget *o = resizable();
	while (o->is_group()) {
	  Widget* p = ((Group*)o)->resizable();
	  if (!p || p == o) break;
	  o = p;
	}
	int minw = w(); if (o->w() > 72) minw -= (o->w()-72);
	int minh = h(); if (o->h() > 72) minh -= (o->h()-72);
	size_range(minw, minh, 0, 0);
      } else {
	size_range(w(), h(), w(), h());
      }
    }

    // Raise any parent windows, to get around stupid window managers
    // that think "child of" means "next to" rather than "above":
    //if (child_of() && !override()) ((Window*)child_of())->show();

    // create the window:
    create();

    // create & map child windows:
    Group::handle(SHOW);

    // We must turn this flag on first because Windows will call
    // the WinProc directly and that code wants this on:
    set_visible();

    // map the window, making it iconized if fl_show_iconic is on:
#if USE_X11
    // for X, iconic stuff was done by create()
    XMapRaised(xdisplay, i->xid);
#elif defined(_WIN32)
    int showtype;
    if (parent())
      showtype = SW_SHOWNORMAL; //SW_RESTORE;
    else if (!modal() && fl_show_iconic)
      showtype = SW_SHOWMINNOACTIVE,fl_show_iconic = false;
    // If we've captured the mouse, we don't want do activate any
    // other windows from the code, or we lose the capture.
    // Also, we don't want to activate the window for tooltips.
    else if (grab() || override())
      showtype = SW_SHOWNOACTIVATE;
    else
      showtype = SW_SHOWNORMAL;
    deferred_call(SHOW_WINDOW, i->xid, showtype);
#elif USE_QUARTZ
    if (!modal() && fl_show_iconic) {
      fl_show_iconic = 0;
      ::CollapseWindow( i->xid, true ); // \todo Mac ; untested
    } else {
      ::ShowWindow(i->xid);
    }
#else
#error
#endif

  } else {

    // Raise+deiconize an already-made top-level window:
    if (!parent()) {
      // Raise any parent windows, to get around stupid window managers
      // that think "child of" means "next to" rather than "above":
      //if (child_of() && !override()) ((Window*)child_of())->show();
#if USE_X11
      XMapRaised(xdisplay, i->xid);
#elif defined(_WIN32)
      if (IsIconic(i->xid)) deferred_call(OPEN_ICON, i->xid);
      if (!grab() && !override()) deferred_call(RAISE_WINDOW, i->xid);
#elif USE_QUARTZ
      ::ShowWindow(i->xid); // does this de-iconize?
      if (!grab() && !override()) {
	::BringToFront(i->xid);
	::SelectWindow(i->xid);
      }
#else
#error
#endif
    }

    // This will call handle(SHOW) if visible is false, which will
    // cause the window to be mapped. Does nothing if visible is
    // already true:
    Widget::show();
  }
}

/*! Tell the system that this window will not have an icon, it will
  dissappear and reappear when \a parent is iconized or shown, and it
  is forced to always be above \a parent. On X this is called a
  "Transient window", and Windows calls this a "overlapping child". \a
  parent is different than the parent(), which must be zero.

  Changing this value causes destroy() to be called, due to stupid
  limitations in X and Windows.

  Win32 and some X window managers have an annoying bug where calling
  show() on this will also raise the parent window to right below
  this, making many useful user interface designs impossible!

  If you want a dialog that blocks interaction with the other windows
  of your application or with all other applications, you need to look
  at exec() (or possibly fltk::modal()). */
void Window::child_of(const Window* parent) {
  if (contains(parent)) return;
  while (parent && parent->parent()) parent = parent->window();
  if (child_of_ != parent) destroy();
  child_of_ = parent;
}

/*! Same as child_of(parent); show(). */
void Window::show(const Window* parent) {
  child_of(parent);
  show();
}

/*!
  If exec() is currently being called, make it hide this window and
  return \a return_value.

  Does nothing if exec() is not being called.

  Results are undefined if the innermost exec() is being called on a
  window \e other than this one. Current behavior is to cause that
  exec to return false.
*/
void Window::make_exec_return(bool return_value) {
  if (return_value) set_flag(STATE);
  fltk::exit_modal();
}

/*!

  The window is popped up and this function does not return until
  make_exec_return() is called, or the window is destroyed or hide()
  is called, or fltk::exit_modal() is called.  During this time events
  to other windows in this application are either thrown away or
  redirected to this window.

  This does child_of(parent) (using first() if parent is null), so
  this window is a floating panel that is kept above the parent.  It
  then uses fltk::modal(this,grab) to make all events go to this
  window.

  The return value is the argument to make_exec_return(), or false
  if any other method is used to exit the loop.

  If parent is null the window that last received an event is used as
  the parent. This is convenient for popups that appear in response to
  a mouse or key click.

  See fltk::modal() for what grab does. This is useful for popup menus.
*/
bool Window::exec(const Window* parent, bool grab) {
  clear_flag(STATE);
  child_of(parent ? parent : first());
  Widget* saved_modal = fltk::modal(); bool saved_grab = fltk::grab();
  fltk::modal(this, grab);
  show();
  while (modal() && !exit_modal_flag()) wait();
  hide();
  modal(saved_modal, saved_grab);
  return state();
}

/*! Make the window with a normal system border and behavior, but
  place it inside the \a frame as though that was the desktop. This is
  what Windows calls "MDI". Typically the other window (which must
  already be shown) is a child window so that space can remain around
  it for a menu/tool bar.

  Notice that parent() of the window must be zero and it will remain
  zero after this is called. Fltk uses a zero parent to indicate that
  the system is managing the window.

  On systems that don't support nested desktops (i.e. X) this does
  child_of(frame) and show(), which produces an overlapping window
  that will remain above the frame window. (IMHO this is a great
  improvement over MDI!).
*/
#if defined(_WIN32) && !USE_X11
extern const Window* fl_mdi_window;
void Window::show_inside(const Window* frame) {
  fl_mdi_window = frame;
  show();
  fl_mdi_window = 0;
}
#else
void Window::show_inside(const Window* frame) {
  show(frame);
}
#endif

////////////////////////////////////////////////////////////////

/*! Indicates that a rectangular region is damaged. draw() will be
  called later with damage() set to
  fltk::DAMAGE_ALL|fltk::DAMAGE_EXPOSE and with FLTK's clipping set to
  at least the given rectangle. Normally this is called more than once
  and the clip region will be the union of all these calls. In
  addition damage from the operating system (ie from overlapping
  windows) will increase the clipping region.

  This can be used to get speed up and improve complex displays of
  many overlapping and changing objects. Even if you do nothing else
  about it, it is usually faster to do a drawing operation that is
  clipped than one that appears, so display will be faster. You can
  also check to see if anything by testing fltk::not_clipped(x,y,w,h)
  or fltk::clip_box(...) and skipping unnecessary drawing calls
  completely. Also if your normal drawing causes blinking (due to
  overlapping objects) this can make the display look much better by
  limiting the blinking to the small area that is actually changing.  */
void Widget::redraw(const fltk::Rectangle& r1) {
  // go up to the window, clipping to each widget's area, quit if empty:
  Widget* window = this;
  fltk::Rectangle r(r1);
  for (;;) {
    if (r.x() < 0) r.set_x(0);
    if (r.y() < 0) r.set_y(0);
    if (r.r() > window->w()) r.set_r(window->w());
    if (r.b() > window->h()) r.set_b(window->h());
    if (r.empty()) return;
    if (window->is_window()) break;
    r.move(window->x(), window->y());
    window = window->parent();
    if (!window) return;
  }
  CreatedWindow* i = CreatedWindow::find((Window*)window);
  if (!i) return; // window not mapped, so ignore it
  window->damage_ |= DAMAGE_EXPOSE;
  i->expose(r);
}

// Merge a rectangle into a window's expose region. If the entire
// window is damaged we switch to a DAMAGE_ALL mode which will
// avoid drawing it twice:
//+++ verify port to FLTK2
void CreatedWindow::expose(const fltk::Rectangle& r) {
  // Ignore if window already marked as completely damaged:
  if (window->damage() & DAMAGE_ALL) ;
  // Detect expose events that cover the entire window:
  else if (r.x()<=0 && r.y()<=0 && r.r()>=window->w() && r.b()>=window->h()) {
    window->set_damage(DAMAGE_ALL);
  } else if (!region) {
    // create a new region:
#if USE_X11
    region = XRectangleRegion(r.x(), r.y(), r.w(), r.h());
#elif defined(_WIN32)
    region = CreateRectRgn(r.x(), r.y(), r.r(), r.b());
#elif USE_QUARTZ
    region = NewRgn();
    SetRectRgn(region, r.x(), r.y(), r.r(), r.b());
#endif
  } else {
    // merge with the region:
#if USE_X11
    XRectangle R;
    R.x = r.x(); R.y = r.y(); R.width = r.w(); R.height = r.h();
    XUnionRectWithRegion(&R, region, region);
#elif defined(_WIN32)
    HRGN R = CreateRectRgn(r.x(), r.y(), r.r(), r.b());
    CombineRgn(region, region, R, RGN_OR);
    DeleteObject(R);
#elif USE_QUARTZ
    RgnHandle R = NewRgn();
    SetRectRgn(R, r.x(), r.y(), r.r(), r.b());
    UnionRgn(R, region, region);
    DisposeRgn(R);
#else
#error
#endif
  }
  // make flush() search for this window:
  fltk::damage(1); // make flush() do something
}

/*! \fn bool Window::double_buffer() const
  Returns true if set_double_buffer() was called, returns false if
  clear_double_buffer() was called. If neither has been called this
  returns a machine-dependent state (systems where double buffering
  is efficient turn it on by default).
*/

/*! \fn void Window::set_double_buffer();
  If the window is double-buffered, all drawing is done first to some
  offscreen image, and then copied to the screen as a single block.
  This eliminates blinking as the window is updated, and often the
  application looks faster, even if it actually is slower.
*/

/*! \fn void Window::clear_double_buffer();
  Turn off double buffering, so that drawing directly goes to the
  visible image on the screen. Not all systems can do this, they
  will remain double buffered even if this is off.
*/

/** A subclass of Window can define this method to draw an "overlay"
  image that appears atop everything else in the window. This will
  only be called if you call redraw_overlay() on the shown() window,
  and it will stop being called if you call erase_overlay().

  This is designed for a fast-changing graphic such as a selection
  rectangle. The implementation is to put the window into
  double_buffer() mode, and then set things so the on-screen image is
  being drawn before this method is called. The previous overlay image
  was erased before calling this by copying the back buffer to the
  front. Notice that the overlay will blink as you redraw it, you must
  trade off this annoyance against the slower update of the overlay
  if you just draw it normally in the draw() method.

  Some systems don't allow drawing in the front buffer. In this case
  the entire back buffer is redrawn and the overlay drawn atop it.

  The default version draws a red rectangle, so that mistaken calls
  to redraw_overlay() are detectable.
*/
void Window::draw_overlay() {
  setcolor(RED);
  fillrect(0,0,w(),h());
}

/** Indicate that the image made by draw_overlay() has changed and must
  be drawn or redrawn. If the image is blank you should call erase_overlay().

  This does nothing if the window is not shown(), it is assummed that
  overlays are only drawn in response to user input.
*/
void Window::redraw_overlay() {
  if (i) {
    i->overlay = true;
    set_damage(damage()|DAMAGE_OVERLAY);
    fltk::damage(1);
  }
}

/** Indicate that the overlay drawn with draw_overlay() is blank.
  draw_overlay() will not be called until redraw_overlay() is
  called again.
*/
void Window::erase_overlay() {
  if (i && i->overlay) {
    i->overlay = false;
    set_damage(damage()|DAMAGE_OVERLAY);
    fltk::damage(1);
  }
}

////////////////////////////////////////////////////////////////

/**
  Hides the window and also deletes all window system information
  about the window, and thus returns it back to the state it was in
  before the first show(). It is harmless to call this if the window
  is already destroyed.

  Subclasses can override this, if you do this you must also override
  the destructor and make it call destroy().
*/
void Window::destroy() {
  free_backbuffer();
  CreatedWindow* x = i;
  if (!x) return;
  i = 0;

  // remove from the list of windows:
#if USE_QUARTZ
  // remove child/brother pointers as well...
  if (CreatedWindow::first == x) CreatedWindow::first = x->next;
  for (CreatedWindow* w = CreatedWindow::first; w; w = w->next) {
    if (w->children == x) w->children = x->brother;
    if (w->brother == x) w->brother = x->brother;
    if (w->next == x) w->next = x->next;
  }
#else
  CreatedWindow** pp = &CreatedWindow::first;
  for (; *pp != x; pp = &(*pp)->next) if (!*pp) return;
  *pp = x->next;
#endif

  // recursively remove any subwindows:
  for (CreatedWindow *x1 = CreatedWindow::first; x1;) {
    Window* subwindow = x1->window;
    if (subwindow->window() == this || subwindow->child_of() == this) {
      bool v = subwindow->visible();
      subwindow->destroy();
      if (v) subwindow->set_visible();
      x1 = CreatedWindow::first;
    } else x1 = x1->next;
  }

  // Make sure no events are sent to this window:
  if (flag(MODAL)) modal(0, false);
  throw_focus();
  clear_visible();

#if USE_X11
  stop_drawing(x->xid);
  if (x->region) XDestroyRegion(x->region);
  if (x->frontbuffer) XDestroyWindow(xdisplay, x->frontbuffer);
  XDestroyWindow(xdisplay, x->xid);
#elif defined(_WIN32)
  stop_drawing(x->xid);
  deferred_call(DESTROY_WINDOW, x->xid);
  if (child_of() && child_of()->shown()) {
    HWND pwnd = xid(child_of());
    if (pwnd) deferred_call(KEEP_ACTIVE, pwnd);
  }
  if (x->region) DeleteObject(x->region);
#elif USE_QUARTZ
  //+++Q: this should probably call 'stop_drawing' as well...
  fltk::release_quartz_context(x);
  if (x->region) DisposeRgn(x->region);
  DisposeWindow(x->xid);
#endif
  delete x;
}

/*! Calls destroy(). The destructor <i>also deletes all the
  children</i>. This allows a whole tree to be deleted at once, without
  having to keep a pointer to all the children in the user code. A
  kludge has been done so the Window and all of it's children
  can be automatic (local) variables, but you must declare the
  Window first so that it is destroyed last. */
Window::~Window() {
  destroy();
}

/** \fn void Window::borders( Rectangle *r ) const

  Rectangle is set to the the size of the borders that will be added
  around this window. This is done by querying the window system.
  Because it is more convienent for most code the result is actually
  the rectangle that would be around the border if this window was
  zero in size and placed at 0,0. x,y are typically negative and w,h
  are typically positive. To get the actual rectangle around your
  window, add these values to the window's size.
*/
/** \fn void Window::border( bool set)
    if set is true, then a window border will be set,
    otherwise the window will have neither border nor caption.
*/
// Implementation in the system-specific code

//
// End of "$Id: Window.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
