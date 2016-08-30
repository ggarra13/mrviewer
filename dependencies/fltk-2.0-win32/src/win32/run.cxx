//
// "$Id: run.cxx 8556 2011-04-02 02:41:00Z bgbnbigben $"
//
// _WIN32-specific code for the Fast Light Tool Kit (FLTK).
// This file is #included by Fl.cxx
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

// This file contains win32-specific code for fltk which is always linked
// in.	Search other files for "_WIN32" or filenames ending in _win32.cxx
// for other system-specific code.

#include <config.h>
#include <fltk/events.h>
#include <fltk/layout.h>
#include <fltk/Window.h>
#include <fltk/Style.h>
#include <fltk/win32.h>
#include <fltk/filename.h>
#include <fltk/utf.h>
#include <fltk/Monitor.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>

#if USE_CAIRO
# include <cairo.h>
# include <cairo-win32.h>
  FL_API cairo_t * fltk::cr=0;
#endif

////////////////////////////////////////////////////////////////
// fabien: added Cairo support for WIN32
#if USE_CAIRO
namespace fltk {
    cairo_surface_t * cairo_create_surface(fltk::Window* w) {
	return cairo_win32_surface_create((HDC)w->backbuffer());
    }
}
#endif

// FIXME - Following block fixes weird problem with compiling FLTK
//   inside Visual C++ (.NET 2003 Enterprise Architect)
#ifdef _MSC_VER
# define IN
# define OUT
#endif

#if 1
# include <winsock.h>
#else
# include <winsock2.h>
#endif

#include <commctrl.h>
#include <ctype.h>
#include <wchar.h>
#include <fltk/math.h>

using namespace fltk;

#define DEBUG_TABLET 0
#if DEBUG_TABLET
#include <stdio.h>
#endif

//
// USE_ASYNC_SELECT - define it non-zero if you have WSAAsyncSelect()...
//
#define USE_ASYNC_SELECT 1

// USE_IMM - define it non-zero if you want Input Method
#define USE_IMM 1

#if USE_IMM
# if defined(__MINGW32__) || defined(__CYGWIN__)
#   include <imm.h>
# endif
#endif

//
// WM_SYNCPAINT is an "undocumented" message, which is finally defined in
// VC++ 6.0.
//
#ifndef WM_SYNCPAINT
#  define WM_SYNCPAINT 0x0088
#endif

#ifndef WM_MOUSELEAVE
#  define WM_MOUSELEAVE 0x02a3
#endif

#ifndef WM_MOUSEWHEEL
#  define WM_MOUSEWHEEL 0x020a
#endif

#ifndef WM_MOUSEHWHEEL
#  define WM_MOUSEHWHEEL 0x020e
#endif

#ifndef WHEEL_DELTA
#  define WHEEL_DELTA 120	// according to MSDN.
#endif

#ifndef ENUM_CURRENT_SETTINGS
#  define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#endif

// In some of the distributions, the gcc header files are missing some stuff:
#ifndef LPMINMAXINFO
# define LPMINMAXINFO MINMAXINFO*
#endif

#ifndef VK_LWIN
# define VK_LWIN 0x5B
# define VK_RWIN 0x5C
# define VK_APPS 0x5D
#endif

// WM_FLSELECT is the user-defined message used for fltk::add_fd()
// to indicate that a socket is ready.
#define WM_FLSELECT	(WM_USER+0x0400)

// WM_MAKEWAITRETURN is the user-defined message that is used to try
// to make wait() return to the main loop so the windows version acts
// like GUI programs on more sensible operating systems
// Note same symbol is in lock.cxx and must match!
#define WM_MAKEWAITRETURN (WM_USER+0x401)

#if USE_IMM
#define IMM_DYNAMIC_LOADING 1

#ifdef IMM_DYNAMIC_LOADING
# ifdef NOIME
typedef struct tagCOMPOSITIONFORM {
  DWORD dwStyle;
  POINT ptCurrentPos;
  RECT  rcArea;
} COMPOSITIONFORM, *PCOMPOSITIONFORM, NEAR *NPCOMPOSITIONFORM, FAR *LPCOMPOSITIONFORM;
typedef HANDLE HIMC;
#endif
HINSTANCE hLibImm = NULL;
HIMC (WINAPI *pfnImmGetContext)(HWND);
BOOL (WINAPI *pfnImmSetCompositionWindow)(HIMC, LPCOMPOSITIONFORM);
BOOL (WINAPI *pfnImmSetCompositionFontW)(HIMC, LPLOGFONTW);
BOOL (WINAPI *pfnImmReleaseContext)(HWND, HIMC);
HIMC (WINAPI *pfnImmAssociateContext)(HWND, HIMC);
#else
# pragma comment(lib, "imm32.lib")
# define pfnImmGetContext ImmGetContext
# define pfnImmSetCompositionWindow ImmSetCompositionWindow
# define pfnImmSetCompositionFontW ImmSetCompositionFontW
# define pfnImmReleaseContext ImmReleaseContext
# define pfnImmAssociateContext ImmAssociateContext
#endif

static bool fl_use_imm32 = false;

static bool fl_load_imm32() {
#ifdef IMM_DYNAMIC_LOADING
  hLibImm = __LoadLibraryW(L"imm32.dll");
  if (hLibImm == NULL)
    return false;

  *(FARPROC*)&pfnImmGetContext
    = GetProcAddress(hLibImm, "ImmGetContext");
  *(FARPROC*)&pfnImmReleaseContext
    = GetProcAddress(hLibImm, "ImmReleaseContext");
  *(FARPROC*)&pfnImmSetCompositionFontW
    = GetProcAddress(hLibImm, "ImmSetCompositionFontW");
  *(FARPROC*)&pfnImmSetCompositionWindow
    = GetProcAddress(hLibImm, "ImmSetCompositionWindow");
  *(FARPROC*)&pfnImmAssociateContext
    = GetProcAddress(hLibImm, "ImmAssociateContext");

  if (!pfnImmGetContext ||
      !pfnImmReleaseContext ||
      !pfnImmSetCompositionFontW ||
      !pfnImmSetCompositionWindow ||
      !pfnImmAssociateContext) {
    FreeLibrary(hLibImm);
    return false;
  }
  #endif // IMM_DYNAMIC_LOADING
  return true;
  } // fl_load_imm32() function
#endif // USE_IMM

