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
 * @file   mrvColorSpaces.h
 * @author gga
 * @date   Tue Feb 19 19:53:07 2008
 * 
 * @brief  Color space transforms
 * 
 * 
 */

#include <ImfForward.h>
#include <ImfChromaticities.h>
#include "mrvImagePixel.h"

namespace mrv {

  namespace color {

    extern Imath::V3f     kD50_whitePoint;
    extern Imath::V3f     kD65_whitePoint;
    extern Imf::Chromaticities kITU_709_chroma;

    enum Space {
      kRGB,
      kHSV,
      kHSL,
      kCIE_XYZ,
      kCIE_xyY,
      kCIE_Lab,
      kCIE_Luv,
      kYUV,     // Analog  PAL
      kYDbDr,   // Analog  SECAM/PAL-N
      kYIQ,     // Analog  NTSC
      kITU_601, // Digital PAL/NTSC
      kITU_709, // Digital HDTV
//       kYPbPr = 10,
      kLastColorSpace
    };


    const char* space2name( const Space& space );
    const char* space2id( const Space& space );
    const char* space2channels( const Space& space );


    namespace rgb 
    {
      ImagePixel to_xyz( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_709_chroma, 
			 const float Y = 1.0f);
      ImagePixel to_xyY( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_709_chroma, 
			 const float Y = 1.0f);
      ImagePixel to_lab( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_709_chroma, 
			 const float Y = 1.0f );
      ImagePixel to_luv( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_709_chroma, 
			 const float Y = 1.0f );
      ImagePixel to_hsv( const ImagePixel& rgb );
      ImagePixel to_hsl( const ImagePixel& rgb );
      ImagePixel to_yuv( const ImagePixel& rgb );
      ImagePixel to_yiq( const ImagePixel& rgb );
      ImagePixel to_YDbDr( const ImagePixel& rgb );
      ImagePixel to_ITU601( const ImagePixel& rgb );
      ImagePixel to_ITU709( const ImagePixel& rgb );
    }  // namespace rgb


  namespace yuv
  {
  ImagePixel to_rgb( const ImagePixel& yuv256 );
  }

    namespace xyz 
    {
    }

    namespace lab 
    {
    }

    namespace luv 
    {
    }

  }

} // namespace mrv
