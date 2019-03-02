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

#include "core/mrvI8N.h"
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
//#ifdef _WIN32
    // Set icon does not work on Linux yet
    set_icon();
//#endif
}

MainWindow::~MainWindow()
{
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
    // unsigned long buffer[] = {
    //         16, 16,
    //         4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 338034905, 3657433343, 0, 184483840, 234881279, 3053453567, 3221225727, 1879048447, 0, 0, 0, 0, 0, 0, 0, 1224737023, 3305111807, 3875537151,0, 0, 2063597823, 1291845887, 0, 67109119, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 50266112, 3422552319, 0, 0, 3070230783, 2063597823, 2986344703, 771752191, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3422552319, 0, 0, 3372220671, 1509949695, 704643327, 3355443455, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 0, 3422552319, 0, 134152192, 3187671295, 251658495, 0, 3439329535, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3422552319, 0, 0, 2332033279, 1342177535, 167772415, 3338666239, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 4294901760, 0, 3422552319, 0, 0, 436207871, 3322085628, 3456106751, 1375731967, 4278255360, 4026597120, 3758161664, 3489726208, 3204513536, 2952855296, 2684419840, 2399207168, 2130771712, 1845559040, 1593900800, 1308688128, 1040252672, 755040000, 486604544, 234946304, 4278255360, 4043374336, 3774938880, 3506503424, 3221290752, 2952855296, 2667642624, 2399207168, 2130771712, 1862336256, 1627453957, 1359017481, 1073805064, 788591627, 503379721, 218169088, 4278255360, 4043374336, 3758161664, 3506503424, 3221290752, 2952855296, 2684419840, 2415984384, 2130771712, 1862336256, 1577123584, 1308688128, 1040252672, 755040000, 486604544, 218169088, 4278190335, 4026532095, 3758096639, 3489661183, 3221225727, 2952790271, 2667577599, 2415919359, 2130706687, 1862271231, 1593835775, 1325400319, 1056964863, 771752191, 520093951, 234881279, 4278190335, 4026532095, 3758096639, 3489661183, 3221225727, 2952790271, 2667577599, 2415919359, 2130706687, 1862271231, 1593835775, 1325400319, 1056964863, 771752191, 503316735, 234881279, 4278190335, 4026532095, 3758096639, 3489661183, 3221225727, 2952790271, 2684354815, 2399142143, 2130706687, 1862271231, 1593835775, 1325400319, 1040187647, 771752191, 520093951, 234881279, 4294901760, 4043243520, 3774808064, 3506372608, 3221159936, 2952724480, 2684289024, 2399076352, 2147418112, 1862205440, 1593769984, 1308557312, 1040121856, 771686400, 503250944, 234815488, 4294901760, 4060020736, 3758030848, 3506372608, 3221159936, 2952724480, 2684289024, 2415853568, 2130640896, 1862205440, 1593769984, 1308557312, 1040121856, 771686400, 503250944, 234815488, 4294901760, 4043243520, 3774808064, 3489595392, 3237937152, 2952724480, 2684289024, 2415853568, 2147418112, 1862205440, 1593769984, 1325334528, 1056899072, 788463616, 503250944, 234815488,
    // };
    // this->icon( buffer );
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
                                          "_NET_WM_STATE_ABOVE"
                                        };
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
