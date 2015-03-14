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
 * @file   mrvFrame.cpp
 * @author gga
 * @date   Sun Jan 13 13:51:37 2008
 * 
 * @brief  
 * 
 * 
 */

#include <cassert>

#include <iostream>
#include <limits>    // for quietNaN

#include <half.h>

#include "core/mrvFrame.h"

namespace 
{

  unsigned int
  scaleInt(float f, unsigned int i)
  {
    return unsigned(f * i + 0.5);
  }

  // @todo: do not hard-code luminance weights
  float yw[3] = { 0.2126f, 0.7152f, 0.0722f };
}

#include "core/mrvAlignedData.h"
#include "core/mrvFrame_u8.inl"
#include "core/mrvFrame_u16.inl"
#include "core/mrvFrame_u32.inl"
#include "core/mrvFrame_h16.inl"
#include "core/mrvFrame_f32.inl"


namespace mrv {

const char* const VideoFrame::fmts[] = {
"Lumma",
"LummaA",
"BGR",
"BGRA",
"RGB",
"RGBA",

"ITU_601_YCbCr420",
"ITU_601_YCbCr420A",
"ITU_601_YCbCr422",
"ITU_601_YCbCr422A",
"ITU_601_YCbCr444",
"ITU_601_YCbCr444A",

"ITU_709_YCbCr420",
"ITU_709_YCbCr420A", // @todo: not done
"ITU_709_YCbCr422",
"ITU_709_YCbCr422A", // @todo: not done
"ITU_709_YCbCr444",
"ITU_709_YCbCr444A", // @todo: not done

"YByRy420",
"YByRy420A", // @todo: not done
"YByRy422",
"YByRy422A", // @todo: not done
"YByRy444",
"YByRy444A", // @todo: not done
};

/** 
 * Return the size of a pixel in memory.
 *
 * @return size of pixel in memory
 */
  unsigned short VideoFrame::pixel_size()
  {
    switch( _type )
      {
      case kByte:
	return sizeof(char);
      case kShort:
	return sizeof(short);
      case kInt:
	return sizeof(int);
      case kHalf:
	return sizeof(half);
      case kFloat:
	return sizeof(float);
      default:
	throw std::runtime_error("Unknown mrv::Frame pixel type");
      }
  }


