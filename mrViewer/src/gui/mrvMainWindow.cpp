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

// Must come before FL/Fl_x11.H
#include "mrViewer.h"
#include "gui/mrvImageView.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvIO.h"

#include <FL/Enumerations.H>
#include <FL/Fl.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/platform.H>


#include "icons/viewer16.xpm"


#include "mrvPreferencesUI.h"
#include "mrvReelUI.h"


using namespace std;

namespace mrv {


MainWindow::MainWindow( int W, int H, const char* title ) :
Fl_Double_Window( W, H, title )
{
    // Set icon does not work on Linux yet
    set_icon();
}

MainWindow::~MainWindow()
{
    uiMain->uiView->stop();
    delete uiMain->uiView;
    uiMain->uiView = NULL;
}


void MainWindow::set_icon()
{
    fl_open_display();  // Needed for icons

    
// #if defined(_WIN32) || defined(_WIN64)
//     HICON data = LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1));
//     this->icon(data);
// #else
    Fl_Pixmap* pic = new Fl_Pixmap( viewer16_xpm );
    Fl_RGB_Image* rgb = new Fl_RGB_Image( pic );
    icon( rgb );
// #endif

}


void MainWindow::always_on_top()
{
#if defined(_WIN32) || defined(_WIN64)
    // Microsoft (R) Windows(TM)
    SetWindowPos(fl_xid(this), HWND_TOPMOST,
                 0, 0, w()+8, h()+27, 0);
#else
    // XOrg / XWindows(TM)
    XEvent ev;
    static const char* const names[2] = { "_NET_WM_STATE",
                                          "_NET_WM_STATE_ABOVE"
                                        };
    Atom atoms[ 2 ];
    fl_open_display();
    XInternAtoms(fl_display, (char**)names, 2, False, atoms );
    Atom net_wm_state = atoms[ 0 ];
    Atom net_wm_state_above = atoms[ 1 ];
    ev.type = ClientMessage;
    ev.xclient.window = fl_xid(this);
    ev.xclient.message_type = net_wm_state;
    ev.xclient.format = 32;
    ev.xclient.data.l[ 0 ] = active() ? 1 : 0;
    ev.xclient.data.l[ 1 ] = net_wm_state_above;
    ev.xclient.data.l[ 2 ] = 0;
    XSendEvent(fl_display,
               DefaultRootWindow(fl_display),  False,
               SubstructureNotifyMask|SubstructureRedirectMask, &ev);
#endif
} // above_all function


/**
 * Iconize all windows
 *
 */
void MainWindow::iconize_all()
{
    Fl_Window* uiReelWindow = uiMain->uiReelWindow->uiMain;
    if (uiReelWindow) uiReelWindow->iconize();
    return Fl_Window::iconize();
}

/**
 * Handle MainWindow's Fl::events
 *
 * @param event Fl::event enumeration
 *
 * @return 1 if handled, 0 if not.
 */
int MainWindow::handle( int event )
{
    return Fl_Window::handle( event );
}

void MainWindow::layout()
{
    redraw(); // @TODO: FLTK1.4
}

} // namespace mrv