void fl_set_spot(fltk::Font *f, Widget *w, int x, int y) {
#if USE_IMM
  if (!fl_use_imm32) return;

  HWND hwnd;
  {Window* flwindow = w->window();
  if (!flwindow || !flwindow->shown()) return;
  hwnd = xid(flwindow);}
  int change = 0;
  //const char *fnt = NULL;
  static fltk::Font *spotf = NULL;
  static Widget *spotw = NULL;
  //static RECT spot_set;
  static RECT spot;

  if (w != spotw) {
    spotw = w;
    change = 1;
  }
  if (x != spot.left || y != spot.top) {
    spot.left = x;
    spot.top = y;
    change = 1;
  }
  if (f != spotf) {
    spotf = f;
    change = 1;
  }

  if (!change) return;

  static HIMC himcold = 0;
  if (f != NULL) {
    HIMC himc = pfnImmGetContext(hwnd);
    if (himc == 0) {
      himc = himcold;
      pfnImmAssociateContext(hwnd, himc);
    }
    if (himc) {
      COMPOSITIONFORM	cfs;
      LOGFONTW lf;
      cfs.dwStyle = CFS_POINT;
      cfs.ptCurrentPos.x = spot.left;
      cfs.ptCurrentPos.y = spot.top;
      pfnImmSetCompositionWindow(himc, &cfs);
      GetObject(fltk::xfont(), sizeof(LOGFONTW), &lf);
      pfnImmSetCompositionFontW(himc, &lf);
      pfnImmReleaseContext(hwnd, himc);
      himcold = 0;
    }
  } else {
    if (!himcold) {
      HIMC himc = pfnImmGetContext(hwnd);
      pfnImmAssociateContext(hwnd, 0);//NULL); MinGW has problems with NULL
      himcold = himc;
      pfnImmReleaseContext(hwnd, himc);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////
// interface to poll/select call:

// fd's are only implemented for sockets.  Microsoft Windows does not
// have a unified IO system, so it doesn't support select() on files,
// devices, or pipes...
//
// Microsoft provides the Berkeley select() call and an asynchronous
// select function that sends a _WIN32 message when the select condition
// exists...

#ifndef USE_ASYNC_SELECT
static fd_set fdsets[3];
#endif // !USE_ASYNC_SELECT

#define POLLIN 1
#define POLLOUT 4
#define POLLERR 8

static int nfds = 0;
static int fd_array_size = 0;
static struct FD {
  int fd;
  short events;
  void (*cb)(int, void*);
  void* arg;
} *fd = 0;


void fltk::add_fd(int n, int events, FileHandler cb, void *v) {
  remove_fd(n,events);
  int i = nfds++;
  if (i >= fd_array_size) {
    fd_array_size = 2*fd_array_size+1;
    fd = (FD*)realloc(fd, fd_array_size*sizeof(FD));
  }
  fd[i].fd = n;
  fd[i].events = events;
  fd[i].cb = cb;
  fd[i].arg = v;

#ifdef USE_ASYNC_SELECT
  int mask = 0;
  if (events & POLLIN) mask |= FD_READ;
  if (events & POLLOUT) mask |= FD_WRITE;
  if (events & POLLERR) mask |= FD_CLOSE;
  WSAAsyncSelect(n, 0/*window*/, WM_FLSELECT, mask);
#else
  if (events & POLLIN) FD_SET(n, &fdsets[0]);
  if (events & POLLOUT) FD_SET(n, &fdsets[1]);
  if (events & POLLERR) FD_SET(n, &fdsets[2]);
#endif // USE_ASYNC_SELECT
}

void fltk::add_fd(int fd, FileHandler cb, void* v) {
  add_fd(fd, POLLIN, cb, v);
}

void fltk::remove_fd(int n, int events) {
  int i,j;
  for (i=j=0; i<nfds; i++) {
    if (fd[i].fd == n) {
      int e = fd[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      fd[i].events = e;
    }
    // move it down in the array if necessary:
    if (j<i) {
      fd[j]=fd[i];
    }
    j++;
  }
  nfds = j;

#ifdef USE_ASYNC_SELECT
  WSAAsyncSelect(n, 0, 0, 0);
#else
  if (events & POLLIN) FD_CLR(unsigned(n), &fdsets[0]);
  if (events & POLLOUT) FD_CLR(unsigned(n), &fdsets[1]);
  if (events & POLLERR) FD_CLR(unsigned(n), &fdsets[2]);
#endif // USE_ASYNC_SELECT
}

// these pointers are set by the lock() function:
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

static void* thread_message_;
void* fltk::thread_message() {
  void* r = thread_message_;
  thread_message_ = 0;
  return r;
}

// ready() is just like wait(0.0) except no callbacks are done:
static inline int fl_ready() {
  if (__PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) return 1;
#ifdef USE_ASYNC_SELECT
  return 0;
#else
  timeval t;
  t.tv_sec = 0;
  t.tv_usec = 0;
  fd_set fdt[3];
  fdt[0] = fdsets[0];
  fdt[1] = fdsets[1];
  fdt[2] = fdsets[2];
  return ::select(0,&fdt[0],&fdt[1],&fdt[2],&t);
#endif // USE_ASYNC_SELECT
}

/**
The most recent message read by GetMessage() (which is called by
fltk::wait().  This may not be the most recent message sent to an FLTK
window (because our fun-loving friends at MicroSoft decided that
calling the handle procedures directly would be a good idea
sometimes...)
*/
MSG fltk::msg;

// Wait up to the given time for any events or sockets to become ready,
// do the callbacks for the events and sockets.
// It *should* return negative on error, 0 if nothing happens before
// timeout, and >0 if any callbacks were done.	This version only
// returns zero if nothing happens during a 0.0 timeout, otherwise
// it returns 1.
static inline int fl_wait(double time_to_wait) {

#ifndef USE_ASYNC_SELECT
  if (nfds) {
    // For _WIN32 we need to poll for socket input FIRST, since
    // the event queue is not something we can select() on...
    timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;

    fd_set fdt[3];
    fdt[0] = fdsets[0];
    fdt[1] = fdsets[1];
    fdt[2] = fdsets[2];

    if (::select(0,&fdt[0],&fdt[1],&fdt[2],&t)) {
      // We got something - do the callback!
      for (int i = 0; i < nfds; i ++) {
	int f = fd[i].fd;
	short revents = 0;
	if (FD_ISSET(f,&fdt[0])) revents |= POLLIN;
	if (FD_ISSET(f,&fdt[1])) revents |= POLLOUT;
	if (FD_ISSET(f,&fdt[2])) revents |= POLLERR;
	if (fd[i].events & revents) fd[i].cb(f, fd[i].arg);
      }
      time_to_wait = 0.0; // just peek for any messages
    } else {
      // we need to check them periodically, so set a short timeout:
      if (time_to_wait > .001) time_to_wait = .001;
    }
  }
#endif // USE_ASYNC_SELECT

  if (!fl_ready()) {
    fl_unlock_function();
    int t_msec =
      time_to_wait < 2147483.647 ? int(time_to_wait*1000+.5) : 0x7fffffff;
    // int ret_val =
      MsgWaitForMultipleObjects(0, NULL, false, t_msec, QS_ALLINPUT);
    fl_lock_function();
  }

  // Execute all pending messages:
  int ret = 0;
  while (__PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
    ret++;
#ifdef USE_ASYNC_SELECT
    if (msg.message == WM_FLSELECT) {
      // Got notification for socket
      for (int i = 0; i < nfds; i ++)
	if (fd[i].fd == (int)msg.wParam) {
	  (fd[i].cb)(fd[i].fd, fd[i].arg);
	  break;
	}
      // looks like it is best to do the dispatch-message anyway:
    } else
#endif
    if (msg.message == WM_MAKEWAITRETURN) {
      // save any data from fltk::awake() call:
      if (msg.wParam) thread_message_ = (void*)msg.wParam;
      // WM_MAKEWAITRETURN is used by WndProc to try to make wait()
      // return so the main loop recovers and can flush the display. We
      // purposely do not dispatch this message, as the desired result
      // has happened: this function will return.  For who knows what
      // reason, sometimes Windoze will call WndProc directly in
      // response to the Post, so it never gets here. This is detected
      // by the WndProc and at that point I give up and call flush().
    } else {
      TranslateMessage(&msg);
      __DispatchMessage(&msg);
    }
  }

  // This should return 0 for timeout, positive for events, and
  // negative for errors.
  // ret_val is probably useful, what is in it?
  return ret;
}

////////////////////////////////////////////////////////////////

#if USE_MULTIMONITOR
static BOOL CALLBACK monitor_union_cb(HMONITOR hMonitor,
				      HDC hdcMonitor,
				      LPRECT lprcMonitor,
				      LPARAM dwData)
{
  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);
  RECT *r = (RECT*)dwData;
  if ( r->left  > mi.rcWork.left )  r->left   = mi.rcWork.left;
  if ( r->top   > mi.rcWork.top  )  r->top    = mi.rcWork.top;
  if ( r->right < mi.rcWork.right ) r->right  = mi.rcWork.right;
  if ( r->bottom< mi.rcWork.bottom) r->bottom = mi.rcWork.bottom;
  return TRUE;
}
#endif

static bool reload_info = true;
static Monitor allMonitors;
/** Return a "monitor" that surrounds all the monitors.
    If you have a single monitor, this returns a monitor structure that
    defines it. If you have multiple monitors this returns a fake monitor
    that surrounds all of them.
*/
const Monitor& Monitor::all() {
  if (reload_info) {
    reload_info = false;

    DEVMODE mode;
    EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &mode);
#if USE_MULTIMONITOR
    allMonitors.set(GetSystemMetrics(SM_XVIRTUALSCREEN),
		GetSystemMetrics(SM_YVIRTUALSCREEN),
		GetSystemMetrics(SM_CXVIRTUALSCREEN),
		GetSystemMetrics(SM_CYVIRTUALSCREEN));
#else
    allMonitors.set(0, 0, mode.dmPelsWidth, mode.dmPelsHeight);
#endif

    allMonitors.depth_ = mode.dmBitsPerPel;
    HDC screen = GetDC(0);
    allMonitors.dpi_x_ = (float)GetDeviceCaps(screen, LOGPIXELSX);
    allMonitors.dpi_y_ = (float)GetDeviceCaps(screen, LOGPIXELSY);

#if USE_MULTIMONITOR
    int monitors = GetSystemMetrics(SM_CMONITORS);
    if (monitors > 0) {
      RECT r;
      r.left = 100000;
      r.top  = 100000;
      r.right = -100000;
      r.bottom = -100000;
      EnumDisplayMonitors(0,0,monitor_union_cb,(LPARAM)&r);
      allMonitors.work.set( r.left, r.top,
			    r.right-r.left, r.bottom-r.top );
    } else {
      allMonitors.work = allMonitors;
    }
#else
    allMonitors.work = allMonitors;
#endif
  }
  return allMonitors;
}

static Monitor* monitors = 0;
static int num_monitors=0;

#if USE_MULTIMONITOR
# ifndef MONITORINFOF_PRIMARY
#  define MONITORINFOF_PRIMARY        0x00000001
# endif

static int monitor_index;
static BOOL CALLBACK monitor_cb(HMONITOR hMonitor,
				HDC hdcMonitor,
				LPRECT lprcMonitor,
				LPARAM dwData)
{
  Monitor& m = monitors[monitor_index];
  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);
  m.set(mi.rcMonitor.left,
	mi.rcMonitor.top,
	mi.rcMonitor.right - mi.rcMonitor.left,
	mi.rcMonitor.bottom - mi.rcMonitor.top);
  m.work.set(mi.rcWork.left,
	     mi.rcWork.top,
	     mi.rcWork.right - mi.rcWork.left,
	     mi.rcWork.bottom - mi.rcWork.top);
  // put the primary monitor first in list:
  if (monitor_index && (mi.dwFlags&MONITORINFOF_PRIMARY)) {
    Monitor t = monitors[0];
    monitors[0] = m;
    m = t;
  }
  monitor_index++;
  return TRUE;
}
#endif

/** Return an array of all Monitors.
    p is set to point to a static array of Monitor structures describing
    all monitors connected to the system. If there is a "primary" monitor,
    it will be first in the list.

    Subsequent calls will usually
    return the same array, but if a signal comes in indicating a change
    it will probably delete the old array and return a new one.
*/
int Monitor::list(const Monitor** p) {
  if (!num_monitors) {
#if USE_MULTIMONITOR
    num_monitors = GetSystemMetrics(SM_CMONITORS);
    if (num_monitors > 1) {
      monitors = new Monitor[num_monitors];
      // put the dpy and other info into them all:
      const Monitor& a = all();
      for (int j = 0; j < num_monitors; j++) monitors[j] = a;
      // now do the callback to get the per-monitor info:
      monitor_index = 0;
      EnumDisplayMonitors(0,0,monitor_cb,0);
      goto DONE;
    }
#endif
    num_monitors = 1; // indicate that Xinerama failed
    monitors = (Monitor*)(&all());
    // Guess for Nvidea which reports only one big monitor:
    // Guess for dual monitors placed next to each other:
    if (monitors->w() > 2*monitors->h()) {
      int w = monitors[0].w()/2;
      num_monitors = 2;
      monitors = new Monitor[2];
      monitors[0] = monitors[1] = all();
      monitors[0].w(w);
      monitors[1].x(w);
      monitors[1].move_r(-w);
      monitors[0].work.w(w-monitors[0].work.x());
      monitors[1].work.x(w);
      monitors[1].work.move_r(-monitors[0].work.w());
    }
#if USE_MULTIMONITOR
  DONE:;
#endif
#if 0
    printf("Got %d monitors:\n", num_monitors);
    for (int i=0; i < num_monitors; i++) {
      const Monitor& m = monitors[i];
      printf(" %d %d %d %d, work %d %d %d %d\n",
	     m.x(), m.y(), m.w(), m.h(),
	     m.work.x(), m.work.y(), m.work.w(), m.work.h());
    }
#endif
  }
  *p = monitors;
  return num_monitors;
}

/** Return a pointer to a Monitor structure describing the monitor
    that contains or is closest to the given x,y, position.
*/
const Monitor& Monitor::find(int x, int y) {
  const Monitor* monitors;
  int count = list(&monitors);
  const Monitor* ret = monitors+0;
  if (count > 1) {
    int r = 0;
    for (int i = 0; i < count; i++) {
      const Monitor& m = monitors[i];
      // find distances to nearest edges:
      int rx;
      if (x <= m.x()) rx = m.x()-x;
      else if (x >= m.r()) rx = x-m.r();
      else rx = 0;
      int ry;
      if (y <= m.y()) ry = m.y()-y;
      else if (y >= m.b()) ry = y-m.b();
      else ry = 0;
      // return this screen if inside:
      if (rx <= 0 && ry <= 0) return m;
      // use larger of horizontal and vertical distances:
      if (rx < ry) rx = ry;
      // remember if this is the closest screen:
      if (!i || rx < r) {
	ret = monitors+i;
	r = rx;
      }
    }
  }
  return *ret;
}

////////////////////////////////////////////////////////////////

void fltk::get_mouse(int &x, int &y) {
  POINT p;
  GetCursorPos(&p);
  x = p.x;
  y = p.y;
}

bool fltk::warp_mouse(int x, int y) {
	return SetCursorPos(x, y) ? true : false;
}

////////////////////////////////////////////////////////////////
// Tablet initialisation and event handling
#include "wintab.h"
#define PACKETDATA (PK_NORMAL_PRESSURE|PK_ORIENTATION|PK_CURSOR)
#define PACKETMODE 0
#include "pktdef.h"

typedef UINT (API *WTINFO)(UINT, UINT, LPVOID);
static WTINFO wtInfo = 0;
typedef HCTX (API *WTOPEN)(HWND, LPLOGCONTEXTA, BOOL);
static WTOPEN wtOpen = 0;
typedef BOOL (API *WTCLOSE)(HCTX);
static WTCLOSE wtClose = 0;
typedef BOOL (API *WTPACKET)(HCTX, UINT, LPVOID);
static WTPACKET wtPacket = 0;
typedef int (API *WTPACKETSGET)(HCTX, int, LPVOID);
static WTPACKETSGET wtPacketsGet = 0;
typedef BOOL (API *WTOVERLAP)(HCTX, BOOL);
static WTOVERLAP wtOverlap = 0;
typedef BOOL (API *WTENABLE)(HCTX, BOOL);
static WTENABLE wtEnable = 0;

static HMODULE wintab_dll = 0;
static HWND wintab_hwnd = 0;
static HCTX wintab_ctx = 0;

static bool stylus_data_valid = false;
static float pressure_add, pressure_mul;

// this function is used to behave gracefully in respect to the WT
// tablet driver by grabbing and releasing tablet devices as needed
void tablet_raise(bool raise) {
  if (!wintab_dll) return;
  if (!wintab_ctx) return;
  wtEnable(wintab_ctx, raise);
  if (raise) wtOverlap(wintab_ctx, TRUE);
}

// if the tablet device driver is not open yet, then create a driver and
// associate it with the first window that is NOT equal to hWnd
void tablet_open(HWND hWnd, HWND avoidWnd) {
  if (!wintab_dll) return; // we don't do any tablet stuff
  if (wintab_ctx) return; // we already have on open tabled driver
  // find a worthy window handle
  if (!hWnd) {
    Window *w = Window::first();
    while (w && xid(w)==hWnd) w = w->next();
    if (!w) return; // no window found. The next window will open a driver automatically.
    // hWnd will now contain a good window handle
    hWnd = xid(w);
  }

  LOGCONTEXT lcMine;           /* The context of the tablet */
  AXIS TabletX, TabletY; /* The maximum tablet size */

  UINT n1, n2;
  wtInfo(WTI_INTERFACE, IFC_NCONTEXTS, &n1);
  wtInfo(WTI_STATUS, STA_CONTEXTS, &n2);
  if (n2>=n1) {
#if DEBUG_TABLET
    printf("'Wintab32.dll' is out of device space (%d/%d). Some application did not clean up.\n", n2, n1);
#endif
    return;
  }
#if DEBUG_TABLET
  printf("%d of %d wintab devices used\n", n2, n1);
#endif

  /* get default region */
  int ret = wtInfo(WTI_DEFCONTEXT, 0, &lcMine);
  if (ret==0) {
#if DEBUG_TABLET
    printf("Error getting table info\n");
#endif
    return;
  }
  /* modify the digitizing region */
  strcpy(lcMine.lcName, "NUKE");
  lcMine.lcOptions |= CXO_MESSAGES|CXO_SYSTEM;
  lcMine.lcPktData =  PACKETDATA;
  lcMine.lcPktMode =  PACKETMODE;
  lcMine.lcMoveMask = PACKETDATA;
  lcMine.lcBtnUpMask = lcMine.lcBtnDnMask;
  /* Set the entire tablet as active */
  wtInfo(WTI_DEVICES,DVC_X,&TabletX);
  wtInfo(WTI_DEVICES,DVC_Y,&TabletY);
  lcMine.lcInOrgX = 0;
  lcMine.lcInOrgY = 0;
  lcMine.lcInExtX = TabletX.axMax;
  lcMine.lcInExtY = TabletY.axMax;

  /* output the data in screen coords */
  lcMine.lcOutOrgX = lcMine.lcOutOrgY = 0;
  lcMine.lcOutExtX = GetSystemMetrics(SM_CXSCREEN);
  /* move origin to upper left */
  lcMine.lcOutExtY = -GetSystemMetrics(SM_CYSCREEN);
  /* open the region */
  wintab_ctx = wtOpen(hWnd, &lcMine, TRUE);
  if (!wintab_ctx) {
#if DEBUG_TABLET
      printf("wtOpen FAILED\n");
#endif
      return;
  }
  if (wintab_ctx) wintab_hwnd = hWnd;
}

// if the window that manages the WT driver is beeing destroyed, we
// must close the driver and reopen it on a differnt window
void tablet_close(HWND hWnd) {
  if (!wintab_dll) return; // we don't do any tablet stuff
  if (!wintab_ctx) return; // there's no tablet device to close
  if (wintab_hwnd==hWnd) { // is this device assigned to this window?
    wtClose(wintab_ctx);   // it is: close the device driver
#if DEBUG_TABLET
      printf("WinTab close\n");
#endif
    wintab_ctx = 0;
    wintab_hwnd = 0;
    tablet_open(0, hWnd);  // try to reopen the driver on another window (but not this one!)
  }
}

/*! Call this to indicate that you are interested in getting more
  information from a pen tablet device. It does some necessary overhead
  to enable tablets on X and Windows. In the future FLTK may do this
  automatically without you calling this.

  If a tablet is detected, fltk::event_pressure()
  will return the pen pressure as a float value between 0 and 1. Some
  tablets also support event_x_tilt() and event_y_tilt().
*/
bool fltk::enable_tablet_events() {
  if (wintab_dll) return true;
  open_display();
  // dynamically load the Windows Tablet driver DLL
  wintab_dll = LoadLibrary("Wintab32.dll");
  if (!wintab_dll) {
#if DEBUG_TABLET
    printf("'Wintab32.dll' not found. There seems to be no tablet installed on this machine\n");
#endif
    return false;
  }
  wtInfo = (WTINFO)GetProcAddress(wintab_dll, "WTInfoA");
  wtOpen = (WTOPEN)GetProcAddress(wintab_dll, "WTOpenA");
  wtClose = (WTCLOSE)GetProcAddress(wintab_dll, "WTClose");
  wtPacket = (WTPACKET)GetProcAddress(wintab_dll, "WTPacket");
  wtEnable = (WTENABLE)GetProcAddress(wintab_dll, "WTEnable");
  wtOverlap = (WTOVERLAP)GetProcAddress(wintab_dll, "WTOverlap");
  wtPacketsGet = (WTPACKETSGET)GetProcAddress(wintab_dll, "WTPacketsGet");
  if (!wtInfo || !wtOpen || !wtClose || !wtPacket || !wtPacketsGet || !wtOverlap) {
#if DEBUG_TABLET
    printf("'Wintab32.dll' is missing some required functions.\n");
#endif
    FreeLibrary(wintab_dll);
    wintab_dll = 0;
    return false;
  }
  AXIS pressure;
  wtInfo(WTI_DEVICES, DVC_NPRESSURE, &pressure);
  pressure_add = 0.0f;
  pressure_mul = 1.0f/pressure.axMax;
  //++ get information about the availability of tilt data
  tablet_open(0, 0);
  return true;
}

////////////////////////////////////////////////////////////////
// Determite if windows has unicode capability

int has_unicode()
{
  static int has_unicode = -1;
  if (has_unicode == -1) {
    OSVERSIONINFOA os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    GetVersionExA(&os);
    has_unicode = (os.dwPlatformId==VER_PLATFORM_WIN32_NT);
  }
  return has_unicode;
}

////////////////////////////////////////////////////////////////
// code used for selections:

static char *selection_buffer[2];
static int selection_length[2];
static int selection_buffer_length[2];
static bool i_own_selection;

// call this when you create a selection:
void fltk::copy(const char *stuff, int len, bool clipboard) {
  if (!stuff || len<0) return;
  if (len >= selection_buffer_length[clipboard]) {
    delete[] selection_buffer[clipboard];
    int n = selection_buffer_length[clipboard]*2;
    if (!n) n = 1024;
    while (len >= n) n *= 2;
    selection_buffer_length[clipboard] = n;
    selection_buffer[clipboard] = new char[n];
  }
  memcpy(selection_buffer[clipboard], stuff, len);
  selection_buffer[clipboard][len] = 0; // needed for direct paste
  selection_length[clipboard] = len;
  if (clipboard) {
    // set up for "delayed rendering":
    if (OpenClipboard(xid(Window::first()))) {
      EmptyClipboard();
      // We might want to use CF_TEXT if there are any illegal UTF-8
      // characters so the bytes are preserved?
      if (has_unicode()) {
        // w2k/xp
        SetClipboardData(CF_UNICODETEXT, NULL);
      } else {
        // w9x
        SetClipboardData(CF_TEXT, NULL);
      }
      CloseClipboard();
      // we should be able to set this true here, but for some reason
      // some changes by other programs do not send the events that
      // makes this turn off:
    }
    i_own_selection = true;
  }
}

static char* previous_buffer = 0;
static int previous_length = 0;

/* Handle CF_TEXT paste */
static void pasteA(Widget& receiver, char* txt) {
  int len = strlen(txt);
  if (len >= previous_length) {
    delete[] previous_buffer;
    previous_length = len+100;
    previous_buffer = new char[previous_length];
  }
  strcpy(previous_buffer, txt);
  e_text = previous_buffer;
  // strip the CRLF pairs: ($%$#@^)
  char* a = previous_buffer;
  char* b = a;
  char* e = a+len;
  while (a<e) {
    if (*a == '\r' && a[1] == '\n') a++;
    else *b++ = *a++;
  }
  *b = 0;
  e_length = b - e_text;
  receiver.handle(PASTE);
}

/* Handle CF_UNICODETEXT paste */
static void pasteW(Widget& receiver, wchar_t* ucs) {
  int ucslen = wcslen(ucs);
  int len = utf8fromwc(previous_buffer, previous_length, ucs, ucslen);
  if (len >= previous_length) {
    delete[] previous_buffer;
    previous_length = len+100;
    previous_buffer = new char[previous_length];
    len = utf8fromwc(previous_buffer, previous_length, ucs, ucslen);
  }
  e_text = previous_buffer;
  // strip the CRLF pairs: ($%$#@^)
  char* a = previous_buffer;
  char* b = a;
  char* e = a+len;
  while (a<e) {
    if (*a == '\r' && a[1] == '\n') a++;
    else *b++ = *a++;
  }
  *b = 0;
  e_length = b - e_text;
  receiver.handle(PASTE);
}

// Call this when a "paste" operation happens:
void fltk::paste(Widget &receiver, bool clipboard) {
  if (!clipboard || i_own_selection) {
    // We already have it, do it quickly without window server.
    // Notice that the text is clobbered if set_selection is
    // called in response to PASTE!
    e_text = selection_buffer[clipboard];
    e_length = selection_length[clipboard];
    receiver.handle(PASTE);
  } else {
    if (!OpenClipboard(NULL)) return;
    HANDLE h;
    if ((h = GetClipboardData(CF_UNICODETEXT))) {
      pasteW(receiver, (wchar_t*)GlobalLock(h));
      GlobalUnlock(h);
    } else if ((h = GetClipboardData(CF_TEXT))) {
      pasteA(receiver, (char*)GlobalLock(h));
      GlobalUnlock(h);
    }
    CloseClipboard();
  }
}

// Communicating data to Win32 requires it to be in "global memory", this
// copies the given selection to such a block and returns it. It appears
// this block is usually handed to Windows and Windows deletes it.
HANDLE fl_global_selection(int clipboard) {
  //printf("fl_global_selection(%d)\n", clipboard);
  int n = utf8towc(selection_buffer[clipboard],
		   selection_length[clipboard],
		   0, 0);
  HANDLE h = GlobalAlloc(GHND, sizeof(wchar_t) * (n+1));
  LPWSTR p = (LPWSTR)GlobalLock(h);
  utf8towc(selection_buffer[clipboard],
	   selection_length[clipboard],
	   p, n+1);
  GlobalUnlock(h);
  return h;
}

// Same as fl_global_selection(), but for ASCII text.
// Clipboard format: CF_TEXT
HANDLE fl_global_selection_ansi(int clipboard) {
  //printf("fl_global_selection_ansi(%d)\n", clipboard);
  int n = utf8tomb(selection_buffer[clipboard], selection_length[clipboard], 0, 0);
  HANDLE h = GlobalAlloc(GHND, n+1);
  LPSTR p = (LPSTR)GlobalLock(h);
  utf8tomb(selection_buffer[clipboard], selection_length[clipboard], p, n+1);
  GlobalUnlock(h);
  return h;
}

static void renderallformats() {
  if (!i_own_selection) return;
  i_own_selection = false;
  OpenClipboard(NULL);
  EmptyClipboard();
  SetClipboardData(CF_TEXT, fl_global_selection_ansi(1));
  SetClipboardData(CF_UNICODETEXT, fl_global_selection(1));
  CloseClipboard();
}

////////////////////////////////////////////////////////////////

// Drag-n-drop requires GCC 3.x or a non-GNU compiler...
#if !defined(__GNUC__) || __GNUC__ >= 3
// Drag-n-drop is totally disabled until Wonko can figure out what is wrong
# define USE_DRAGDROP 1
#else
# define USE_DRAGDROP 0
#endif

#if USE_DRAGDROP
// I believe this was written by Matthias Melcher, correct?

#include <ole2.h>
#include <shellapi.h>

static Window *dnd_target_window = 0;

/*
 * subclass the IDropTarget to receive data from DnD operations
 */
class FlDropTarget : public IDropTarget
{
  DWORD m_cRefCount;
  DWORD lastEffect;
  int px, py;
public:
  FlDropTarget() : m_cRefCount(0) { } // initialize
  HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, LPVOID *ppvObject ) {
    if (IID_IUnknown==riid || IID_IDropTarget==riid)
    {
      *ppvObject=this;
      ((LPUNKNOWN)*ppvObject)->AddRef();
      return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
  }
  ULONG STDMETHODCALLTYPE AddRef() { return ++m_cRefCount; }
  ULONG STDMETHODCALLTYPE Release() {
    long nTemp;
    nTemp = --m_cRefCount;
    //if (nTemp==0) delete this; // we don't really destroy it
    return nTemp;
  }
  HRESULT STDMETHODCALLTYPE DragEnter( IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
    if( !pDataObj ) return E_INVALIDARG;
    // set e_modifiers here from grfKeyState, set e_x and e_root_x
    // check if FLTK handles this drag and return if it can't (i.e. BMP drag without filename)
    POINT ppt;
    e_x_root = ppt.x = pt.x;
    e_y_root = ppt.y = pt.y;
    HWND hWnd = WindowFromPoint( ppt );
    Window *target = find( hWnd );
    if (target) {
      e_x = e_x_root-target->x();
      e_y = e_y_root-target->y();
    }
    dnd_target_window = target;
    px = pt.x; py = pt.y;
      // FLTK has no mechanism yet for the different drop effects, so we allow move and copy
    if ( target && handle( DND_ENTER, target ) )
      *pdwEffect = DROPEFFECT_MOVE|DROPEFFECT_COPY; //|DROPEFFECT_LINK;
    else
      *pdwEffect = DROPEFFECT_NONE;
    lastEffect = *pdwEffect;
    flush(); // get the display to update for local drags
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE DragOver( DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
		if ( px==pt.x && py==pt.y )
    {
      *pdwEffect = lastEffect;
      return S_OK;
    }
    if ( !dnd_target_window )
    {
      *pdwEffect = lastEffect = DROPEFFECT_NONE;
      return S_OK;
    }
    // set e_modifiers here from grfKeyState, set e_x and e_root_x
    e_x_root = pt.x;
    e_y_root = pt.y;
    if (dnd_target_window) {
      e_x = e_x_root-dnd_target_window->x();
      e_y = e_y_root-dnd_target_window->y();
    }
    // Group will change DND_DRAG into DND_ENTER and DND_LEAVE if needed
    if ( handle( DND_DRAG, dnd_target_window ) )
      *pdwEffect = DROPEFFECT_MOVE|DROPEFFECT_COPY; //|DROPEFFECT_LINK;
    else
      *pdwEffect = DROPEFFECT_NONE;
    px = pt.x; py = pt.y;
    lastEffect = *pdwEffect;
    flush(); // get the display to update for local drags
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE DragLeave() {
    if ( dnd_target_window )
    {
      handle( DND_LEAVE, dnd_target_window );
      flush(); // get the display to update for local drags
      dnd_target_window = 0;
    }
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE Drop( IDataObject *data, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
    if ( !dnd_target_window )
      return S_OK;
    Window *target_window = dnd_target_window;
    dnd_target_window = 0;
    e_x_root = pt.x;
    e_y_root = pt.y;
    if (target_window) {
      e_x = e_x_root-target_window->x();
      e_y = e_y_root-target_window->y();
    }
    // tell FLTK that the user released an object on this widget
    if ( !handle( DND_RELEASE, target_window ) )
      return S_OK;

    Widget* target = belowmouse();
    if (!target) return S_OK;
    Widget *w = target;
    while (w->parent()) w = w->window();
    HWND hwnd = xid( (Window*)w );

    FORMATETC fmt = { 0 };
    STGMEDIUM medium = { 0 };

    fmt.tymed = TYMED_HGLOBAL;
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.cfFormat = CF_UNICODETEXT;
    // if it is ASCII text, send an PASTE with that text
    if ( data->GetData( &fmt, &medium )==S_OK ) {
      pasteW(*target, (wchar_t*)GlobalLock( medium.hGlobal ));
      GlobalUnlock( medium.hGlobal );
      ReleaseStgMedium( &medium );
      SetForegroundWindow( hwnd );
      return S_OK;
    }
    fmt.tymed = TYMED_HGLOBAL;
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.cfFormat = CF_TEXT;
    // if it is ASCII text, send an PASTE with that text
    if ( data->GetData( &fmt, &medium )==S_OK )
    {
      void *stuff = GlobalLock( medium.hGlobal );
      e_length = strlen( (char*)stuff ); // min(strlen, len)
      e_text = (char*)stuff;
      target->handle(PASTE); // e_text will be invalid after this call
      GlobalUnlock( medium.hGlobal );
      ReleaseStgMedium( &medium );
      SetForegroundWindow( hwnd );
      return S_OK;
    }

    fmt.tymed = TYMED_HGLOBAL;
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.cfFormat = CF_HDROP;
    // if it is a pathname list, send an PASTE with a \n seperated list of filepaths
    if ( data->GetData( &fmt, &medium )==S_OK )
    {
      HDROP hdrop = (HDROP)medium.hGlobal;
      int i, n, nn = 0, nf = DragQueryFileA( hdrop, (UINT)-1, 0, 0 );
      for ( i=0; i<nf; i++ ) nn += DragQueryFileA( hdrop, i, 0, 0 );
      nn += nf;
      if (has_unicode()) {
	wchar_t *buffer = new wchar_t[nn+1];
	wchar_t *dst = buffer;
	for ( i=0; i<nf; i++ ) {
	  n = DragQueryFileW( hdrop, i, dst, nn );
	  dst += n;
	  if ( i<nf-1 ) *dst++ = '\n';
	}
	*dst = 0;
	pasteW(*target, buffer);
	delete[] buffer;
      } else {
	char* buffer = new char[nn+1];
	e_length = nn;
	char *dst = buffer;
	e_text = buffer;
	for ( i=0; i<nf; i++ ) {
	  n = DragQueryFileA( hdrop, i, dst, nn );
	  dst += n;
	  if ( i<nf-1 ) *dst++ = '\n';
	}
	*dst = 0;
	target->handle(PASTE);
	delete[] buffer;
      }
      e_length = 0;
      ReleaseStgMedium( &medium );
      SetForegroundWindow( hwnd );
      return S_OK;
    }
    return S_OK;
  }
} flDropTarget;

#endif

////////////////////////////////////////////////////////////////

//
// NO_TRACK_MOUSE - define it if you dont have TrackMouseEvent()
//
// Fortunately, most of user have this.
// the TrackMouseEvent() function is included since windows 98 and NT4
// if you dont have it (win95) - this is only needed to support subwindow
// enter/leave notification under Windows.
//

//#define NO_TRACK_MOUSE 1

#ifndef NO_TRACK_MOUSE
extern "C" {
  BOOL WINAPI TrackMouseEvent(LPTRACKMOUSEEVENT lpEventTrack);
};
#endif

// Return the shift flags for event_state():
static unsigned long shiftflags(bool ignorealt=false) {
  unsigned long state = 0;
  // if GetKeyState is expensive we might want to comment some of these out:
  if (GetKeyState(VK_SHIFT)&~1) state |= SHIFT;
  if (GetKeyState(VK_CAPITAL)) state |= CAPSLOCK;
  if (GetKeyState(VK_CONTROL)&~1) state |= CTRL;
  // Alt gets reported for the Alt-GR switch on foreign keyboards.
  // ignorealt is turned on in this case so we don't act like alt is on
  if (!ignorealt && (GetKeyState(VK_MENU)&~1)) {
    // Some weird bugfix by Microsoft in recent versions of Windows makes
    // alt "stay on" until the next keystroke. But calling async seems
    // to fix it.
    if (GetAsyncKeyState(VK_MENU)&~1) state |= ALT;
  }
  if (GetKeyState(VK_NUMLOCK)) state |= NUMLOCK;
  if (GetKeyState(VK_LWIN)&~1 || GetKeyState(VK_RWIN)&~1) {
    // _WIN32 bug?  GetKeyState returns garbage if the user hit the
    // Windows key to pop up start menu. Again calling async seems
    // to fix it:
    if ((GetAsyncKeyState(VK_LWIN)|GetAsyncKeyState(VK_RWIN))&~1)
      state |= META;
  }
  if (GetKeyState(VK_SCROLL)) state |= SCROLLLOCK;
  return state;
}

static unsigned recent_keysym = 0;

static bool mouse_event(Window *window, int what, unsigned button,
			WPARAM wParam, LPARAM lParam)
{
  xmousewin = window;
  if (!window) return false;
#ifndef NO_TRACK_MOUSE
  TRACKMOUSEEVENT tme;
  tme.cbSize    = sizeof(TRACKMOUSEEVENT);
  tme.dwFlags   = TME_LEAVE;
  tme.hwndTrack = xid(window);
  TrackMouseEvent(&tme);
#endif
  static int px, py, pmx, pmy;
  POINT pt;
  e_x = pt.x = (signed short)LOWORD(lParam);
  e_y = pt.y = (signed short)HIWORD(lParam);
  ClientToScreen(xid(window), &pt);
  e_x_root = pt.x;
  e_y_root = pt.y;
  while (window->parent()) {
    e_x += window->x();
    e_y += window->y();
    if(window->window() ) window = window->window();
    else break;
  }

  unsigned long state = shiftflags();
#if 0
  // mouse event reports some shift flags, perhaps save them?
  if (wParam & MK_SHIFT) state |= SHIFT;
  if (wParam & MK_CONTROL) state |= CTRL;
#endif
  if (wParam & MK_LBUTTON) state |= BUTTON1;
  if (wParam & MK_MBUTTON) state |= BUTTON2;
  if (wParam & MK_RBUTTON) state |= BUTTON3;
  e_state = state;

  // make the stylus data produce something useful if there's no pen
  if (!stylus_data_valid) {
    e_pressure = e_state&BUTTON1 ? 1.0f : 0.0f;
    e_x_tilt = e_y_tilt = 0.0f;
    e_device = DEVICE_MOUSE;
  }

  switch (what) {
  case 1: // double-click
    // This is not detecting triple-clicks, does anybody know how to fix?
    if (e_is_click == button) {e_clicks++; goto J1;}
  case 0: // single-click
    e_clicks = 0;
  J1:
    if (!grab_) SetCapture(xid(window));
    e_keysym = e_is_click = button;
    recent_keysym = 0;
    px = pmx = e_x_root; py = pmy = e_y_root;
    if (handle(PUSH, window)) return true;
    // If modal is on and 0 is returned, we should turn off modal and
    // pass the event on to other widgets. The pass-on part is nyi!
    if (grab_) {exit_modal(); /* NYI */ }
    return false;

  case 2: // release:
    // WAS: this should turn off e_is_click if more than .2 second passed
    // since the push event!
    if (!grab_) ReleaseCapture();
    e_keysym = button;
    return handle(RELEASE,window);

  case 3: // move:
  default: // avoid compiler warning
    // MSWindows produces extra events even if mouse does not move, ignore em:
    if (e_x_root == pmx && e_y_root == pmy) return true;
    pmx = e_x_root; pmy = e_y_root;
    if (abs(e_x_root-px)>5 || abs(e_y_root-py)>5) {
      e_is_click = 0;
      recent_keysym = 0;
    }
    return handle(MOVE,window);
  }
}

// convert a MSWindows VK_x to an Fltk (X) Keysym:
// See also the inverse converter in get_key_win32.C
// This table is in numeric order by VK:
static const struct {unsigned short vk, fltk, extended;} vktab[] = {
  {VK_BACK,	BackSpaceKey},
  {VK_TAB,	TabKey},
  {VK_CLEAR,	Keypad5,	ClearKey},
  {VK_RETURN,	ReturnKey,	KeypadEnter},
  {VK_SHIFT,	LeftShiftKey,	RightShiftKey},
  {VK_CONTROL,	LeftCtrlKey,	RightCtrlKey},
  {VK_MENU,	LeftAltKey,	RightAltKey},
  {VK_PAUSE,	PauseKey},
  {VK_CAPITAL,	CapsLockKey},
  {VK_ESCAPE,	EscapeKey},
  {VK_SPACE,	SpaceKey},
#if IGNORE_NUMLOCK
  {VK_PRIOR,	Keypad9,	PageUpKey},
  {VK_NEXT,	Keypad3,	PageDownKey},
  {VK_END,	Keypad1,	EndKey},
  {VK_HOME,	Keypad7,	HomeKey},
  {VK_LEFT,	Keypad4,	LeftKey},
  {VK_UP,	Keypad8,	UpKey},
  {VK_RIGHT,	Keypad6,	RightKey},
  {VK_DOWN,	Keypad2,	DownKey},
  {VK_SNAPSHOT, PrintKey},	// does not work on NT
  {VK_INSERT,	Keypad0,	InsertKey},
  {VK_DELETE,	DecimalKey,	DeleteKey},
#else
  {VK_PRIOR,	PageUpKey},
  {VK_NEXT,	PageDownKey},
  {VK_END,	EndKey},
  {VK_HOME,	HomeKey},
  {VK_LEFT,	LeftKey},
  {VK_UP,	UpKey},
  {VK_RIGHT,	RightKey},
  {VK_DOWN,	DownKey},
  {VK_SNAPSHOT, PrintKey},	// does not work on NT
  {VK_INSERT,	InsertKey},
  {VK_DELETE,	DeleteKey},
#endif
  {VK_LWIN,	LeftMetaKey},
  {VK_RWIN,	RightMetaKey},
  {VK_APPS,	MenuKey},
  {VK_MULTIPLY, MultiplyKey},
  {VK_ADD,	AddKey},
  {VK_SUBTRACT, SubtractKey},
  {VK_DECIMAL,	DecimalKey},
  {VK_DIVIDE,	DivideKey},
  {VK_NUMLOCK,	NumLockKey},
  {VK_SCROLL,	ScrollLockKey},
  {0xba,	';'},
  {0xbb,	'='},
  {0xbc,	','},
  {0xbd,	'-'},
  {0xbe,	'.'},
  {0xbf,	'/'},
  {0xc0,	'`'},
  {0xdb,	'['},
  {0xdc,	'\\'},
  {0xdd,	']'},
  {0xde,	'\''}
};
bool fl_last_was_extended;
static inline int ms2fltk(int vk, LPARAM lParam) {
  static unsigned short vklut[256];
  static unsigned short extendedlut[256];
  if (!vklut[1]) { // init the table
    unsigned int i;
    for (i = 0; i < 256; i++) vklut[i] = tolower(i);
    for (i=VK_F1; i<=VK_F16; i++) vklut[i] = i+(F1Key-VK_F1);
    for (i=VK_NUMPAD0; i<=VK_NUMPAD9; i++) vklut[i] = i+(Keypad0-VK_NUMPAD0);
    for (i = 0; i < sizeof(vktab)/sizeof(*vktab); i++) {
      vklut[vktab[i].vk] = vktab[i].fltk;
      extendedlut[vktab[i].vk] = vktab[i].extended;
    }
    for (i = 0; i < 256; i++) if (!extendedlut[i]) extendedlut[i] = vklut[i];
  }
  static bool rShiftWasOn = false;
  
  if (vk == VK_SHIFT && HIWORD(GetKeyState(VK_RSHIFT)) && !rShiftWasOn) {
    rShiftWasOn = true;
    if (rShiftWasOn) lParam |= 1 << 24;
  } else if (vk == VK_SHIFT && rShiftWasOn && !HIWORD(GetKeyState(VK_RSHIFT))) {
    rShiftWasOn = false;
    lParam |= 1 << 24;
  }
  if (lParam&(1<<24)) {
    if (!(lParam&(1<<31))) fl_last_was_extended = true;
    return extendedlut[vk];
  } else {
    if (!(lParam&(1<<31))) fl_last_was_extended = false;
    return vklut[vk];
  }
}

#if USE_COLORMAP
extern HPALETTE fl_select_palette(HDC dc); // in color_win32.C
#else
static inline bool fl_select_palette(HDC) {return false;}
#endif

extern void fl_prune_deferred_calls(HWND);
HWND ignore_size_change_window;

#define MakeWaitReturn() __PostMessage(hWnd, WM_MAKEWAITRETURN, 0, 0)
static CreatedWindow * sCreatedWindowPending = 0; // this hack keeps tracks of the CreatedWindow instance during its creation before we know its xid ...

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  //printf("Window %x msg %x\n", hWnd, uMsg);
  // Copy the message to msg so add_handler code can see it, it is
  // already there if this is called by DispatchMessage, but not if
  // Windows calls this directly.
  msg.hwnd = hWnd;
  msg.message = uMsg;
  msg.wParam = wParam;
  msg.lParam = lParam;
  //msg.time = ???
  //msg.pt = ???
  //msg.lPrivate = ???

  Window *window = find(hWnd);
  // Don't have any window/xid yet during reentrant window creation ? hack it !
  if (!window && sCreatedWindowPending) {
      window=sCreatedWindowPending->window;
      sCreatedWindowPending->xid = hWnd;
      }
  switch (uMsg) {

  case WM_SYNCPAINT :
  case WM_NCPAINT :
  case WM_ERASEBKGND :
    // Andreas Weitl - WM_SYNCPAINT needs to be passed to DefWindowProc
    // so that Windows can generate the proper paint messages...
    // Similarly, WM_NCPAINT and WM_ERASEBKGND need this, too...
    break;

  case WM_CAPTURECHANGED:
    if (grab_) {
      exit_modal();
      // Make wait() return so that the function that called modal()
      // can continue executing:
      MakeWaitReturn();
    }
    break;

#if 0
  case WM_NCACTIVATE:
    // This prevents the title bar highlighting from turning off while a
    // modal window is running. This is certainly necessary for menus
    // but they do the grab which seems to make this not happen.
    if (modal_) wParam = 1;
    break;
#endif

  case WM_QUIT: // this should not happen?
    fatal("WM_QUIT message");

  case WM_CLOSE: // user clicked close box
    if (!window) break;
    if (!modal_ || window == modal_) window->do_callback();
    return 1;

  case WM_DESTROY:
    // Microsoft, you MORONS! Don't destroy stuff I create!
    // Oh well, try to handle this as gracefully as possible so we
    // don't send any commands to the now non-existent window:
    tablet_close(hWnd);
    if (window) window->destroy();
    fl_prune_deferred_calls(hWnd);
    break;

  case WM_ACTIVATE:
    tablet_raise(LOWORD(wParam)!=WA_INACTIVE);
    break;

  case WM_PAINT: {
    if (!window) break;
    CreatedWindow *i = CreatedWindow::find(window);
    i->wait_for_expose = false;
    // Merge the region into whatever is accumulated by fltk. I do this
    // by invalidating the fltk region and reading the resulting region
    // back:
    if (i->region) InvalidateRgn(hWnd, i->region, FALSE);
    else i->region = CreateRectRgn(0,0,0,0);
    GetUpdateRgn(hWnd, i->region, 0);
    // This convinces MSWindows we have painted whatever they wanted
    // us to paint, and stops it from sending WM_PAINT messages:
    ValidateRgn(hWnd, i->region);
    fltk::damage(1); // make flush() do something
    // Originally it called flush() directly here, but it appears to
    // be better to get wait() to return by posting a message. This
    // merges the damage together and waits for idle. Windows appears to
    // sometimes send WM_PAINT while we are painting the previous contents
    // and this minimizes the chances that will mess it up:
    MakeWaitReturn();
    //flush();
    break;}

  case WM_LBUTTONDOWN:	mouse_event(window, 0, 1, wParam, lParam); return 0;
  case WM_LBUTTONDBLCLK:mouse_event(window, 1, 1, wParam, lParam); return 0;
  case WM_LBUTTONUP:	mouse_event(window, 2, 1, wParam, lParam); return 0;
  case WM_MBUTTONDOWN:	mouse_event(window, 0, 2, wParam, lParam); return 0;
  case WM_MBUTTONDBLCLK:mouse_event(window, 1, 2, wParam, lParam); return 0;
  case WM_MBUTTONUP:	mouse_event(window, 2, 2, wParam, lParam); return 0;
  case WM_RBUTTONDOWN:	mouse_event(window, 0, 3, wParam, lParam); return 0;
  case WM_RBUTTONDBLCLK:mouse_event(window, 1, 3, wParam, lParam); return 0;
  case WM_RBUTTONUP:	mouse_event(window, 2, 3, wParam, lParam); return 0;

  case WM_MOUSEMOVE:    mouse_event(window, 3, 0, wParam, lParam); return 0;

  case WM_MOUSELEAVE:
    // In fltk2 we should only call handle(LEAVE) if the mouse is
    // not pointing at a window belonging to the application. This seems
    // to work, probably because the enter event has already been done
    // and has changed xmousewin to some other window:
    if (window == xmousewin) {xmousewin = 0; handle(LEAVE, window);}
    break;

  case WM_KILLFOCUS:
    window = 0;
  case WM_SETFOCUS:
    if (xfocus != window) {
      xfocus = window;
      fix_focus();
      MakeWaitReturn();
    }
    break;

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
    // save the keysym until we figure out the characters:
    e_keysym = ms2fltk(wParam,lParam);
    // See if TranslateMessage turned it into a WM_*CHAR message:
    if (__PeekMessage(&msg, hWnd, WM_CHAR, WM_SYSDEADCHAR, PM_REMOVE)) {
      uMsg = msg.message;
      wParam = msg.wParam;
      lParam = msg.lParam;
    }
    // fall through to the character case:
  case WM_DEADCHAR:
  case WM_SYSDEADCHAR:
  case WM_CHAR:
  case WM_SYSCHAR: {
    // Mask keeps the mouse button state:
    // Alt gets reported for the Alt-GR switch on foreign keyboards,
    // so we ignore it for WM_CHAR messages:
    e_state = (e_state & ANY_BUTTON) | shiftflags(uMsg==WM_CHAR);
    if (lParam & (1<<31)) { // key up events.
      e_is_click = (recent_keysym == e_keysym);
      recent_keysym = 0;
      if (handle(KEYUP, window)) return 0;
      break;
    }
    // if same as last key, increment repeat count:
    if (lParam & (1<<30)) {
      e_key_repeated++;
      recent_keysym = 0;
    } else {
      e_key_repeated = 0;
      recent_keysym = e_keysym;
    }

    // translate to text:
    static char buffer[31]; // must be big enough for fltk::compose() output
    static char dbcsbuf[3];
    if (uMsg == WM_CHAR || uMsg == WM_SYSCHAR) {
      if (e_keysym==ReturnKey || e_keysym==KeypadEnter) {
	buffer[0] = '\r';
	e_length = 1;
      } else if (has_unicode()) {
	// If we have registered UNICODE window under NT4, 2000, XP
	// We get WCHAR as wParam
	e_length = utf8encode((unsigned short)wParam, &buffer[0]);
      } else {
	if (!dbcsbuf[0] && IsDBCSLeadByte((unsigned char)wParam)) {
	  dbcsbuf[0] = (char)wParam;
	  break;
	}
	int dbcsl = 1;
	if(dbcsbuf[0]) {
	  dbcsbuf[1] = (unsigned char) wParam;
	  dbcsl = 2;
	} else {
	  dbcsbuf[0] = (unsigned char) wParam;
	}
	dbcsbuf[2] = 0;
	e_length = utf8frommb(buffer, sizeof(buffer), dbcsbuf, dbcsl);
	if (e_length >= sizeof(buffer)) e_length = sizeof(buffer)-1;
	dbcsbuf[0] = 0;
      }
    } else {
      dbcsbuf[0] = 0;
      e_length = 0;
    }
    buffer[e_length] = 0;
    e_text = buffer;

    // for (int i = lParam&0xff; i--;)
    if (window) while (window->parent()) window = window->window();
    handle(KEY,window);
    return 0;}

  case WM_MOUSEWHEEL: {
    static int ydelta = 0; // running total of all motion
    ydelta -= (SHORT)(HIWORD(wParam));
    if ((e_dy = ydelta / WHEEL_DELTA)) {
      if ((SHORT)(HIWORD(wParam)) < 0) {
        e_keysym = WheelUp;
	e_state |= BUTTON4;
      } else {
        e_keysym = WheelDown;
	e_state |= BUTTON5;
      }
      ydelta -= e_dy * WHEEL_DELTA;
      handle(MOUSEWHEEL, xmousewin ? xmousewin : window);
    }
    return 0;
  }

  case WM_MOUSEHWHEEL: {
	  puts("MOUSEHWHEEL!");
    static int xdelta = 0;
    xdelta -= (SHORT)(HIWORD(wParam));
    if ((e_dx = xdelta / WHEEL_DELTA)) {
      if ((SHORT)(HIWORD(wParam)) < 0) {
        e_keysym = WheelLeft;
	e_state |= BUTTON6;
      } else {
        e_keysym = WheelRight;
	e_state |= BUTTON7;
      }
      xdelta -= e_dx * WHEEL_DELTA;
      handle(MOUSEWHEEL, xmousewin ? xmousewin : window);
    }
  }

  case WM_GETMINMAXINFO:
    if (window) CreatedWindow::find(window)->set_minmax((LPMINMAXINFO)lParam);
    break;

  case WM_SHOWWINDOW:
    if (!window) break;
    if (wParam) { // Map event
      if (window->parent()) break; // ignore child windows
      // figure out where OS put automatically-placed windows:
      if (window->x()==USEDEFAULT || window->y()==USEDEFAULT) {
	POINT wul = { 0, 0 }; ClientToScreen(xid(window), &wul);
	window->x(wul.x);
	window->y(wul.y);
      }
      MakeWaitReturn();
    } else { // Unmap event
      CreatedWindow::find(window)->wait_for_expose = true;
    }
    break;

  case WM_MAKEWAITRETURN:
    // save any data from fltk::awake() call:
    if (wParam) thread_message_ = (void*)wParam;
    // This will be called if MakeWaitReturn fails because Stoopid Windows
    // called the WndProc directly. Instead do the best we can, which is
    // to flush the display.
    flush();
    break;

  case WM_WINDOWPOSCHANGING:
    {
      if (!window || window->parent()) break; // ignore child windows
      // fltk does not think making a window iconic is a size change, but
      // Windows does. This makes it ignore changes when the window is
      // made iconic, and the "ignore_size_change_window" is used to
      // ignore changes when it is de-iconized.
      if ( window->iconic() ) break;
      fltk::Rectangle r; window->borders(&r);
      WINDOWPOS *pos = (WINDOWPOS*)lParam;
      if (hWnd == ignore_size_change_window) {
	ignore_size_change_window = 0;
        // Record the automatic position:
	if (window->x()==USEDEFAULT) window->x(pos->x-r.x());
	if (window->y()==USEDEFAULT) window->y(pos->y-r.y());
      } else {
	fltk::Rectangle newRect;
	if ( pos->flags & SWP_NOMOVE ) {
	  newRect.x( window->x() );
	  newRect.y( window->y() );
	} else {
	  newRect.x( pos->x-r.x() );
	  newRect.y( pos->y-r.y() );
	}
	if ( pos->flags & SWP_NOSIZE ) {
	  newRect.w( window->w() );
	  newRect.h( window->h() );
	} else {
	  newRect.w( pos->cx-r.w() );
	  newRect.h( pos->cy-r.h() );
	}
	if ( window->resize( newRect.x(), newRect.y(), newRect.w(), newRect.h() ) ) {
	  window->layout_damage( window->layout_damage() | LAYOUT_USER );
	  window->layout();
	  pos->x = window->x()+r.x();
	  pos->y = window->y()+r.y();
	  pos->cx = window->w()+r.w();
	  pos->cy = window->h()+r.h();
	}
      }
    }
    break;

  case WM_SETCURSOR:
    if (window && LOWORD(lParam) == HTCLIENT) {
      while (!window->is_window() && window->parent()) 
	  window = window->window();
      CreatedWindow* i = CreatedWindow::find(window);
      if (i && i->cursor != default_cursor &&
	  !i->cursor_for->contains(belowmouse_))
	i->cursor = default_cursor;
      SetCursor(i->cursor);
      return 0;
    }
    break;

#if USE_COLORMAP
  case WM_QUERYNEWPALETTE :
    {
      CreatedWindow* i = CreatedWindow::find(window);
      if (fl_select_palette(i->dc)) InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

  case WM_PALETTECHANGED:
    if (!window) break; // see STR #1902
    if ((HWND)wParam != hWnd) {
      CreatedWindow* i = CreatedWindow::find(window);
      if (fl_select_palette(i->dc)) UpdateColors(i->dc);
    }
    break;
#endif

  case WM_CREATE :
    tablet_open(hWnd, 0);
    break;

  case WM_DISPLAYCHANGE:
  case WM_SETTINGCHANGE:
    reload_info = true;
    if ( monitors != &allMonitors )
      delete[] monitors;
    monitors = 0;
    num_monitors = 0;
  case WM_SYSCOLORCHANGE:
    reload_theme();
    break;

  case WM_DESTROYCLIPBOARD:
    i_own_selection = false;
    return 1;

  case WM_RENDERALLFORMATS:
    renderallformats();
    return 1;

  case WM_RENDERFORMAT:
    if (wParam == CF_TEXT)
      SetClipboardData(CF_TEXT, fl_global_selection_ansi(1));
    else
      SetClipboardData(CF_UNICODETEXT, fl_global_selection(1));
    // Windoze also seems unhappy if I don't do this. Documentation very
    // unclear on what is correct:
    return 1;

  case WT_PACKET: { // stylus motion/pressure packet
    static PACKET pkt;
    //if (wtPacket((HCTX)lParam, wParam, &pkt)) {
    if (wtPacket(wintab_ctx, wParam, &pkt)) {
      e_pressure = (((float)pkt.pkNormalPressure)+pressure_add)*pressure_mul;
      float a = pkt.pkOrientation.orAzimuth / 1800.0f * 3.141592654f;
      float b = (900-pkt.pkOrientation.orAltitude) / 900.0f;
      e_x_tilt = sinf(a) * b;
      e_y_tilt = cosf(a) * b;
      //++ incidently, the device numbers are correct for pen and eraser on Wacom
      //++ however, this code must be updated for other devices to function properly
      e_device = pkt.pkCursor;
    } }
    return 1;

  case WT_PROXIMITY: // stylus proximity packet
    stylus_data_valid = LOWORD(lParam)!=0;
    return 1;

  default:
    if (handle(0,0)) return 0;
    break;
  }

  return __DefWindowProc(hWnd, uMsg, wParam, lParam);
}

////////////////////////////////////////////////////////////////

void Window::borders( fltk::Rectangle *r ) const
{
  if (!this->border() || this->override() || this->parent()) {
    r->set(0,0,0,0);
    return;
  }
  // Request the border sizes using new API:
  LONG style;
  LONG styleEx;
  if (shown()) {
    // get the style flags that were used to create the window:
    style = GetWindowLong(i->xid, GWL_STYLE);
    styleEx = GetWindowLong(i->xid, GWL_EXSTYLE);
  } else {
    // generate the values that window->show() would use:
    if (resizable() || maxw != minw || maxh != minh) // resizable
      style = WS_THICKFRAME | WS_MAXIMIZEBOX | WS_CAPTION;
    else
      style = WS_DLGFRAME | WS_CAPTION;
    style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    if (!contains(modal()))
      style |= WS_SYSMENU | WS_MINIMIZEBOX;
    else
      style |= WS_SYSMENU ; // keep the possibility to close the box as in osx and X11
    styleEx = WS_EX_LEFT | WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT;
  }
  RECT rect;
  rect.left = x();
  rect.top = y();
  rect.right = this->r();
  rect.bottom = b();
  // get the decoration rectangle for the desired client rectangle
  AdjustWindowRectEx(&rect, style, FALSE, styleEx);
  r->set(rect.left-x(), rect.top-y(),
	 rect.right-rect.left-w(),
	 rect.bottom-rect.top-h());
#if 0
  // The older api, which apparently does not return correct sizes on XP?
  int bt = GetSystemMetrics(SM_CYCAPTION);
  int dx, by;
  if (maxw != minw || maxh != minh) { // resizable
    dx = GetSystemMetrics(SM_CXSIZEFRAME);
    by = GetSystemMetrics(SM_CYSIZEFRAME);
  } else { // fixed-size
    dx = GetSystemMetrics(SM_CXFIXEDFRAME);
    by = GetSystemMetrics(SM_CYFIXEDFRAME);
  }
  r->set(-dx, -(bt+by), 2*dx, bt+2*by);
#endif
}

////////////////////////////////////////////////////////////////

void Window::layout() {
  // If only the xy position changed, then win32 did everything for us, so don't
  // call the Group layout.
  if (layout_damage() & ~LAYOUT_XY) Group::layout();
  // Fix the window:
  system_layout();
}

void Window::system_layout() {
  UINT flags;
  if (layout_damage() & LAYOUT_WH) {
    free_backbuffer();
    flags = SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE;
  } else if (layout_damage() & LAYOUT_XY) {
    flags = SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_NOSIZE;
  } else {
    flags = 0;
  }
  if (i && flags) {
    fltk::Rectangle r(*this);
    borders(&r);
    r.x(r.x()+x());
    r.y(r.y()+y());
    r.w(r.w()+w());
    r.h(r.h()+h());
    for (Widget* p = parent(); p && !p->is_window(); p = p->parent())
      r.move(p->x(), p->y());
    // avoid zero size windows. A zero sized window on Win32
    // will cause continuously new redraw events
    int W = r.w(); if (W < 1) W = 1;
    int H = r.h(); if (H < 1) H = 1;
    SetWindowPos(i->xid, 0, r.x(), r.y(), W, H, flags);
    if (!(flags & SWP_NOSIZE)) {redraw(); /*i->wait_for_expose = true;*/}
  }
}

////////////////////////////////////////////////////////////////
// Innards of Window::create():

void Window::create() {
  CreatedWindow::create(this);
}

const Window* fl_mdi_window = 0; // set by show_inside()
HCURSOR fltk::default_cursor;

static void register_unicode(HICON smallicon, HICON bigicon)
{
  static WNDCLASSEXW wc;
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)WndProc;
  wc.cbClsExtra = wc.cbWndExtra = 0;
  wc.hInstance = xdisplay;
  wc.hIcon = bigicon;
  wc.hIconSm = smallicon;
  if (!default_cursor) default_cursor = LoadCursor(NULL, IDC_ARROW);
  wc.hCursor = default_cursor;
  //uchar r,g,b; get_color(GRAY,r,g,b);
  //wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(r,g,b));
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = L"fltk";
  wc.cbSize = sizeof(wc);
  RegisterClassExW(&wc);
  // This is needed or multiple DLL's get confused (?):
  // No, doing this makes none of the windows appear:
  //UnregisterClass(wc.lpszClassName, xdisplay);
}

static void register_ansi(HICON smallicon, HICON bigicon)
{
  static WNDCLASSEXA wc;
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)WndProc;
  wc.cbClsExtra = wc.cbWndExtra = 0;
  wc.hInstance = xdisplay;
  wc.hIcon = bigicon;
  wc.hIconSm = smallicon;
  if (!default_cursor) default_cursor = LoadCursor(NULL, IDC_ARROW);
  wc.hCursor = default_cursor;
  //uchar r,g,b; get_color(GRAY,r,g,b);
  //wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(r,g,b));
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "fltk";
  wc.cbSize = sizeof(wc);
  RegisterClassExA(&wc);
  // This is needed or multiple DLL's get confused (?):
  // No, doing this makes none of the windows appear:
  //UnregisterClass(wc.lpszClassName, xdisplay);
}

void CreatedWindow::create(Window* window) {

  static bool registered = false;
  if (!registered) {
    registered = true;

    HICON icon = (HICON)window->icon();
    HICON bigicon = icon;
    if (!icon) {
      icon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(101),
			      IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR|LR_SHARED);
      bigicon = (HICON)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(101),
				 IMAGE_ICON, 32, 32,LR_DEFAULTCOLOR|LR_SHARED);
      if (!icon) icon = bigicon;
      if (!bigicon) bigicon = icon;
      if (!icon) icon = bigicon = LoadIcon(NULL, IDI_APPLICATION);
    }
    if(has_unicode()) {
      register_unicode(icon, bigicon);
    } else {
      register_ansi(icon, bigicon);
    }

#if USE_DRAGDROP
    OleInitialize(0L);
#endif

#if USE_IMM
    fl_use_imm32 = fl_load_imm32();
#endif
  }

  HWND parent;
  DWORD style;
  DWORD styleEx;

  int xp = window->x();
  int yp = window->y();
  int wp = window->w();
  int hp = window->h();

  if (window->parent()) {

    style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD;
    styleEx = WS_EX_LEFT | WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT;
    Widget* p = window->parent();
    while (!p->is_window()) {
      xp += p->x(); yp += p->y();
      p=p->parent();
    }
    parent = fltk::xid((Window*)p);

  } else if (window->override()) {

    // Ideally window should be above everything including taskbar, not
    // raise anything, and not alter the keyboard focus!
    style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP;
    styleEx = WS_EX_LEFT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    parent = 0;

  } else {

    if (!window->border())
      style = WS_POPUP;
    else if (window->maxw != window->minw || window->maxh != window->minh)
      style = WS_THICKFRAME | WS_MAXIMIZEBOX | WS_CAPTION;
    else
      style = WS_DLGFRAME | WS_CAPTION;
    style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    fltk::Rectangle r; window->borders(&r);
    if (!window->contains(modal()))
      style |= WS_SYSMENU | WS_MINIMIZEBOX;
    else
      style |= WS_SYSMENU ;// keep the possibility to close the box as in osx and X11
    styleEx = WS_EX_LEFT | WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT;
    if (xp != USEDEFAULT) xp += r.x();
    if (yp != USEDEFAULT) yp += r.y();
    wp += r.w();
    hp += r.h();
    if (window->child_of() && window->child_of()->shown()) {
      parent = window->child_of()->i->xid;
    } else if (fl_mdi_window) {
      parent = fl_mdi_window->i->xid;
      styleEx |= WS_EX_MDICHILD;
    } else {
      parent = 0;
    }

  }

  CreatedWindow* x = new CreatedWindow;
  x->backbuffer = 0;
  x->overlay = false;
  x->window = window; window->i = x;
  x->region = 0;
  x->cursor = default_cursor;
  x->cursor_for = 0;
  const char *name = window->label();

  wchar_t ucs_name[1024];
  if (name && *name) utf8towc(name, strlen(name), ucs_name, 1024);
  else ucs_name[0] = 0;

  // CreateWindowEx enters WndProc by sending many important msgs like WM_GETMINMAXINFO *before* the window handle (xid here) is set in the instance..
  sCreatedWindowPending=x; // start of hack
  x->xid = __CreateWindowExW(styleEx,
			     L"fltk", ucs_name, style,
			     xp, yp, wp, hp,
			     parent,
			     NULL, // menu
			     xdisplay,
			     NULL // creation parameters
			     );

  sCreatedWindowPending=0; // end of hack

  x->dc = GetDC(x->xid);
  SetTextAlign(x->dc, TA_BASELINE|TA_LEFT);
  SetBkMode(x->dc, TRANSPARENT);
  x->bdc = 0;

  x->wait_for_expose = true;
  x->next = CreatedWindow::first;
  CreatedWindow::first = x;

  // How to set the icon:
  //SendMessage(x->xid, WM_SETICON, ICON_BIG, (LPARAM)bigicon);
  //SendMessage(x->xid, WM_SETICON, ICON_SMALL, (LPARAM)smallicon);
  // where bigicon & small icon are HICON

#if USE_DRAGDROP
  RegisterDragDrop(x->xid, &flDropTarget);
#endif
}

