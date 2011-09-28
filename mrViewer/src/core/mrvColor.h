/**
 * @file   mrvColor.h
 * @author 
 * @date   Fri Oct 13 10:11:05 2006
 * 
 * @brief  Some fltk routines to deal with fltk::colors more easily.
 * 
 * 
 */

#ifndef mrvColor_h
#define mrvColor_h


#include "mrvImagePixel.h"

namespace mrv
{

  enum BrightnessType {
    kAsLuminance,
    kAsLumma,
    kAsLightness,
  };

  float calculate_brightness( const mrv::ImagePixel& rgba,
			      const mrv::BrightnessType type );

}


#endif // mrvColor_h