  size_t VideoFrame::data_size()
  {
    size_t size = 0;

    unsigned W2, H2, WH2;
    unsigned WH = _width * _height;

    switch( _format )
      {
      case kLumma:
	size = WH * _channels; break;

      case kITU_709_YCbCr420A:
      case kITU_601_YCbCr420A:
      case kYByRy420A:
	size = WH; // alpha

      case kYByRy420:
      case kITU_709_YCbCr420:
      case kITU_601_YCbCr420:
	size += WH;   // Y
	W2 = (_width  + 1) / 2;
	H2 = (_height + 1) / 2;
	WH2 = W2 * H2;
	size += 2 * WH2;  // U and V
	break;

      case kITU_709_YCbCr422:
      case kITU_601_YCbCr422:
	size += WH;   // Y
	W2 = (_width  + 1) / 2;
	WH2 = W2 * _height;
	size += 2 * WH2;  // U and V
	break;

      default:
	size = WH * _channels;
	break;
      }

    // multiply size by that of pixel type.  We assume all
    // channels use same pixel type.
    return size * pixel_size();
  }

/** 
 * Allocate a frame aligned to 16 bytes in memory.
 * 
 */
  void VideoFrame::allocate()
  {
    mrv::aligned16_uint8_t* ptr = new mrv::aligned16_uint8_t[ data_size() ];
    assert( ((unsigned long)ptr) % 16 == 0 );

    _data.reset( ptr );
  }

/** 
 * Return pixel value of a certain coordinate
 * 
 * @param x valid x coordinate of video frame
 * @param y valid y coordinate of video frame
 * 
 * @return Image pixel values
 */
  ImagePixel VideoFrame::pixel( const unsigned int x,
                                const unsigned int y ) const
  {
     if ( !_data ) {
      return ImagePixel(std::numeric_limits<float>::quiet_NaN(),
			std::numeric_limits<float>::quiet_NaN(),
			std::numeric_limits<float>::quiet_NaN(),
			std::numeric_limits<float>::quiet_NaN());
     }

    assert( x < _width  );
    assert( y < _height );


    switch( _type )
      {
      case kByte:
	return pixel_u8( x, y );
      case kShort:
	return pixel_u16( x, y );
      case kInt:
	return pixel_u32( x, y );
      case kHalf:
	return pixel_h16( x, y );
      case kFloat:
	return pixel_f32( x, y );
      default:
	throw std::runtime_error("Unknown mrv::Frame pixel type");
      }
  }

/** 
 * Set the value of a pixel
 * 
 * @param x valid x coordinate of video frame
 * @param y valid y coordinate of video frame
 * @param p pixel values to set x,y with
 */
  void VideoFrame::pixel( const unsigned int x, const unsigned int y,
			  const ImagePixel& p )
  {
    if ( !_data ) 
      throw std::runtime_error("mrv::Frame No pixel data to change");
      
    assert( x < _width  );
    assert( y < _height );

    switch( _type )
      {
      case kByte:
	return pixel_u8( x, y, p );
      case kShort:
	return pixel_u16( x, y, p );
      case kInt:
	return pixel_u32( x, y, p );
      case kHalf:
	return pixel_h16( x, y, p );
      case kFloat:
	return pixel_f32( x, y, p );
      default:
	throw std::runtime_error("Unknown mrv::Frame pixel type");
      }
  }

/** 
 * Scale video frame in X
 * 
 * @param f scale factor
 * 
 * @return scaled video frame
 */
  VideoFrame* VideoFrame::scaleX(float f) const
  {
    unsigned int dw1 = scaleInt( f, _width );
    if ( dw1 < 1 ) dw1 = 1;

    //
    // Create a scaled frame
    //
    VideoFrame* scaled = new VideoFrame( _frame, 
					 dw1,
					 _height,
					 _channels,
					 _format,
					 _type,
					 _repeat );


    if ( dw1 > 1 )
      f = float (_width - 1) / float (dw1 - 1);
    else
      f = 1;

    for (unsigned int x = 0; x < dw1; ++x)
      {
          float x1 = float(x) * f;
          unsigned int xs = static_cast< unsigned int>( x1 );
          unsigned int xt = std::min( xs + 1, _width - 1 );

          float t = x1 - float(xs);
          assert( t >= 0.0f && t <= 1.0f );

          float s = 1.0f - t;
          assert( s >= 0.0f && s <= 1.0f );

          for (unsigned int y = 0; y < _height; ++y)
	  {
              const ImagePixel& ps = pixel(xs, y);
              const ImagePixel& pt = pixel(xt, y);

	     assert( ps.r >= 0.f && ps.r <= 1.0f );
	     assert( ps.g >= 0.f && ps.g <= 1.0f );
	     assert( ps.b >= 0.f && ps.b <= 1.0f );
	     assert( ps.a >= 0.f && ps.a <= 1.0f );

             assert( pt.r >= 0.f && pt.r <= 1.0f );
             assert( pt.g >= 0.f && pt.g <= 1.0f );
             assert( pt.b >= 0.f && pt.b <= 1.0f );
             assert( pt.a >= 0.f && pt.a <= 1.0f );

	    ImagePixel p(
			 ps.r * s + pt.r * t,
			 ps.g * s + pt.g * t,
			 ps.b * s + pt.b * t,
			 ps.a * s + pt.a * t
			 );

	    // assert( p.r >= 0.f && p.r <= 1.0f );
	    // assert( p.g >= 0.f && p.g <= 1.0f );
	    // assert( p.b >= 0.f && p.b <= 1.0f );
	    // assert( p.a >= 0.f && p.a <= 1.0f );


	    scaled->pixel( x, y, p );
	  }
      }

    return scaled;
  }

/**
 * Scale video frame in Y using a box filter.
 *
 * @param f value to scale
 *
 * @return scaled video frame
 */
  VideoFrame* VideoFrame::scaleY(float f) const
  {
    unsigned int dh1 = scaleInt( f, _height );
    if ( dh1 < 1 ) dh1 = 1;

    //
    // Create a scaled frame
    //
    VideoFrame* scaled = new VideoFrame( _frame, 
					 _width,
					 dh1,
					 _channels,
					 _format,
					 _type,
					 _repeat );

    if ( dh1 > 1 )
      f = float(_height - 1) / float(dh1 - 1);
    else
      f = 1;

    for (unsigned int y = 0; y < dh1; ++y)
    {
	float y1 = float(y) * f;
	unsigned int ys = static_cast< unsigned int>( y1 );
	unsigned int yt = std::min( ys + 1, _height - 1 );
	float t = y1 - float(ys);
	assert( t >= 0.0f && t <= 1.0f );
	float s = 1.0f - t;
	assert( s >= 0.0f && s <= 1.0f );

	for (unsigned int x = 0; x < _width; ++x)
	{
           const ImagePixel& ps = pixel(x, ys);
	   const ImagePixel& pt = pixel(x, yt);

	    // assert( ps.r >= 0.f && ps.r <= 1.0f );
	    // assert( ps.g >= 0.f && ps.g <= 1.0f );
	    // assert( ps.b >= 0.f && ps.b <= 1.0f );
	    // assert( ps.a >= 0.f && ps.a <= 1.0f );

	    // assert( pt.r >= 0.f && pt.r <= 1.0f );
	    // assert( pt.g >= 0.f && pt.g <= 1.0f );
	    // assert( pt.b >= 0.f && pt.b <= 1.0f );
	    // assert( pt.a >= 0.f && pt.a <= 1.0f );

	    ImagePixel p(
			 ps.r * s + pt.r * t,
			 ps.g * s + pt.g * t,
			 ps.b * s + pt.b * t,
			 ps.a * s + pt.a * t
			 );


	    // assert( p.r >= 0.f && p.r <= 1.0f );
	    // assert( p.g >= 0.f && p.g <= 1.0f );
	    // assert( p.b >= 0.f && p.b <= 1.0f );
	    // assert( p.a >= 0.f && p.a <= 1.0f );

	    scaled->pixel( x, y, p );
	}
    }

    return scaled;
  }

/**
 * Quickly resize video frame without any filtering or smoothing of any kind.
 *
 * @param w width of resulting video frame
 * @param h height of resulting video frame
 *
 * @return resized video frame
 */
VideoFrame* VideoFrame::quick_resize( unsigned int w, unsigned int h ) const
{
     double fx, fy;
     if ( w == 0 || width() == 0 )
     {
	fx = 1.0;
     }
     else
     {
	fx = (double)width() / (double) w;
     }

    if ( h == 0 || height() == 0 )
    {
	fy = 1.0;
     }
    else
       fy = (double)height() / (double) h;

    //
    VideoFrame* scaled = new VideoFrame( _frame, w, h, 3, kRGB, kByte );

    for ( unsigned y = 0; y < h; ++y )
    {
       for ( unsigned x = 0; x < w; ++x )
       {
          Pixel p = this->pixel( unsigned( x*fx+.5 ), 
                                 unsigned( y*fy+.5 ) );
          scaled->pixel( x, y, p );
       }
    }
    return scaled;
}

/**
 * Resize video frame with box filtering.
 *
 * @param w width of resulting video frame
 * @param h height of resulting video frame
 *
 * @return resized video frame
 */
  VideoFrame* VideoFrame::resize( unsigned int w, unsigned int h ) const
  {

    double f;
     if ( w == 0 || width() == 0 )
	f = 1.0;
     else
	f = (double) w / (double)width();

    VideoFrame* scaledX = scaleX( f );

     if ( h == 0 || height() == 0 )
	f = 1.0;
     else
	f = (double) h / (double)height();

    VideoFrame* scaled  = scaledX->scaleY( f );
    delete scaledX;

    return scaled;
  }

/** 
 * Return whether picture has alpha or not.
 * 
 * @return true if it has alpha, false if not
 */
  bool VideoFrame::has_alpha() const
  {
    switch( _format )
      {
      case kLummaA:
      case kBGRA:
      case kRGBA:
      case kITU_601_YCbCr420A:
      case kITU_601_YCbCr422A:
      case kITU_601_YCbCr444A:
      case kITU_709_YCbCr420A:
      case kITU_709_YCbCr422A:
      case kITU_709_YCbCr444A:
      case kYByRy420A:
      case kYByRy422A:
      case kYByRy444A:
	return true;
      default:
	return false;
      }
  }

/** 
 * Equality operator.  Copy video frame into another video frame
 * 
 * @param b Video Frame to copy from
 * 
 * @return The new video frame.
 */
VideoFrame::self& VideoFrame::operator=( const VideoFrame::self& b )
{
   _frame    = b.frame();
   _pts      = b.pts();
   _repeat   = b.repeat();
   _width    = b.width();
   _height   = b.height(); 
   _channels = b.channels();
   _format   = b.format();
   _mtime    = b.mtime();
   _type     = b.pixel_type();
   allocate();
   memcpy( _data.get(), b.data().get(), data_size() );
   return *this;
}

} // namespace mrv