////////////////////////////////////////////////////////////////

HINSTANCE fltk::xdisplay = NULL;

extern "C" {
  // Forward declaration of emulation functions
  HWND WINAPI ansi_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
  HMODULE WINAPI ansi_LoadLibraryW(LPCWSTR lpFileName);
  BOOL WINAPI ansi_SetWindowTextW(HWND hWnd, LPCWSTR lpString);
  int WINAPI ansi_MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
  HFONT WINAPI ansi_CreateFontIndirectW(CONST LOGFONTW *);
  BOOL WINAPI ansi_GetTextMetricsW(HDC, LPTEXTMETRICW);
};

pfCreateWindowExW       __CreateWindowExW	= CreateWindowExW;
pfLoadLibraryW		__LoadLibraryW		= LoadLibraryW;
pfSetWindowTextW	__SetWindowTextW	= SetWindowTextW;
pfPeekMessage		__PeekMessage		= PeekMessageW;
pfGetMessage		__GetMessage		= GetMessageW;
pfDispatchMessage       __DispatchMessage	= DispatchMessageW;
pfPostMessage		__PostMessage		= PostMessageW;
pfDefWindowProc		__DefWindowProc		= DefWindowProcW;
pfMessageBoxW		__MessageBoxW		= MessageBoxW;
pfCreateFontIndirectW	__CreateFontIndirectW	= CreateFontIndirectW;
pfGetTextMetricsW	__GetTextMetricsW	= GetTextMetricsW;

