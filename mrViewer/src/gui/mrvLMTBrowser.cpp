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
 * @file   mrvLMTBrowser.cpp
 * @author gga
 * @date   Mon Jul  2 08:11:24 2007
 *
 * @brief
 *
 *
 */

#include <mrvIO.h>
#include <mrvLMTBrowser.h>

namespace
{
const char* kModule = "lmt";
}


namespace mrv {

LMTBrowser::LMTBrowser(int x, int y, int w, int h, const char* l) :
    Fl_Browser( x, y, w, h, l )
{
}

LMTBrowser::~LMTBrowser()
{
}


void LMTBrowser::fill( const mrv::media& fg )
{
    this->clear();

    _fg = fg;
    if ( !_fg ) return;

    CMedia* img = _fg->image();

    size_t num = img->number_of_lmts();

    for ( size_t i = 0; i < num; ++i )
    {
        const char* lmt = img->look_mod_transform(i);
        this->add( lmt );
    }

}

int LMTBrowser::handle( int e )
{
    return Fl_Browser::handle( e );
}


} // namespace mrv
