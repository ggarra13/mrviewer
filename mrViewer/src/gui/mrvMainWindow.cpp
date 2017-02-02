/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mrvMainWindow.cpp
 * @author gga
 * @date   Wed Jul  4 23:17:46 2007
 * 
 * @brief  main window for mrViewer
 * 
 * 
 */

#include <iostream>

// Must come before fltk/x11.h
#include "mrViewer.h"
#include "gui/mrvImageView.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvIO.h"

#include <fltk/Cursor.h>
#include <fltk/events.h>
#include <fltk/layout.h>
#include <fltk/run.h>


#if !defined(_WIN32)
#  include <fltk/x11.h>
#undef Bool
#define Bool int
#  include <X11/xpm.h>
#undef Bool
#  include "icons/viewer16.xpm"
#else
#  include <windows.h>
#  include <fltk/win32.h>
#  include "resource.h"
#endif


using namespace std;

namespace mrv {

#ifndef _WIN32
  Pixmap p, mask;
#endif

  MainWindow::MainWindow( int W, int H, const char* title ) :
    fltk::Window( W, H, title )
  {
#ifdef _WIN32
      // Set icon does not work on Linux yet
      set_icon();
#endif
  }

  MainWindow::~MainWindow()
  {
      DBG( "*****************************************************" );
      uiMain->uiView->stop();
      delete uiMain->uiView;
      uiMain->uiView = NULL;
  }


  void MainWindow::set_icon()
  {
    fltk::open_display();  // Needed for icons 
    
#if defined(_WIN32) || defined(_WIN64)
    HICON data = LoadIcon(fltk::xdisplay, MAKEINTRESOURCE(IDI_ICON1));
    this->icon(data);
#else

    // XpmCreatePixmapFromData comes from libXpm (libgd-xpm* on Debian)
    int ok = XpmCreatePixmapFromData(fltk::xdisplay,
                                     DefaultRootWindow(fltk::xdisplay),
                                     const_cast<char**>( viewer16_xpm ),
                                     &p, NULL, NULL);

    if ( ok == XpmSuccess )
      {
          this->icon(reinterpret_cast<char*>( p ) );
      }
#endif

  }


  void MainWindow::always_on_top() 
  {
#if defined(_WIN32) || defined(_WIN64)
    // Microsoft (R) Windows(TM)
    SetWindowPos(fltk::xid(this), HWND_TOPMOST, 
		 0, 0, w()+8, h()+27, 0);
#else
    // XOrg / XWindows(TM)
    XEvent ev;
    static const char* const names[2] = { "_NET_WM_STATE", 
					  "_NET_WM_STATE_ABOVE" };
    Atom atoms[ 2 ];
    fltk::open_display();
    XInternAtoms(fltk::xdisplay, (char**)names, 2, False, atoms );
    Atom net_wm_state = atoms[ 0 ];
    Atom net_wm_state_above = atoms[ 1 ];
    ev.type = ClientMessage;
    ev.xclient.window = fltk::xid(this);
    ev.xclient.message_type = net_wm_state;
    ev.xclient.format = 32;
    ev.xclient.data.l[ 0 ] = active() ? 1 : 0;
    ev.xclient.data.l[ 1 ] = net_wm_state_above;
    ev.xclient.data.l[ 2 ] = 0;
    XSendEvent(fltk::xdisplay, 
	       DefaultRootWindow(fltk::xdisplay),  False, 
	       SubstructureNotifyMask|SubstructureRedirectMask, &ev);
#endif
  } // above_all function


  /** 
   * Iconize all windows
   * 
   */
  void MainWindow::iconize_all()
  {
    fltk::Window* uiReelWindow = uiMain->uiReelWindow->uiMain;
    if (uiReelWindow) uiReelWindow->iconize();
    return fltk::Window::iconize();
  }

  /** 
   * Handle MainWindow's fltk::events
   * 
   * @param event fltk::event enumeration
   * 
   * @return 1 if handled, 0 if not.
   */
  int MainWindow::handle( int event )
  {
    return fltk::Window::handle( event );
  }

void MainWindow::layout()
{
   fltk::Window::layout();

   if ( layout_damage() & fltk::LAYOUT_W || layout_damage() & fltk::LAYOUT_H )
   {
      if ( uiMain->uiPrefs->uiPrefsAutoFitImage->value() )
         uiMain->uiView->fit_image();
   }
}

} // namespace mrv
