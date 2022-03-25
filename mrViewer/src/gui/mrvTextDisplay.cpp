/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
 * @file   mrvTextDisplay.cpp
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

#include <iostream>

#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl.H>
#include <FL/fl_utf8.h>

#include "gui/mrvTextDisplay.h"

namespace mrv {


TextDisplay::TextDisplay( int x, int y, int w, int h, const char* l  ) :
Fl_Text_Display( x, y, w, h, l )
{
    color( FL_GRAY0 );
    buffer( new Fl_Text_Buffer() );
}

TextDisplay::~TextDisplay()
{
    Fl_Text_Buffer* b = buffer();
    buffer(NULL);
    delete b;
}


}
