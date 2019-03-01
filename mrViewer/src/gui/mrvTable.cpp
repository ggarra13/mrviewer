/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

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
 * @file   mrvTable.h
 * @author gga
 * @date   Wed Jan 31 14:26:28 2007
 *
 * @brief  A table that can contain widgets.
 *
 *
 */

#include <gui/mrvTable.h>

namespace mrv {

Table::Table( int x, int y, int w, int h, const char* l ) :
Fl_Table( x, y, w, h, l )
{
}

Table::~Table()
{
}

void Table::draw_cell(TableContext context, int R, int C, 
		      int X, int Y, int W, int H)
{
    // @TODO: fltk1.4
}
    
} // namespace mrv


