/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuo

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
 * @file   mrvTextDisplay.h
 * @author gga
 * @date   Tue Oct  9 20:42:05 2007
 *
 * @brief
 *
 *
 */

#ifndef mrvTextDisplay_h
#define mrvTextDisplay_h


#include <FL/Fl_Text_Display.H>

namespace mrv {

class TextDisplay : public Fl_Text_Display
{
public:
    TextDisplay( int x, int y, int w, int h, const char* l = 0 );
    ~TextDisplay();

public:
    Fl_Text_Buffer* buffer_;
};

}

#endif