void fltk::open_display() {
  static int been_here=0;
  if(been_here) return;
  been_here = 1;

  if (has_unicode()) {
    // Setup our function pointers to "W" functions
    fltk::xdisplay              = GetModuleHandleW(NULL);
  } else {
    // Setup our function pointers to "A" and emulation functions
    fltk::xdisplay              = GetModuleHandleA(NULL);
    __CreateWindowExW           = ansi_CreateWindowExW;
    __LoadLibraryW		= ansi_LoadLibraryW;
    __SetWindowTextW	        = ansi_SetWindowTextW;
    __PeekMessage		= PeekMessageA;
    __GetMessage		= GetMessageA;
    __DispatchMessage	        = DispatchMessageA;
    __PostMessage		= PostMessageA;
    __DefWindowProc		= DefWindowProcA;
    __MessageBoxW		= ansi_MessageBoxW;
    __CreateFontIndirectW       = ansi_CreateFontIndirectW;
    __GetTextMetricsW           = ansi_GetTextMetricsW;
  }
}
void fltk::close_display() {}

void Window::size_range_() {
  size_range_set = 1;
}

void CreatedWindow::set_minmax(LPMINMAXINFO minmax)
{
  fltk::Rectangle r; window->borders(&r);

  minmax->ptMinTrackSize.x = window->minw + r.w();
  minmax->ptMinTrackSize.y = window->minh + r.h();
  if (window->maxw) {
    minmax->ptMaxTrackSize.x =
      minmax->ptMaxSize.x = window->maxw + r.w();
  }
  if (window->maxh) {
    minmax->ptMaxTrackSize.y =
      minmax->ptMaxSize.y = window->maxh + r.h();
  }
}

