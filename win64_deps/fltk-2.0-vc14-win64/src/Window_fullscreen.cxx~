//
// "$Id: Window_fullscreen.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Fullscreen window support for the Fast Light Tool Kit (FLTK).
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

#include <config.h>
#include <fltk/Window.h>
#include <fltk/Monitor.h>
#include <fltk/events.h>
#include <fltk/x.h>
#include <string.h>

using namespace fltk;

static void innards(Window*, bool fullscreen, int X, int Y, int W, int H);

/*! Make the window completely fill the \a monitor, without any window
  manager border or taskbar or anything else visible. Use
  fullscreen_off() to undo this.

  Known bugs:

  - Older versions of both Linux and Windows will refuse to hide the
  taskbar. Proposed solutions for this tend to have detrimental
  effects, such as making it impossible to switch tasks or to put
  another window atop this one. It appears that newer versions of both
  Linux and Windows work correctly, so we will not fix this.

  - Many older X window managers will refuse to position the window
  correctly and instead place them so the top-left of the border
  in the screen corner. You may be able to fix this by calling
  hide() first, then fullscreen(), then show(). We don't do this
  because it causes newer window systems to blink unnecessarily.

  - Some X window managers will raise the window when you change the size.

  - Calling resize() before calling fullscreen_off() will result in
  unpredictable effects, and should be avoided.

*/
void Window::fullscreen(const Monitor& monitor) {
  set_flag(NOBORDER);
  innards(this, true, monitor.x(), monitor.y(), monitor.w(), monitor.h());
}

/*! Chooses the Monitor that the center of the window is on to be
  the one to resize to. */
void Window::fullscreen() {
  fullscreen(Monitor::find(x()+w()/2, y()+h()/2));
}

/*! Turns off any side effects of fullscreen(), then does resize(x,y,w,h). */
void Window::fullscreen_off(int X,int Y,int W,int H) {
  clear_flag(Window::NOBORDER);
  innards(this, false, X, Y, W, H);
}

// Does a resize(x,y,w,h) but also does other crap necessary to convince
// the system that the window is full-screen:
static void innards(Window* window, bool fullscreen, int X, int Y, int W, int H) {
  if (window->shown()) {

#if USE_X11
    // This will make some window managers obey the border being turned on/off.
    // Most other modern window managers will allow the border to be placed
    // off-screen:
    // Unfortunatly stoopid MetaCity raises the window. Sigh
    CreatedWindow::find(window)->sendxjunk();

# if 1
    // Supposedly this tells the new X window managers to put this atop 
    // the taskbar. My tests have shown absolutly zero effect, so this is
    // either wrong or the window managers are ignoreing it. Newer X
    // window managers seem to work without this, they probably recognize
    // attempts to make the window the size of the screen

    // this method does in fact work, and is used below in my maximize()
    // so it should probably work here as well
    // possible problem is that sometimes one have to process/flush events
    // i.e. by using fltk::wait(1) to have this working
    // Perhaps below code can be done correctly(?) again - look at maximize()
    // (or perhaps Im totally wrong, Im new to Xlib  ;)  --Rafal

    static Atom _NET_WM_STATE;
    static Atom _NET_WM_STATE_REMOVE;
    static Atom _NET_WM_STATE_ADD;
    static Atom _NET_WM_STATE_FULLSCREEN;
    if (!_NET_WM_STATE) {
# define MAX_ATOMS 30
      Atom* atom_ptr[MAX_ATOMS];
      const char* names[MAX_ATOMS];
      int i = 0;
# define atom(a,b) atom_ptr[i] = &a; names[i] = b; i++
      atom(_NET_WM_STATE		, "_NET_WM_STATE");
      atom(_NET_WM_STATE_REMOVE	, "_NET_WM_STATE_REMOVE");
      atom(_NET_WM_STATE_ADD	, "_NET_WM_STATE_ADD");
      atom(_NET_WM_STATE_FULLSCREEN, "_NET_WM_STATE_FULLSCREEN");
# undef atom
      Atom atoms[MAX_ATOMS];
      XInternAtoms(xdisplay, (char**)names, i, 0, atoms);
      for (; i--;) *atom_ptr[i] = atoms[i];
    }
    XEvent e;
    e.xany.type = ClientMessage;
    e.xany.window = xid(window);
    e.xclient.message_type = _NET_WM_STATE;
    e.xclient.format = 32;
    e.xclient.data.l[0] = fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
    e.xclient.data.l[1] = (long)_NET_WM_STATE_FULLSCREEN;
    e.xclient.data.l[2] = (long)0;
    e.xclient.data.l[3] = (long)0;
    e.xclient.data.l[4] = (long)0;
    XSendEvent(xdisplay, RootWindow(xdisplay, xscreen), 0,
	       SubstructureNotifyMask|SubstructureRedirectMask, &e);
# endif

#elif defined(_WIN32)

# if 1
    // Disable caption & borders
    LONG flags = GetWindowLong(xid(window), GWL_STYLE);
    if (fullscreen)
      flags &= ~(WS_BORDER | WS_CAPTION | WS_THICKFRAME);
    else
      flags |= (WS_BORDER | WS_CAPTION | WS_THICKFRAME);
    SetWindowLong(xid(window), GWL_STYLE, flags);
    // This appears to be necessary on WinXP:
    if (fullscreen) SetWindowPos(xid(window), 0, X,Y,W,H, 0);
# endif

# if 0
    // Hide the taskbar window. This really screws things up if your
    // program hangs, and appears unncessary on W2K and XP.
#  ifdef _WIN32_WCE
    // Windows CE taskbar
#   define TASKBAR_CLASS TEXT("HHTaskBar")
#  elif _WIN32
    // Desktop windows taskbar
#   define TASKBAR_CLASS TEXT("Shell_TrayWnd")
#  endif
    // Find taskbar and hide it
    HWND taskbar = FindWindow(TASKBAR_CLASS, NULL);
    if (taskbar) ShowWindow(taskbar, fullscreen ? SW_HIDE : SW_RESTORE);
# endif

# if 0
    // Make window topmost, so it goes top of taskbar
    // This will still keep modal and child_of windows top of this.
    // WAS: not according to my tests. Already-existing child windows
    // do not raise. Also makes it impossible to get out if your program
    // wedges, alt+tab will "switch" but this window remains on top.
    SetWindowPos(xid(window), HWND_TOPMOST, X,Y,W,H,0);
    // Used to do 0,0,0,0,SWP_NOSIZE|SWP_NOMOVE)
# endif

#endif
  }

  window->resize(X, Y, W, H);
}

