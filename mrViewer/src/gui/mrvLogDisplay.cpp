/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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
 * @file   mrvLogDisplay.cpp
 * @author gga
 * @date   Tue Oct  9 20:56:16 2007
 *
 * @brief
 *
 *
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <stdexcept>
#include <iostream>

#include <boost/thread/thread.hpp>

#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl.H>
#include <FL/fl_utf8.h>

#include "core/mrvThread.h"
#include "core/mrvHome.h"
#include "gui/mrvIO.h"
#include "gui/mrvLogDisplay.h"
#include "mrViewer.h"

namespace mrv {

// Style table
static
Fl_Text_Display::Style_Table_Entry kLogStyles[] = {
    // FONT COLOR      FONT FACE   SIZE   ATTR
    // --------------- ----------- ----- ------
    {  FL_BLACK,  FL_HELVETICA, 14,   0 }, // A - Info
    {  FL_DARK_YELLOW, FL_HELVETICA, 14,   0 }, // B - Warning
    {  FL_RED,    FL_HELVETICA, 14,   0 }, // C - Error
};

LogDisplay::ShowPreferences LogDisplay::prefs = LogDisplay::kNever;
std::atomic<bool> LogDisplay::shown( false );
std::atomic<bool> LogDisplay::show( false );

    std::string conn_string;
    std::string log_string;
    std::string style_string;

void static_log(void*)
{
    //
    // First, handle log window showing up and scrolling
    //
    LogUI* logUI = ViewerUI::uiLog;
    Fl_Window* logwindow = logUI->uiMain;
    if ( !logwindow ) return;

    mrv::LogDisplay* log = logUI->uiLogText;

    boost::mutex::scoped_lock lock( io::logbuffer::mutex );

    if ( !log_string.empty() )
    {
        log->style_buffer()->append( style_string.c_str() );
        log->buffer()->append( log_string.c_str() );

        log_string.clear();
        style_string.clear();

        static unsigned  lines = 0;
        if ( log->visible() && log->lines() != lines )
        {
            log->scroll( log->lines()-1, 0 );
            lines = log->lines();
        }
    }

    if ( mrv::LogDisplay::show == true )
    {
        mrv::LogDisplay::show = false;
        if ( logUI && logwindow )
        {
            logwindow->show();
        }
    }

}

LogDisplay::LogDisplay( int x, int y, int w, int h, const char* l  ) :
Fl_Text_Display( x, y, w, h, l ),
_lines( 0 )
{

    color( FL_GRAY0 );

    scrollbar_align( FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT );

    wrap_mode( WRAP_AT_BOUNDS, 80 );

    delete mBuffer;
    delete mStyleBuffer;
    mBuffer = new Fl_Text_Buffer();
    mStyleBuffer = new Fl_Text_Buffer();
    highlight_data(mStyleBuffer, kLogStyles, 3, 'A', 0, 0);

}

LogDisplay::~LogDisplay()
{
    delete mBuffer; mBuffer = NULL;
    delete mStyleBuffer; mStyleBuffer = NULL;
}

void LogDisplay::clear()
{
    mStyleBuffer->text("");
    mBuffer->text("");
    _lines = 0;
}

void LogDisplay::save( const char* file )
{
    char buf[4096];
    if ( !file )
    {
        std::string home = mrv::homepath();
        sprintf( buf, "%s/mrViewer.log", home.c_str() );
        file = buf;
    }

    try {

        int err = mBuffer->savefile( file );

        if ( err != 0 ) throw std::runtime_error( strerror(err) );


        info( "Saved log as \"" );
        info( file );
        info( "\"." );
    }
    catch( const std::runtime_error& e )
    {
        char err[4200];
        sprintf( err, "Could not save log file \"%s\".", file );
        error( err );
        error( e.what() );
    }
    catch( ... )
    {
        char err[4200];
        sprintf( err, "Could not save log file \"%s\".", file );
        error( err );
    }

}


void LogDisplay::info( const char* x )
{
    size_t t = strlen(x);
    char* buf = (char*)malloc( t+1 );
    buf[t] = 0;
    while( t-- )
    {
        if ( x[t] == '\n' ) {
            ++_lines;
            buf[t] = '\n';
        }
        else
        {
            buf[t] = 'A';
        }
    }


    style_string += buf;
    log_string   += x;

    free( buf );

    Fl::awake( static_log, NULL );

}

void LogDisplay::warning( const char* x )
{
    size_t t = strlen(x);
    char* buf = (char*)malloc( t+1 );
    buf[t] = 0;
    while( t-- )
    {
        if ( x[t] == '\n' ) {
            ++_lines;
            buf[t] = '\n';
        }
        else
        {
            buf[t] = 'B';
        }
    }

    style_string += buf;
    log_string   += x;

    free( buf );

    Fl::awake( static_log, NULL );
}

void LogDisplay::error( const char* x )
{
    size_t t = strlen(x);
    char* buf = (char*)malloc( t+1 );
    buf[t] = 0;
    while( t-- )
    {
        if ( x[t] == '\n' ) {
            ++_lines;
            buf[t] = '\n';
        }
        else
        {
            buf[t] = 'C';
        }
    }

    style_string += buf;
    log_string   += x;

    free( buf );

    if ( prefs == kAlways || (prefs == kOnce && !shown) )
    {
        shown = true;
        show = true;
    }

    Fl::awake( static_log, NULL );
}

}