/**
 * Returns true if the window is shown but is iconized.
 */
bool Window::iconic() const {
  return i && IsIconic(i->xid);
}

////////////////////////////////////////////////////////////////

void Window::label(const char *name,const char *iname) {
  Widget::label(name);
  iconlabel_ = iname;
  if (i && !parent()) {
    wchar_t ucs_name[1024];
    if (name && *name) utf8towc(name, strlen(name), ucs_name, 1024);
    else ucs_name[0] = 0;
    __SetWindowTextW(i->xid, ucs_name);
    // if (!iname) iname = filename_name(name);
    // should do something with iname here, it should label the taskbar icon
  }
}

////////////////////////////////////////////////////////////////
// Drawing context
//
// DC's are more of a pain in Windows...
//
// Each window has a DC made with GetDC(window). If I understand the
// documentation right, this is destroyed/released when the window
// is destroyed.
//
// Calling Image::make_current() will make a temporary DC. This is
// kept until you draw into another image, or you destroy the image.
//
// ImageDraw creates a temporary DC. See Image.cxx

const Window *Window::drawing_window_;
int fl_clip_w, fl_clip_h;

/** The device context that is currently being drawn into. */
HDC fltk::dc;

#if USE_CAIRO
static void cairo_invalidate_context() {
  // invalidate cairo context so that it will be resynchronized
  // on next first real draw()
    if (fltk::cr) {
	cairo_destroy (fltk::cr); // delete previous context pointing on old hdc
	fltk::cr = 0; // don't create now, wait for draw()
    }
    return;
}
#endif

