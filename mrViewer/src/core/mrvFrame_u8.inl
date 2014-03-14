/**
 * @file   mrvFrame_u8.inl
 * @author gga
 * @date   Thu Jan 24 15:26:04 2008
 * 
 * @brief  Pixel operations using 8-bit unsigned chars
 * 
 * 
 */

namespace mrv {

  ImagePixel VideoFrame::pixel_u8( const unsigned int x, 
				   const unsigned int y ) const
  {
    ImagePixel p( 0.f, 0.f, 0.f, 0.f );
    unsigned offset = y * _width + x;

    boost::uint8_t* d = (boost::uint8_t*)_data.get();
    boost::uint8_t* col = d + offset * _channels;
    boost::uint8_t yp = 0, cb = 0, cr = 0;
    switch( _format )
      {
      case kLummaA:
	{
	  unsigned int len = _width * _height;
	  p.a = d[len + offset] / 255.0f;
	}
      case kLumma:
	p.r = p.g = p.b = col[0] / 255.0f; break;
      case kBGRA:
	p.a = col[3] / 255.0f;
      case kBGR:
	p.r = col[2] / 255.0f;
	p.g = col[1] / 255.0f;
	p.b = col[0] / 255.0f;
	break;
      case kRGBA:
	p.a = col[3] / 255.0f;
      case kRGB:
	p.r = col[0] / 255.0f;
	p.g = col[1] / 255.0f;
	p.b = col[2] / 255.0f;
	break;
      case kITU_709_YCbCr444:
      case kITU_601_YCbCr444:
	{
	  unsigned int len = _width * _height;
	  yp = d[ offset ];
	  cb = d[ len + offset ];
	  cr = d[ len*2 + offset ];
	  break;
	}
      case kITU_709_YCbCr420A:
      case kITU_601_YCbCr420A:
      case kYByRy420A:
	{
	  unsigned int Ylen    = _width * _height;
	  unsigned int w2      = (_width  + 1) / 2;
	  unsigned int h2      = (_height + 1) / 2;
	  unsigned int Cblen   = w2 * h2;
	  p.a = d[ Ylen + Cblen * 2 + offset] / 255.0f;
	}
      case kITU_709_YCbCr420:
      case kITU_601_YCbCr420:
      case kYByRy420:
	{
	  unsigned int Ylen    = _width * _height;
	  unsigned int w2      = (_width  + 1) / 2;
	  unsigned int h2      = (_height + 1) / 2;
	  unsigned int Cblen   = w2 * h2;
	  unsigned int offset2 = (y/2) * w2 + x / 2;

	  yp = d[ offset ];
	  cb = d[ Ylen + offset2 ];
	  cr = d[ Ylen + Cblen + offset2 ];
	  p.a = 1.0f;
	  break;
	}
      case kITU_709_YCbCr422:
      case kITU_601_YCbCr422:
	{
	  unsigned int  Ylen = _width * _height;
	  unsigned int w2 = (_width + 1) / 2;
	  unsigned int Cblen = w2 * _height;
	  unsigned int offset2 = y * w2 + x / 2;
	  yp = d[ offset ];
	  cb = d[ Ylen + offset2 ];
	  cr = d[ Ylen + Cblen + offset2 ];
	  p.a = 1.0f;
	  break;
	}
      default:
	throw std::runtime_error("Unknown mrv::Frame format");
      }

    if ( _format >= kYByRy420 )
      {
	 p.r = float( (cb + 1) * yp );
	 p.b = float( (cr + 1) * yp );
	 p.g = float( (yp - p.r * yw[0] - p.b * yw[2]) / yw[1] * 255.0f);
      }
    else if ( _format >= kITU_709_YCbCr420 )
      {
	// ITU. 702 YCbCr conversion
	 float  Y = float(yp) / 255.0f;
	 float Pb = float(cb) / 255.0f - 0.5f;
	 float Pr = float(cr) / 255.0f - 0.5f;

	 assert( Y >= 0.0f && Y <= 1.0f );
	 assert( Pb >= -0.5f && Pb <= 0.5f );
	 assert( Pr >= -0.5f && Pr <= 0.5f );

	 p.r = Y                  + Pr * 1.402f;
	 p.g = Y - Pb * 0.344136f - Pr * 0.714136f;
	 p.b = Y + Pb * 1.772f;


      }
    else if ( _format >= kITU_601_YCbCr420 )
      {
	// ITU. 601 YCbCr conversion
	float  Y = float( short(yp) - 16  );
	float Cb = float( short(cb) - 128 );
	float Cr = float( short(cr) - 128 );

	p.r = Y * 0.00456621f                    + Cr * 0.00625893f;
	p.g = Y * 0.00456621f - Cb * 0.00153632f - Cr * 0.00318811f;
	p.b = Y * 0.00456621f + Cb * 0.00791071f;

	// Sanity check. Needed, as ffmpeg can return invalid values
	if ( p.r < 0.0f )      p.r = 0.0f;
	else if ( p.r > 1.0f ) p.r = 1.0f;

	if ( p.g < 0.0f )      p.g = 0.0f;
	else if ( p.g > 1.0f ) p.g = 1.0f;

	if ( p.b < 0.0f )      p.b = 0.0f;
	else if ( p.b > 1.0f ) p.b = 1.0f;

      }

    return p;
  }

