/**
 * @file   mrvFrame_h16
 * @author gga
 * @date   Sun Jan 13 13:51:37 2008
 * 
 * @brief  Pixel operations using 32-bit floats.
 * 
 * 
 */

namespace mrv {

  ImagePixel VideoFrame::pixel_f32( const unsigned int x, 
				    const unsigned int y ) const
  {
    ImagePixel p( 0.f, 0.f, 0.f, 0.f );
    unsigned offset = y * _width + x;

    float* d = (float*) _data.get();
    float* col = d + offset * _channels;
    float yp = 0.f, cb = 0.f, cr = 0.f;
    switch( _format )
      {
      case kLummaA:
	{
	  unsigned int len = _width * _height;
	  p.a = d[len + offset];
	}
      case kLumma:
	p.r = p.g = p.b = col[0]; break;
      case kBGRA:
	p.a = col[3];
      case kBGR:
	p.r = col[2];
	p.g = col[1];
	p.b = col[0];
	break;
      case kRGBA:
	p.a = col[3];
      case kRGB:
	p.r = col[0];
	p.g = col[1];
	p.b = col[2];
	break;
      case kITU_709_YCbCr444:
      case kITU_601_YCbCr444:
	{
	  unsigned int len = _width * _height;
	  yp = d[ offset];
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
	  p.a = d[ Ylen + Cblen * 2 + offset];
	}
      case kITU_709_YCbCr420:
      case kYByRy420:
      case kITU_601_YCbCr420:
	{
	  unsigned int Ylen    = _width * _height;
	  unsigned int w2      = (_width  + 1) / 2;
	  unsigned int h2      = (_height + 1) / 2;
	  unsigned int Cblen   = w2 * h2;
	  unsigned int offset2 = (y/2) * w2 + x / 2;

	  yp = d[ offset ];
	  cb = d[ Ylen + offset2 ];
	  cr = d[ Ylen + Cblen + offset2 ];
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
	  break;
	}
      default:
	throw std::runtime_error("Unknown mrv::Frame format");
      }

    if ( _format >= kYByRy420 )
      {
	 p.r = float( (cb + 1) * yp );
	 p.b = float( (cr + 1) * yp );
	 p.g = float( (yp - p.r * yw[0] - p.b * yw[2]) / yw[1] );
      }
    else if ( _format >= kITU_709_YCbCr420 )
      {
	// ITU_709_YCbCr conversion
	float  Y = yp;
	float Pb = cb - 0.5f;
	float Pr = cr - 0.5f;

	p.r = Y                  + Pr * 1.402f;
	p.g = Y - Pb * 0.344136f - Pr * 0.714136f;
	p.b = Y + Pb * 1.772f;
      }
    else if ( _format >= kITU_601_YCbCr420 )
      {
	// YCbCr conversion
	float  Y = yp - 0.0625f;
	float Cb = cb - 0.5f;
	float Cr = cr - 0.5f;

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

  void VideoFrame::pixel_f32( const unsigned int x, const unsigned int y,
			      const ImagePixel& p )
  {
    unsigned offset = y * _width + x;

    float* d = (float*) _data.get();
    float* col = d + offset * _channels;
    float* yp = NULL, *cb = NULL, *cr = NULL;
    switch( _format )
      {
      case kLummaA:
	{
	  unsigned int len = _width * _height;
	  d[len + offset] = p.a * 255.0f;
	}
      case kLumma:
	col[0] = p.r; break;
      case kBGRA:
	col[3] = p.a;
      case kBGR:
	col[2] = p.r;
	col[1] = p.g;
	col[0] = p.b;
	break;
      case kRGBA:
	col[3] = p.a;
      case kRGB:
	col[0] = p.r;
	col[1] = p.g;
	col[2] = p.b;
	break;
      case kITU_709_YCbCr444:
      case kITU_601_YCbCr444:
	{
	  unsigned int len = _width * _height;
	  yp = d + offset;
	  cb = d + len + offset;
	  cr = d + len*2 + offset;
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
	  d[ Ylen + Cblen * 2 + offset] = p.a;
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
	*yp = p.r * yw[0] + p.g * yw[1] + p.b * yw[2];
	if ( *yp > 0.0f )
	  {
	    *cr = ( p.r - *yp ) / *yp;
	    *cb = ( p.b - *yp ) / *yp;
	  }
	else
	  {
	    *cr = *cb = 0.0f;
	  }
      }
    else if ( _format >= kITU_709_YCbCr420 )
      {
	*yp =  p.r * 0.299f    + p.g * 0.587f    + p.b * 0.114f;
	*cb = -p.r * 0.168736f - p.g * 0.331264f + p.b * 0.5f;
	*cr =  p.r * 0.500000f - p.g * 0.418688f - p.b * 0.081312f;
      }
    else if ( _format >= kITU_601_YCbCr420 )
      {
	*yp = 16  + p.r * 65.481f + p.g * 128.553f + p.b * 24.966f;
	*cb = 128 - p.r * 37.797f - p.g * 74.203f  + p.b * 112.0f;
	*cr = 128 + p.r * 112.0f  - p.g * 93.786f  - p.b * 18.214f;
	// Sanity check
	if      ( *yp < 1.0f   ) *yp = 1.0f;
	else if ( *yp > 254.0f ) *yp = 254.0f;
	if      ( *cb < 1.0f   ) *cb = 1.0f;
	else if ( *cb > 254.0f ) *cb = 254.0f;
	if      ( *cr < 1.0f   ) *cr = 1.0f;
	else if ( *cr > 254.0f ) *cr = 254.0f;
      }
  }



} // namespace mrv