void Widget::make_current() const {
  int x = 0;
  int y = 0;
  const Widget* widget = this;
  while (!widget->is_window()) {
    x += widget->x();
    y += widget->y();
    widget = widget->parent();
  }
  const Window* window = (const Window*)widget;
  Window::drawing_window_ = window;
  fl_clip_w = window->w();
  fl_clip_h = window->h();
  dc = CreatedWindow::find( window )->dc;
#if USE_CAIRO
  cairo_invalidate_context();
#endif
  fl_select_palette(dc);
  load_identity();
  translate(x,y);
}

HDC fl_bitmap_dc;
namespace fltk {class Image;}
fltk::Image* fl_current_Image;

void fltk::draw_into(HBITMAP bitmap, int w, int h) {
  if (!fl_bitmap_dc) {
    fl_bitmap_dc = CreateCompatibleDC(getDC());
    SetTextAlign(fl_bitmap_dc, TA_BASELINE|TA_LEFT);
    SetBkMode(fl_bitmap_dc, TRANSPARENT);
  }
  SelectObject(fl_bitmap_dc, bitmap);
  dc = fl_bitmap_dc;
  fl_select_palette(dc);
#if USE_CAIRO
    cairo_surface_t * surface =cairo_win32_surface_create(dc);
    cairo_invalidate_context();
    cc = cairo_create(surface);
    cairo_surface_destroy(surface);
#endif
  load_identity();
  fl_clip_w = w;
  fl_clip_h = h;
  fl_current_Image = 0;
}

