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
 * @file   clonedImage.h
 * @author gga
 * @date   Fri Sep 21 01:16:30 2007
 * 
 * @brief  Image class used to clone other images
 * 
 * 
 */

#ifndef clonedImage_h
#define clonedImage_h

#include "CMedia.h"

namespace mrv {

  class clonedImage : public CMedia
  {
  public:
    clonedImage( const CMedia* other );

    virtual const char* const format() const { return "Cloned Image"; }

    virtual bool is_sequence() const { return false; }

    virtual bool has_changed() { return false; }

    ////////////////// Set the frame for the current image (sequence)
    virtual bool            frame( const boost::int64_t f ) { return true; };
    virtual boost::int64_t  frame() const { return _frame; }

  };

}

#endif // clonedImage_h

