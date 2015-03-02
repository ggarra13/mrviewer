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
 * @file   mrvColorSpaces.cpp
 * @author gga
 * @date   Tue Feb 19 20:00:20 2008
 * 
 * @brief  
 * 
 * 
 */

#include <cassert>
#include <algorithm>

#include "mrvColorSpaces.h"


namespace {

  struct ColorSpaceInfo {
    const char* id;
    const char* name;
    const char* channels;
  };



  const ColorSpaceInfo kSpaceInfo[] = 
    {
      // ID       Name                  Channels
      { "RGB",	"Linear RGB",	   	"R G B" },
      { "HSV",	"Normalized HSV",	"H S V" },
      { "HSL",	"Normalized HSL",	"H S L" },
      { "XYZ",	"CIE XYZ",	   	"X Y Z" },
      { "xyY",	"CIE xyY",	   	"x y Y" },
      { "Lab",	"CIE L*a*b*",	   	"L a b" },
      { "Luv",  "CIE L*u*v*",	   	"L u v" },
      { "YUV",	"Analog PAL",	   	"Y U V" },
      { "YDD",	"Analog SECAM/PAL-N",	"Y Db Dr" },
      { "YIQ",	"Analog NTSC",	   	"Y I Q"   },
      { "601",	"Digital PAL/NTSC",	"Y Cb Cr" },
      { "709",	"Digital HDTV",	   	"Y Cb Cr" },
    };


  inline float cubeth(float v)
  {
    if (v > 0.008856f) {
      return (float)pow((double)v, 1.0/3.0);
    }
    else {
      return (float)(7.787037037037037037037037037037*v + 16.0/116.0);
    }
  }

  inline float icubeth(float v)
  {
    if (v > 0.20689303448275862068965517241379f)
      return v*v*v;
    else 
      if (v > 16.0f/116.0f)
	return (float)((v - 16.0f / 116.0f) / 7.787037037037037037037037037037f);
      else
	return 0.0f;
  }

  /** 
   * Calculate hue for HSV or HSL
   * 
   * @param rgb         original RGB pixel values
   * @param maxV        The max value of r, g, and b in rgb pixel
   * @param spanV       The (max-min) value between r,g,b channels in rgb pixel
   * 
   * @return a float representing the pixel hue [0..1]
   */
  inline
  float hue( const mrv::ImagePixel& rgb, const float maxV, const float spanV )
  {
    float h;
    if ( rgb.r == maxV ) {
      h = (rgb.g - rgb.b) / spanV;
    }
    else if ( rgb.g == maxV ) {
      h = 2.0f + (rgb.b - rgb.r) / spanV;
    }
    else {  // ( rgb.b == maxV ) 
      h = 4.0f + (rgb.r - rgb.g) / spanV;
    }
    if ( h < 0.0f ) {
      h += 6;
    }
    h /= 6;
    return h;
  }

}

namespace mrv {


  namespace color {

    Imf::Chromaticities kITU_709_chroma;
    Imath::V3f     kD50_whitePoint(0.3457f, 0.3585f, 0.2958f);
    Imath::V3f     kD65_whitePoint(0.3127f, 0.3290f, 0.3582f);


    const char* space2name( const Space& space )
    {
      assert( space >= 0 && space <= kLastColorSpace );
      return kSpaceInfo[ (unsigned)space ].name;
    }

    const char* space2id( const Space& space )
    {
      assert( space >= 0 && space <= kLastColorSpace );
      return kSpaceInfo[ (unsigned)space ].id;
    }

    const char* space2channels( const Space& space )
    {
      assert( space >= 0 && space <= kLastColorSpace );
      return kSpaceInfo[ (unsigned)space ].channels;
    }

    namespace rgb {

      ImagePixel to_xyz( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma, const float Y )
      {
	ImagePixel r( rgb );
	const Imath::M44f& m = RGBtoXYZ( chroma, Y );
	Imath::V3f* v = (Imath::V3f*)&r;
	*v = *v * m;
	return r;
      }

      ImagePixel to_xyY( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma, const float Y)
      {
	ImagePixel xyy( to_xyz( rgb, chroma, Y ) );

	float sum = xyy.r + xyy.g + xyy.b;

	xyy.b = xyy.g;

	if ( sum == 0.0f )
	  {
	    xyy.r = chroma.white.x;
	    xyy.g = chroma.white.y;
	  }
	else
	  {
	    sum   = 1.0f / sum;
	    xyy.r *= sum;
	    xyy.g *= sum;
	  }

	return xyy;
      }

      ImagePixel to_lab( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma, const float Y )
      {
	ImagePixel lab( to_xyz( rgb, chroma, Y ) );

	float Xn = cubeth( lab.r / chroma.white.x );
	float Yn = cubeth( lab.g / chroma.white.y );
	float Zn = cubeth( lab.b / (1.0f - chroma.white.x - chroma.white.y) );

	lab.r = (116.0f * Yn - 16.0f);
	lab.g = (500.0f * (Xn - Yn));
	lab.b = (200.0f * (Yn - Zn));
	return lab;
      }