  void VideoFrame::pixel_u8( const unsigned int x, const unsigned int y,
			     const ImagePixel& p )
  {
    unsigned offset = y * _width + x;

    boost::uint8_t* d = (boost::uint8_t*)_data.get();
    boost::uint8_t* col = d + offset * _channels;
    boost::uint8_t* yp = NULL, *cb = NULL, *cr = NULL;
    switch( _format )
      {
      case kLummaA:
	{
	  unsigned int len = _width * _height;
	  d[len + offset] = boost::uint8_t( p.a * 255.0f );
	}
      case kLumma:
	col[0] = boost::uint8_t(p.r * 255.0f); break;
      case kBGRA:
	col[3] = boost::uint8_t(p.a * 255.0f);
      case kBGR:
	col[2] = boost::uint8_t(p.r * 255.0f);
	col[1] = boost::uint8_t(p.g * 255.0f);
	col[0] = boost::uint8_t(p.b * 255.0f);
	break;
      case kRGBA:
	col[3] = boost::uint8_t(p.a * 255.0f);
      case kRGB:
	col[0] = boost::uint8_t(p.r * 255.0f);
	col[1] = boost::uint8_t(p.g * 255.0f);
	col[2] = boost::uint8_t(p.b * 255.0f);
	break;
      case kITU_709_YCbCr444:
      case kITU_601_YCbCr444:
	{
	  unsigned int len = _width * _height;
	  yp = d + offset;
	  cb = d + offset + len;
	  cr = d + offset + len*2;
	  break;
	}
      case kITU_709_YCbCr420A:
      case kITU_601_YCbCr420A:
      case kYByRy420A:
	{
	  unsigned int Ylen    = _width * _height;
	  unsigned int w2      = (_width  + 1) / 2;
	  unsigned int h2      = (_height + 1) / 2;
	  unsigned int Cblen2  = w2 * h2 * 2;
	  d[ Ylen + Cblen2 + offset] = boost::uint8_t(p.a * 255.0f);
	}
      case kITU_709_YCbCr420:
      case kITU_601_YCbCr420:
      case kYByRy420:
	{
	  unsigned int Ylen    = _width * _height;
	  unsigned int w2      = (_width  + 1) / 2;
	  unsigned int h2      = (_height + 1) / 2;
	  unsigned int Cblen   = w2 * h2;
	  unsigned int offset2 = (y/2) * w2 + x / 2;

	  yp = d + offset;
	  cb = d + Ylen + offset2;
	  cr = d + Ylen + Cblen + offset2;
	  break;
	}
      case kITU_709_YCbCr422:
      case kITU_601_YCbCr422:
	{
	  unsigned int  Ylen = _width * _height;
	  unsigned int w2 = (_width + 1) / 2;
	  unsigned int Cblen = w2 * _height;
	  unsigned int offset2 = y * w2 + x / 2;
	  yp = d + offset;
	  cb = d + Ylen + offset2;
	  cr = d + Ylen + Cblen + offset2;
	  break;
	}
      default:
	throw std::runtime_error("Unknown mrv::Frame format");
      }

    if ( _format >= kYByRy420 )
      {
	assert( yp && cb && cr );

	ImagePixel t = p;
	if ( t.r < 0.0f )      t.r = 0.0f;
	else if ( t.r > 1.0f ) t.r = 1.0f;

	if ( t.g < 0.0f )      t.g = 0.0f;
	else if ( t.g > 1.0f ) t.g = 1.0f;

	if ( t.b < 0.0f )      t.b = 0.0f;
	else if ( t.b > 1.0f ) t.b = 1.0f;

	float Y = t.r * yw[0] + t.g * yw[1] + t.b * yw[2];
        *yp = boost::uint8_t( Y * 255.0f);
	if ( *yp > 0.0f )
	  {
	    *cr = boost::uint8_t( 128 + 255 * (( t.r - Y ) / Y) );
	    *cb = boost::uint8_t( 128 + 255 * (( t.b - Y ) / Y) );
	  }
	else
	  {
	    *cr = *cb = 0;
	  }
      }
    else if ( _format >= kITU_709_YCbCr420 )
      {
	assert( yp && cb && cr );

	*yp = boost::uint8_t(  p.r * 76.245f   + p.g * 149.685f   + 
			       p.b * 29.07f    );
	*cb = boost::uint8_t( -p.r * 43.02768f - p.g * 84.47232f  + 
			       p.b * 127.5f    );
	*cr = boost::uint8_t(  p.r * 127.5f    - p.g * 106.76544f - 
			       p.b * 20.73456f );
      }
    else if ( _format >= kITU_601_YCbCr420 )
      {
	ImagePixel t = p;
	if ( t.r < 0.0f )      t.r = 0.0f;
	else if ( t.r > 1.0f ) t.r = 1.0f;

	if ( t.g < 0.0f )      t.g = 0.0f;
	else if ( t.g > 1.0f ) t.g = 1.0f;

	if ( t.b < 0.0f )      t.b = 0.0f;
	else if ( t.b > 1.0f ) t.b = 1.0f;

	assert( yp && cb && cr );
	*yp = boost::uint8_t(16  + 65.481f * t.r + 128.553f * t.g + 
			     24.966f * t.b);
	*cb = boost::uint8_t(128 - 37.797f * t.r - 74.203f  * t.g + 
			     112.f   * t.b);
	*cr = boost::uint8_t(128 + 112.f   * t.r - 93.786f  * t.g - 
			     18.214f * t.b);
      }
  }

}