void Window::maximize() {
#ifdef _WIN32
  // ShowWindow - http://msdn.microsoft.com/en-us/library/ms633548.aspx

  // idea from Edzard Egberts - thanks

  HWND hWnd = fltk::xid(this);
  ShowWindow(hWnd, SW_MAXIMIZE);

#elif USE_X11
  // #elif __linux__
  // by Rafal Maj on idea from Mans Rullgard http://tinyurl.com/68mvk3
  // TODO: make it work on very old/simple WMs (as described in url above)
  // TODO: test it more / cleanup (some Xlib expert, please take a look)

  Display *dpy = xdisplay;

  XEvent xev;
  Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
  Atom maximizeV = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", False);
  Atom maximizeH = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
  // XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False); // that would give true fullscreen

  memset(&xev, 0, sizeof(xev));
  xev.type = ClientMessage;
  xev.xclient.window = fltk::xid(this);
  xev.xclient.message_type = wm_state;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = 1;
  xev.xclient.data.l[1] = maximizeV;
  xev.xclient.data.l[2] = maximizeH;
  xev.xclient.data.l[3] = 0;
  XSendEvent(xdisplay, RootWindow(xdisplay, xscreen), 0,
             SubstructureNotifyMask|SubstructureRedirectMask, &xev);

  // flush it right away? Also seems to works without this as well...
  // XFlush(dpy);

#else
# warning "This method will not work on this system. (you can ignore this warning)"
#endif
}

/*! void Window::clear_border()
  This may only be called on a newly constructed window before the
  first call to show().  There will not be any visible border around
  the window, the drawable area extends right to the edge. On most
  systems this means the user cannot move, iconize, or resize the
  window (unless your program does it).
*/

/*! bool Window::border() const
  Returns false if clear_border() has been called.
*/

/*! bool Window::set_override()

  This may only be called on a newly constructed window before the
  first call to show(). There will be no border, similar to
  clear_border(). In addition, the window will get the exact position
  and size set by the program and will not have an entry in the task
  bar. This is used by the fltk menus and tooltips.

  On X this causes "override redirect". On most X systems this is
  only a good idea for short-lived windows such as menus and tooltips,
  and it forces the window to be atop all other windows. It also turns
  on "save under" on the assumption that the window is short-lived.

*/

/*! bool Window::override() const
  Returns true if set_override() has been called.
*/

//
// End of "$Id: Window_fullscreen.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
