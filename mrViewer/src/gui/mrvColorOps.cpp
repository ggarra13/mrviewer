/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
 * @file   mrvColorOps.cpp
 * @author gga
 * @date   Fri Jan 18 09:54:42 2008
 *
 * @brief
 *
 *
 */

#include "FL/fl_draw.H"
#include "gui/mrvColorOps.h"

namespace mrv {


Fl_Color darker( Fl_Color c, uchar v )
{
    uchar r,g,b;
    Fl::get_color( c, r, g, b );
    if (r > v ) r -= v;
    else r = 0;
    if (g > v ) g -= v;
    else g = 0;
    if (b > v ) b -= v;
    else b = 0;
    return fl_rgb_color( r, g, b );
}

Fl_Color lighter( Fl_Color c, uchar v )
{
    uchar r,g,b;
    Fl::get_color( c, r, g, b );

    if ( 0xff - r > v ) r += v;
    else r = 0xff;
    if ( 0xff - g > v ) g += v;
    else g = 0xff;
    if ( 0xff - b > v ) b += v;
    else b = 0xff;

    return fl_rgb_color( r, g, b );
}

}