#if __BORLANDC__ || __DMC__
  void fltk::stop_drawing(HWND window) {}
#else
  /** Destroy any dc or other objects used to draw into this window. */
  void fltk::stop_drawing(HWND window) {}
  void fltk::stop_drawing(HBITMAP bitmap) {}
#endif

static HDC screen_dc = 0;

/** Return an arbitrary HDC which you can use for Win32 functions that
    need one as an argument. The returned value is short-lived and may
    be destroyed the next time anything is drawn into a window!
*/
HDC fltk::getDC() {
  //if (dc) return dc;
  if (CreatedWindow::first) return CreatedWindow::first->dc;
  if (screen_dc) ReleaseDC(0,screen_dc);
  return (screen_dc = GetDC(0));
}

////////////////////////////////////////////////////////////////
// Window update, double buffering, and overlay:
void*Window::backbuffer() const {return i ? (void*)i->bdc:0;}

void Window::flush() {

  drawing_window_ = this;
  fl_clip_w = w();
  fl_clip_h = h();

  unsigned char damage = this->damage();

  if (this->double_buffer() || i->overlay) {
    // double-buffer drawing

    // ML: Set eraseoverlay=true only if flush is caused by Window::redraw_overlay()
    //     Otherwise (flush from wm) set it to false, and set clip region correctly
    //     so drawing is lot faster.
    //bool eraseoverlay = i->overlay || (damage&DAMAGE_OVERLAY);
    bool eraseoverlay = i->overlay && (damage&DAMAGE_OVERLAY);

    if (eraseoverlay) damage &= ~DAMAGE_OVERLAY;

    if (!i->backbuffer) { // we need to create back buffer
      i->backbuffer = CreateCompatibleBitmap(i->dc, w(), h());
      i->backbuffer_bad = true;
      i->bdc = CreateCompatibleDC(i->dc);
      SelectObject(i->bdc, i->backbuffer);
      SetTextAlign(i->bdc, TA_BASELINE|TA_LEFT);
      SetBkMode(i->bdc, TRANSPARENT);
    }

    // draw the back buffer if it needs anything:
    if (damage || i->backbuffer_bad) {
      // set the graphics context to draw into back buffer:
      dc = i->bdc;
      fl_select_palette(dc);
#if USE_CAIRO
      cairo_invalidate_context();
#endif
      load_identity();
      if ((damage & DAMAGE_ALL) || i->backbuffer_bad) {
	set_damage(DAMAGE_ALL);
	draw();
      } else {
	// draw all the changed widgets:
	if (damage & ~DAMAGE_EXPOSE) {
	  set_damage(damage & ~DAMAGE_EXPOSE);
	  draw();
	}
	// draw for any expose events (if Xdbe is not being used this will
	// only happen for redraw(x,y,w,h) calls):
	if (i->region) {
	  clip_region(i->region); i->region = 0;
	  set_damage(DAMAGE_EXPOSE); draw();
	  clip_region(0);
	}
      }
      i->backbuffer_bad = false;
      //fl_restore_clip(); // duplicate region into new dc (there is none)
    }

    dc = i->dc;

    // Clip the copying of the pixmap to the damage area,
    // this makes it faster, especially if the damage area is small:
    if (!damage && !eraseoverlay) {
      clip_region(i->region); i->region = 0;
    }

    // Copy the backbuffer to the window:
    // Must be an implementation problem in the server, but on Irix (at least)
    // it is much faster if I clip the rectangle requested down:
    Rectangle r(w(),h());
    if (intersect_with_clip(r))
      BitBlt(dc, r.x(), r.y(), r.w(), r.h(), i->bdc, r.x(), r.y(), SRCCOPY);

    if (i->overlay) draw_overlay();
    clip_region(0);

  }  else {

    // Single buffer drawing
    dc = i->dc;
    fl_select_palette(dc);
#if USE_CAIRO
    cairo_invalidate_context();
#endif
    load_identity();
    if (damage & ~DAMAGE_EXPOSE) {
      set_damage(damage & ~DAMAGE_EXPOSE);
      draw();
      i->backbuffer_bad = true;
    }
    if (i->region && !(damage & DAMAGE_ALL)) {
      clip_region(i->region); i->region = 0;
      set_damage(DAMAGE_EXPOSE); draw();
      clip_region(0);
    }
  }
}

