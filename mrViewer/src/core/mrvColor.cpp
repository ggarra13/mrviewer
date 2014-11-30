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
 * @file   mrvColor.cpp
 * @author gga
 * @date   Mon Aug 27 23:28:07 2007
 * 
 * @brief  
 * 
 * 
 */

#include <cmath>  // for pow

#include "mrvColor.h"

namespace mrv
{


  float calculate_brightness( const mrv::ImagePixel& rgba,
			      const mrv::BrightnessType type )
  {
    float L;
    switch( type )
      {
      case kAsLuminance:
	 L = (0.2126f * rgba.r + 0.7152f * rgba.g + 0.0722f * rgba.b);
	break;
      case kAsLightness:
	 L = (0.2126f * rgba.r + 0.7152f * rgba.g + 0.0722f * rgba.b);
	 if ( L >= 0.008856f )
	    L = 116 * (pow( L, 1.0f/3.0f)) - 16;
      
	// normalize it to 0...1, instead of 0...100
	L = L / 100.f;
	break;
      case kAsLumma: 
      default:
	L = (rgba.r + rgba.g + rgba.b) / 3.0f;
	break;
      }
    return L;
  }


} // namespace mrv

