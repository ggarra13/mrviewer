/**
 * @file   mrvColorSpaces.h
 * @author gga
 * @date   Tue Feb 19 19:53:07 2008
 * 
 * @brief  Color space transforms
 * 
 * 
 */

#include <ImfChromaticities.h>
#include "mrvImagePixel.h"

namespace mrv {

  namespace color {

    extern Imath::V3f     kD50_whitePoint;
    extern Imath::V3f     kD65_whitePoint;
    extern Imf::Chromaticities kITU_702_chroma;

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
      kITU_702, // Digital HDTV
//       kYPbPr = 10,
      kLastColorSpace
    };


    const char* space2name( const Space& space );
    const char* space2id( const Space& space );
    const char* space2channels( const Space& space );


    namespace rgb 
    {
      ImagePixel to_xyz( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_702_chroma, 
			 const float Y = 1.0f);
      ImagePixel to_xyY( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_702_chroma, 
			 const float Y = 1.0f);
      ImagePixel to_lab( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_702_chroma, 
			 const float Y = 1.0f );
      ImagePixel to_luv( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma = kITU_702_chroma, 
			 const float Y = 1.0f );
      ImagePixel to_hsv( const ImagePixel& rgb );
      ImagePixel to_hsl( const ImagePixel& rgb );
      ImagePixel to_yuv( const ImagePixel& rgb );
      ImagePixel to_yiq( const ImagePixel& rgb );
      ImagePixel to_YDbDr( const ImagePixel& rgb );
      ImagePixel to_ITU601( const ImagePixel& rgb );
      ImagePixel to_ITU702( const ImagePixel& rgb );
    }  // namespace rgb


  namespace yuv
  {
  ImagePixel to_rgb( const ImagePixel& rgb );
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
