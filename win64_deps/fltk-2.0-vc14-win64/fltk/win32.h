// "$Id: win32.h 8752 2011-05-29 08:44:01Z bgbnbigben $"
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

/** \file

Declarations of FLTK symbols and interfaces that only exist if FLTK is
compiled on Windows. It is recommended you avoid using this header
file, and that you segregate code requiring it to it's own source
file.

This header includes the horrible <windows.h> header file, followed
by a large list of undef's to get rid of name conflicts. It is recommended
you use this if you need any windows functions rather than including
that file directly.

You can probably combine FLTK with other libraries that make their own
WIN32 window classes.  The easiest way is to call fltk::wait(), it
will call DispatchMessage() for all messages to the other windows. If
your other library insists on reading all the events, it will still
work (as long as it calls DispatchMessage()), but you will
have to arrange for the function fltk::flush() to be called regularily
so that widgets are updated. Timeouts, the idle function, and file
descriptor callbacks will not work in this case.

Many of the functions have the same name and purpose as ones defined
in x11.h, just with different return types. Due to how Doxygen works,
the X version of these is described here.
*/

#if defined(_MSC_VER)
# pragma once /* speeds up compilation */
#endif

#ifndef fltk_win32_h
#define fltk_win32_h

#ifndef DOXYGEN

#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif

# include <windows.h>
# include <winuser.h>
# undef DELETE
# undef ERROR
# undef IN
# undef OPAQUE
# undef OUT
# undef POINT
//#undef far
# undef max
# undef min
//#undef near

#if USE_CAIRO
# include <fltk/fltk_cairo.h>
# include <cairo-win32.h>
#endif

extern "C" {

// Function pointer declarations
// WAS: I suspect these can be put into win32/run.cxx!

typedef HWND (WINAPI *pfCreateWindowExW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
typedef HMODULE (WINAPI *pfLoadLibraryW)(LPCWSTR lpFileName);
typedef BOOL (WINAPI *pfPeekMessage)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
typedef BOOL (WINAPI *pfGetMessage)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
typedef LRESULT (WINAPI *pfDispatchMessage)(const MSG *lpmsg);
typedef BOOL (WINAPI *pfSetWindowTextW)(HWND hWnd, LPCWSTR lpString);
typedef LRESULT (WINAPI *pfDefWindowProc)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL (WINAPI *pfPostMessage)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef int (WINAPI *pfMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

typedef HFONT (WINAPI *pfCreateFontIndirectW)(CONST LOGFONTW *);
typedef BOOL (WINAPI *pfGetTextMetricsW)(HDC, LPTEXTMETRICW);

extern pfCreateWindowExW __CreateWindowExW;
extern pfLoadLibraryW	 __LoadLibraryW;
extern pfPeekMessage	 __PeekMessage;
extern pfGetMessage	 __GetMessage;
extern pfDispatchMessage __DispatchMessage;
extern pfSetWindowTextW	 __SetWindowTextW;
extern pfDefWindowProc	 __DefWindowProc;
extern pfPostMessage	 __PostMessage;
extern pfMessageBoxW	 __MessageBoxW;

extern pfCreateFontIndirectW __CreateFontIndirectW;
extern pfGetTextMetricsW     __GetTextMetricsW;

}; /* extern "C" */
#endif // !DOXYGEN

#include "draw.h"

namespace fltk {

/// \name fltk/win32.h
//@{

////////////////////////////////////////////////////////////////
// constant information about the display:

extern FL_API void	open_display();
extern FL_API void	close_display();
extern FL_API HINSTANCE	xdisplay;
extern FL_API HPALETTE	xpalette; // non-zero only on 8-bit displays!

////////////////////////////////////////////////////////////////
// event handling:

extern FL_API MSG msg;

////////////////////////////////////////////////////////////////
// drawing functions:

extern FL_API HDC	dc;
extern FL_API HDC	getDC();
extern FL_API HFONT	xfont();
extern FL_API TEXTMETRICW* textmetric();
extern FL_API COLORREF	current_xpixel;
extern FL_API COLORREF	xpixel(Color i);
extern FL_API HPEN	setpen();
extern FL_API HBRUSH	setbrush();
extern FL_API void	clip_region(HRGN);
extern FL_API HRGN	clip_region();

extern FL_API void	draw_into(HBITMAP, int w, int h);
extern FL_API void	stop_drawing(HBITMAP);
extern FL_API void	stop_drawing(HWND);

////////////////////////////////////////////////////////////////
#ifdef fltk_Window_h // only include this if <fltk/Fl_Window.h> was included

/** When fltk tells WIN32 about a Window, one of these objects is created.
    Warning: this object is highly subject to change!  It's definition
    is only here so that fl_xid can be declared inline
*/

class FL_API CreatedWindow {
public:
  HWND xid;
  HDC dc;
  HBITMAP backbuffer;
  HDC bdc;
  Window* window;
  HRGN region;
  void expose(const Rectangle&);
  CreatedWindow* next;
  bool wait_for_expose;
  bool backbuffer_bad; // used for XDBE
  bool overlay; // true if redraw_overlay was called
  HCURSOR cursor;
  const Widget* cursor_for;
  static CreatedWindow* first;
  static CreatedWindow* find(const Window* window) {return window->i;}
  void set_minmax(LPMINMAXINFO minmax);
  static void create(Window*);
};

/** Converts a Window to a xid
    \param w The Window to find
    \return The Window's HWND xid
*/
inline HWND xid(const Window*w) {return CreatedWindow::find(w)->xid;}
/** Converts a xid to a Window
    \param xid The HWND xid to find
    \return The window structure this HWND refers to, or NULL if xid is invalid
*/
Window* find(HWND xid);

extern FL_API HCURSOR default_cursor;

#endif // Fl_Window_H

//@}

}

#endif

//
// End of "$Id: win32.h 8752 2011-05-29 08:44:01Z bgbnbigben $".
//