      ImagePixel to_luv( const ImagePixel& rgb,
			 const Imf::Chromaticities& chroma, const float Y )
      {
	ImagePixel luv( to_xyz( rgb, chroma, Y ) );

	float cwz = (1.0f - chroma.white.x - chroma.white.y);

	float yr = luv.g / chroma.white.y;

	float D  = 1.0f / (luv.r + 15.0f * luv.g + 3 * luv.b);
	float Dn = 1.0f / (chroma.white.x + 15.0f * chroma.white.y + 3 * cwz);

	float u1 = (4.0f * luv.r) * D;
	float v1 = (9.0f * luv.g) * D;

	float ur = (4.0f * chroma.white.x) * Dn;
	float vr = (4.0f * chroma.white.y) * Dn;

	if ( yr > 0.008856f )
	  {
	     float Yn = powf( yr, 1.0f/3.0f);
	     luv.r = 116.0f * Yn - 16.0f; 
	  }
	else
	  {
	    luv.r = 903.3f * yr;
	  }
	luv.g = 13.0f * luv.r * (u1 - ur );
	luv.b = 13.0f * luv.r * (v1 - vr );
	return luv;
      }

      ImagePixel to_hsv( const ImagePixel& rgb )
      {
	float minV = std::min( rgb.r, std::min( rgb.g, rgb.b ) ); 
	float maxV = std::max( rgb.r, std::max( rgb.g, rgb.b ) );
	float h,s,v;
        float spanV = maxV - minV;
	v = maxV;
	s = (maxV != 0.0f) ? (spanV/maxV) : 0.0f;
	if ( s == 0 ) h = 0;
	else {
	  h = hue( rgb, maxV, spanV );
	}
	return ImagePixel( h, s, v, rgb.a );
      }

      ImagePixel to_hsl( const ImagePixel& rgb )
      {
	float minV  = std::min( rgb.r, std::min( rgb.g, rgb.b ) ); 
	float maxV  = std::max( rgb.r, std::max( rgb.g, rgb.b ) );
	float spanV = maxV - minV;
	float sumV  = maxV + minV;
	float h = hue( rgb, maxV, spanV );
	float l = sumV * 0.5f;
	float s;
	if ( maxV == minV ) s = 0.0f;
	else if ( l <= 0.5f )
	  {
	    s = spanV / sumV; // or:  spanV / (2*l)
	  }
	else
	  {
	    s = spanV / (2.0f - sumV); // or: spanV / (2-(2*l)) 
	  }
	return ImagePixel( h, s, l, rgb.a );
      }

      // Analog NTSC
      ImagePixel to_yiq( const ImagePixel& rgb )
      {
	return ImagePixel(
			   rgb.r * 0.299f    + rgb.g * 0.587f    + rgb.b * 0.114f,
			  -rgb.r * 0.595716f - rgb.g * 0.274453f - rgb.b * 0.321263f,
			   rgb.r * 0.211456f - rgb.g * 0.522591f + rgb.b * 0.31135f
			  );
      }

      // Analog PAL
      ImagePixel to_yuv( const ImagePixel& rgb )
      {
	return ImagePixel(
			   rgb.r * 0.299f   + rgb.g * 0.587f   + rgb.b * 0.114f,
			  -rgb.r * 0.14713f - rgb.g * 0.28886f + rgb.b * 0.436f,
			   rgb.r * 0.615f   - rgb.g * 0.51499f - rgb.b * 0.10001f
			  );
      }

      // Analog Secam/PAL-N
      ImagePixel to_YDbDr( const ImagePixel& rgb )
      {
	return ImagePixel(
			   rgb.r * 0.299f + rgb.g * 0.587f + rgb.b * 0.114f,
			  -rgb.r * 0.450f - rgb.g * 0.883f + rgb.b * 1.333f,
			  -rgb.r * 1.333f + rgb.g * 1.116f + rgb.b * 0.217f
			  );
      }


      // ITU. 601 or CCIR 601  (Digital PAL and NTSC )
      ImagePixel to_ITU601( const ImagePixel& rgb )
      {
	return ImagePixel(
			  16.f  + rgb.r * 65.481f + rgb.g * 128.553f + rgb.b * 24.966f,
			  128.f - rgb.r * 37.797f - rgb.g * 74.203f  + rgb.b * 112.0f,
			  128.f + rgb.r * 112.0f  - rgb.g * 93.786f  - rgb.b * 18.214f
			  );
      }

      // ITU. 709  (Digital HDTV )
      ImagePixel to_ITU709( const ImagePixel& rgb )
      {
	return ImagePixel(
			   rgb.r * 0.299f + rgb.g * 0.587f + rgb.b * 0.114f,
			  -rgb.r * 0.299f - rgb.g * 0.587f + rgb.b * 0.886f,
			   rgb.r * 0.701f - rgb.g * 0.587f - rgb.b * 0.114f
			  );
      }

    } // namespace rgb

  namespace yuv {

      // Analog PAL
      ImagePixel to_rgb( const ImagePixel& yuv )
      {
	ImagePixel   rgb(
			  1.164f * (yuv.r - 16) + 2.018f * (yuv.g - 128),
			  1.164f * (yuv.r - 16) - 0.813f * (yuv.b - 128) 
			  - 0.391f * (yuv.g - 128),
			  1.164f * (yuv.r - 16) + 1.596f * (yuv.b - 128) 
			 );
	return rgb;
      }
  }

  } // namespace color

} // namespace mrv