/*! Get rid of extra storage created by drawing when double_buffer() was
  turned on. */
void Window::free_backbuffer() {
  if (!i || !i->backbuffer) return;
  DeleteDC(i->bdc);
  i->bdc = 0;
  DeleteObject(i->backbuffer);
  i->backbuffer = 0;
}

////////////////////////////////////////////////////////////////

extern void fl_font_rid();

namespace fltk {

static struct Cleanup { ~Cleanup(); } cleanup;

Cleanup::~Cleanup() {

  // save the clipboard:
  renderallformats();

  // I believe newer w2k is fixed to allow programs to crash or abort
  // without wasting GDI resources. However this code was apparently
  // necessary on Win98 and earler to free some large objects stored
  // in shared DLL's and in the OS itself:
  while (CreatedWindow* x = CreatedWindow::first) x->window->destroy();
  if (fl_bitmap_dc) DeleteDC(fl_bitmap_dc);

  // get rid of allocated font resources
  fl_font_rid();

  // free stuff needed by tablet api:
  if (wintab_ctx) {
    wtClose(wintab_ctx);   // it is: close the device driver
    wintab_ctx = 0;
    wintab_hwnd = 0;
  }
  if (wintab_dll) {
    FreeLibrary(wintab_dll);
    wintab_dll = 0;
  }

  if (screen_dc) ReleaseDC(0,screen_dc);
}

}

extern "C" {

// Convert widechar to multibyte
// src length is determited with wcslen
static inline void CVT2ANSI(LPCWSTR src, char dst[], int dstlen) {
  // WAS: Should this use GetACP() instead of CP_ACP?
  // ML: I think it does same thing internally?
  // WAS: apparently this call does not null-terminate, even though
  // msdn documentation claims it does.
  int len = 0;
  if (src && *src)
    len = WideCharToMultiByte(CP_ACP, 0, src, wcslen(src), dst, dstlen-1, NULL, NULL);
  dst[len] = 0;
}

HWND WINAPI ansi_CreateWindowExW(DWORD dwExStyle,
				LPCWSTR lpClassName,
				LPCWSTR lpWindowName,
				DWORD dwStyle,
				int x,
				int y,
				int nWidth,
				int nHeight,
				HWND hWndParent,
				HMENU hMenu,
				HINSTANCE hInstance,
				LPVOID lpParam
				)
{
  char ansi_windowname[1024], ansi_classname[1024];
  CVT2ANSI(lpWindowName, ansi_windowname, 1024);
  CVT2ANSI(lpClassName,	 ansi_classname,  1024);
  return CreateWindowExA(dwExStyle, ansi_classname, ansi_windowname, dwStyle, x,y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);
}

HMODULE WINAPI ansi_LoadLibraryW(LPCWSTR lpFileName)
{
  char ansi_filename[MAX_PATH];
  CVT2ANSI(lpFileName, ansi_filename, MAX_PATH);
  return LoadLibraryA(ansi_filename);
}

BOOL WINAPI ansi_SetWindowTextW(HWND hWnd, LPCWSTR lpString)
{
  char ansi_windowname[1024];
  CVT2ANSI(lpString, ansi_windowname, 1024);
  return SetWindowTextA(hWnd, ansi_windowname);
}

HFONT WINAPI ansi_CreateFontIndirectW(CONST LOGFONTW *lFont)
{
  LOGFONTA lFontA;
  lFontA.lfCharSet      = lFont->lfCharSet;
  lFontA.lfClipPrecision= lFont->lfClipPrecision;
  lFontA.lfEscapement   = lFont->lfEscapement;
  lFontA.lfHeight       = lFont->lfHeight;
  lFontA.lfItalic       = lFont->lfItalic;
  lFontA.lfOrientation  = lFont->lfOrientation;
  lFontA.lfOutPrecision = lFont->lfOutPrecision;
  lFontA.lfPitchAndFamily = lFont->lfPitchAndFamily;
  lFontA.lfQuality      = lFont->lfQuality;
  lFontA.lfStrikeOut    = lFont->lfStrikeOut;
  lFontA.lfUnderline    = lFont->lfUnderline;
  lFontA.lfWeight       = lFont->lfWeight;
  lFontA.lfWidth        = lFont->lfWidth;
  CVT2ANSI(lFont->lfFaceName, lFontA.lfFaceName, LF_FACESIZE);
  return CreateFontIndirectA(&lFontA);
}

BOOL WINAPI ansi_GetTextMetricsW(HDC dc, LPTEXTMETRICW metr)
{
  TEXTMETRICA metrA;
  BOOL ret = GetTextMetricsA(dc, &metrA);
#define COPY(x) metr->x = metrA.x
  COPY(tmHeight);
  COPY(tmAscent);
  COPY(tmDescent);
  COPY(tmInternalLeading);
  COPY(tmExternalLeading);
  COPY(tmAveCharWidth);
  COPY(tmMaxCharWidth);
  COPY(tmWeight);
  COPY(tmOverhang);
  COPY(tmDigitizedAspectX);
  COPY(tmDigitizedAspectY);
  COPY(tmFirstChar);
  COPY(tmLastChar);
  COPY(tmDefaultChar);
  COPY(tmBreakChar);
  COPY(tmItalic);
  COPY(tmUnderlined);
  COPY(tmStruckOut);
  COPY(tmPitchAndFamily);
  COPY(tmCharSet);
  return ret;
}

int WINAPI ansi_MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
  char ansi_text[4096];
  char ansi_caption[1024];
  CVT2ANSI(lpText, ansi_text, 4096);
  CVT2ANSI(lpCaption, ansi_caption, 1024);
  return MessageBoxA(hWnd, ansi_text, ansi_caption, uType);
}

}; /* extern "C" */


FILE* fltk::fltk_fopen(const char* name, const char* flags) {
	wchar_t *wname, *wflags;
	unsigned namelen, flaglen;

	flaglen = utf8towc(flags, strlen(flags), NULL, 0);
	wflags = new wchar_t [flaglen+10];
	utf8towc(flags, strlen(flags), wflags, flaglen+1);
	namelen = utf8towc(name, strlen(name), NULL, 0);
	wname = new wchar_t [namelen+10];
	utf8towc(name, strlen(name), wname, namelen+1);

	FILE* ret = _wfopen(wname, wflags);
	delete [] wname;
	delete [] wflags;
	return ret;
}
//
// End of "$Id: run.cxx 8556 2011-04-02 02:41:00Z bgbnbigben $".
//
