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

#include <algorithm>

#include "core/mrvImageOpts.h"
// #include "OIIOOptionsUI.h"  // @TODO: fltk1.4
#include "EXROptionsUI.h"
#include "WandOptionsUI.h"

namespace mrv {


ImageOpts* ImageOpts::build( ViewerUI* main, std::string ext,
                             const bool has_deep_data )
{
    std::transform( ext.begin(), ext.end(), ext.begin(), (int(*)(int)) tolower);

    if ( ext == ".exr" || ext == ".sxr" || ext == ".mxr" )
        return new EXROptionsUI( main,
                                 CMedia::aces_metadata(),
                                 CMedia::all_layers(),
                                 has_deep_data );


    // @TODO: for fltk1.4
    //if ( ext == ".tx" || ext == ".iff" || ext == ".hdr" )
    //    return new OIIOOptionsUI( main, ext, CMedia::all_layers() );

    return new WandOptionsUI( main,
                              CMedia::aces_metadata(),
                              CMedia::all_layers() );
}

}
