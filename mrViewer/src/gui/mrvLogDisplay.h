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
 * @file   mrvLogDisplay.h
 * @author gga
 * @date   Tue Oct  9 20:42:05 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef mrvLogDisplay_h
#define mrvLogDisplay_h

#include <fltk/TextDisplay.h>

namespace mrv {

class LogDisplay : public fltk::TextDisplay
{
  public:
    enum ShowPreferences
    {
    kNever,
    kOnce,
    kAlways,
    };
  public:
    LogDisplay( int x, int y, int w, int h, const char* l = 0 );
    ~LogDisplay();

    void clear();
    void save( const char* file = NULL );

    void info( const char* x );
    void warning( const char* x );
    void error( const char* x );

    inline unsigned lines() const { return _lines; }
    
  protected:
    unsigned int _lines;

  public:
    static ShowPreferences prefs;
    static bool shown;
    static bool show;  // whether to show this in fltk thread
};

}

#endif
