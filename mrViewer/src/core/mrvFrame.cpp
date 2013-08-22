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

"ITU_702_YCbCr420",
"ITU_702_YCbCr420A", // @todo: not done
"ITU_702_YCbCr422",
"ITU_702_YCbCr422A", // @todo: not done
"ITU_702_YCbCr444",
"ITU_702_YCbCr444A", // @todo: not done

"YByRy420",
"YByRy420A", // @todo: not done
"YByRy422",
"YByRy422A", // @todo: not done
"YByRy444",
"YByRy444A", // @todo: not done
};

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

      case kITU_702_YCbCr420A:
      case kITU_601_YCbCr420A:
      case kYByRy420A:
	size = WH; // alpha

      case kYByRy420:
      case kITU_702_YCbCr420:
      case kITU_601_YCbCr420:
	size += WH;   // Y
	W2 = (_width  + 1) / 2;
	H2 = (_height + 1) / 2;
	WH2 = W2 * H2;
	size += 2 * WH2;  // U and V
	break;

      case kITU_702_YCbCr422:
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

  void VideoFrame::allocate()
  {
    mrv::aligned16_uint8_t* ptr = new mrv::aligned16_uint8_t[ data_size() ];
    assert( ((unsigned long)ptr) % 16 == 0 );

    _data.reset( ptr );
  }

  ImagePixel VideoFrame::pixel( const unsigned int x, const unsigned int y )
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

  VideoFrame* VideoFrame::scaleX(float f)
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
	float x1 = x * f;
	unsigned int xs = static_cast< unsigned int>( x1 );
	unsigned int xt = std::min( xs + 1, _width - 1 );
	float t = x1 - xs;
	float s = 1.0f - t;

	for (unsigned int y = 0; y < _height; ++y)
	  {
	    const ImagePixel& ps = pixel(xs, y);
	    const ImagePixel& pt = pixel(xt, y);
	    
	    ImagePixel p(
			 ps.r * s + pt.r * t,
			 ps.g * s + pt.g * t,
			 ps.b * s + pt.b * t,
			 ps.a * s + pt.a * t
			 );

	    scaled->pixel( x, y, p );
	  }
      }

    return scaled;
  }

  VideoFrame* VideoFrame::scaleY(float f)
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
	float y1 = y * f;
	unsigned int ys = static_cast< unsigned int>( y1 );
	unsigned int yt = std::min( ys + 1, _height - 1 );
	float t = y1 - ys;
	float s = 1.0f - t;

	for (unsigned int x = 0; x < _width; ++x)
	{
	    const ImagePixel ps = pixel(x, ys);
	    const ImagePixel pt = pixel(x, yt);
	    
	    ImagePixel p(
			 ps.r * s + pt.r * t,
			 ps.g * s + pt.g * t,
			 ps.b * s + pt.b * t,
			 ps.a * s + pt.a * t
			 );

	    scaled->pixel( x, y, p );
	}
    }

    return scaled;
  }

  VideoFrame* VideoFrame::resize( unsigned int w, unsigned int h )
  {
    float f = (float)w / (float)width();
    if ( f == 0.f ) f = 1.f;

    VideoFrame* scaledX = scaleX( f );

    f = (float)h / (float)height();
    if ( f == 0.f ) f = 1.f;

    VideoFrame* scaled  = scaledX->scaleY( f );
    delete scaledX;

    return scaled;
  }

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
      case kITU_702_YCbCr420A:
      case kITU_702_YCbCr422A:
      case kITU_702_YCbCr444A:
      case kYByRy420A:
      case kYByRy422A:
      case kYByRy444A:
	return true;
      default:
	return false;
      }
  }

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
