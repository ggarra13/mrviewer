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

#include "core/mrvImageOpts.h"
#include "EXROptionsUI.h"
#include "WandOptionsUI.h"

namespace mrv {


ImageOpts* ImageOpts::build( std::string ext, const bool has_deep_data )
{
    if ( ext == ".exr" || ext == ".sxr" || ext == ".mxr" )
        return new EXROptionsUI( CMedia::aces_metadata(), 
                                 CMedia::all_layers(),
                                 has_deep_data );
    return new WandOptionsUI( CMedia::aces_metadata(),
                              CMedia::all_layers() );
}

}
