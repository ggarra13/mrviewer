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
 * @file   mrvDrawEngine.cpp
 * @author gga
 * @date   Sat Jul 14 07:36:01 2007
 * 
 * @brief  
 * 
 * 
 */


#include "sys/stat.h"

#include <cassert>
#include <limits>


#include "mrvSSE.h"


#ifdef WIN32
#  include <float.h>
#define isnan(x) _isnan(x)
#define isfinite(x) _finite(x)
#else
#  include <math.h>
#endif

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include <Iex.h>
#include <ImathMath.h> // for Math:: functions
#include <ImathFun.h>  // for clamp
#include <half.h>

#ifdef WIN32
#  include <wand/MagickWand.h>
#  include <magick/ImageMagick.h>
#else
#  include <wand/magick-wand.h>
#  include <magick/magick.h>
#endif

#include "CMedia.h"
#include "mrvImageView.h"
#include "mrvColorProfile.h"
#include "mrvException.h"
#include "mrvIO.h"
#include "mrViewer.h"
#include "mrvDrawEngine.h"
#include "mrvThread.h"
#include "core/mrvSSE.h"

using namespace Imath;
using namespace std;

typedef mrv::CMedia::Mutex   Mutex;


namespace {

  const char* kModule = "draw";


  /** 
   * Dummy function used to have a boost::shared_ptr that does not
   * delete its element.
   * 
   * @param ptr ptr to memory allocated by new uchar[x]
   */
  void shared_ptr_no_delete( uchar* ptr )
  {
  }

#pragma pack(push,16) /* Must ensure class & union 16-B aligned */

  struct RgbaGainGamma
  {
    F32vec4 m, g, ff;

    RgbaGainGamma (float gain, float gamma, float scale = 1.0f);
    float operator ()(const float x);

    F32vec4 operator()(const F32vec4& h );

    static F32vec4 oo;
  };

#pragma pack(pop)


  F32vec4 RgbaGainGamma::oo = F32vec4( 0.0f, 0.0f, 0.0f, 0.0f );


  RgbaGainGamma::RgbaGainGamma (float gain, float gamma, float scale ):
    m( F32vec4( 1.0f, gain, gain, gain ) ),
    g( F32vec4( 1.0f / gamma, 1.0f / gamma, 1.0f / gamma, 1.0f / gamma ) ),
    ff( F32vec4( scale, scale, scale, scale ) )
  {}


  float
  RgbaGainGamma::operator ()(float x)
  {
    //
    // Exposure
    //
    x *= *((float*)&m+1);

    //
    // RgbaGainGamma
    //
    if ( x > 0.f )
        x = Imath::Math<float>::pow(x, *((float*)&g+1));

    //
    // Scale and clamp
    //
    return clamp (x, 0.f, 1.0f);
  }


  F32vec4
  RgbaGainGamma::operator()( const F32vec4& h )
  {
    F32vec4 x = h;
    float* f = (float*)&x;

    //
    // Exposure
    //
    x *= m;

    // From apple's vfp.h, this should really be:
    //
    // *x = vpowf( *x, g );
    //
    // not sure what to do on windows/linux yet

    if ( f[0] > 0.f )
        f[0] = Imath::Math<float>::pow(f[0], *((float*)&g+1));
    if ( f[1] > 0.f )
        f[1] = Imath::Math<float>::pow(f[1], *((float*)&g+1));
    if ( f[2] > 0.f )
        f[2] = Imath::Math<float>::pow(f[2], *((float*)&g+1));
    if ( f[3] > 0.f )
        f[3] = Imath::Math<float>::pow(f[3], *((float*)&g+1));

    // Clamp
    x = select_lt( x, oo, oo, x );
    x = select_gt( x, ff, ff, x );

    return x;
  }



  //
  //  Dithering: Reducing the raw 16-bit pixel data to 8 bits for the
  //  OpenGL frame buffer can sometimes lead to contouring in smooth
  //  color ramps.  Dithering with a simple Bayer pattern eliminates
  //  visible contouring.
  //

  unsigned char
  dither (float v, int x, int y)
  {
    static const float d[4][4] =
      {
	{ 0.f / 16,  8.f / 16,  2.f / 16, 10.f / 16 },
	{ 12.f / 16,  4.f / 16, 14.f / 16,  6.f / 16 },
	{ 3.f / 16, 11.f / 16,  1.f / 16,  9.f / 16 },
	{ 15.f / 16,  7.f / 16, 13.f / 16,  5.f / 16 },
      };

    return (unsigned char) (v + d[y & 3][x & 3]);
  }



unsigned widthPerThread  = 1024;
unsigned heightPerThread = 32;
unsigned numPixelsPerThread = widthPerThread * heightPerThread;


/**
 * Prepare an image (or portion of it) for display on an 8-bit device.
 *
 * @param img   image to display
 * @param data  pre-allocated buffer to store 8-bit data
 * @param alpha create buffer with an alpha channel
 */
void display_cb( mrv::DrawEngine::DisplayData* d )
{
  using namespace mrv;

  int step = 3;
  if ( d->alpha ) step = 4;


  const mrv::Recti& rect = d->rect;
  int x, y;
  int xl = rect.x();
  int yl = rect.y();

  int xh = rect.r();
  int yh = rect.b();

  mrv::image_type_ptr orig = d->orig;
  if (!orig) return;

  mrv::image_type_ptr result = d->result;
  if (!result) return;

#if 0   
  CMedia* img = d->image;
  int dw = img->width();
  int oxl = xl;
  int oyl =  0;
  int odw = dw;

  // Convert image using monitor profile
  if ( d->view->use_lut() )
    {
      MagickBooleanType status;
      MagickWandGenesis();
      MagickWand* wand = NewMagickWand();

      char* fmt = "RGBA";

      int bw = xh - xl;
      int bh = yh - yl;
      CMedia::Pixel* icc = new CMedia::Pixel[ bw * bh ];
      CMedia::Pixel* p   = icc;
      for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x, ++p )
	    {
	      *p = frame->pixel(x,y);
	    }
	}

      status = MagickConstituteImage( wand, bw, bh, fmt, FloatPixel, icc );
      if ( status != MagickTrue )
	LOG_ERROR( "Could not create image" );


      uchar* profile;
      size_t  length;

      colorProfile::get( profile, length, d->image->color_profile() );

      if ( profile )
	{
	  status  = MagickSetImageProfile( wand, "ICC", profile, length );
	  if ( status != MagickTrue )
	    LOG_ERROR( "Could not profile image" );

	  colorProfile::get_monitor_profile( profile, length );
	  if ( profile )
	    {
	      status  = MagickProfileImage( wand, "ICC", profile, length );
	      if ( status != MagickTrue )
		LOG_ERROR( "Could not profile image" );
	    }
	}


      MagickGetImagePixels(wand, 0, 0, bw, bh, fmt, FloatPixel, icc );

      DestroyMagickWand( wand );
      MagickWandTerminus();

      fp = icc;
      oxl = 0;
      oyl = yl;
      odw = bw;
    }
#endif


  // Change float image to uchar based on gain and gamma.
  RgbaGainGamma RGBAfunc( d->view->gain(), d->view->gamma() );
  RgbaGainGamma    Afunc( 1.0f, d->view->gamma() );



  switch( d->view->channel_type() )
    {
    case kRed:
      for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x )
	    {
	      CMedia::Pixel p = orig->pixel(x,y);
	      p.r = p.g = p.b = RGBAfunc( p.r );
	      result->pixel( x, y, p );
	    }
	}
      break;
    case kGreen:
      for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x )
	    {
	      CMedia::Pixel p = orig->pixel(x,y);
	      p.r = p.g = p.b = RGBAfunc( p.g );
	      result->pixel( x, y, p );
	    }
	}
      break;
    case kBlue:
      for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x )
	    {
	      CMedia::Pixel p = orig->pixel(x,y);
	      p.r = p.g = p.b = RGBAfunc( p.b );
	      result->pixel( x, y, p );
	    }
	}
      break;
    case kAlpha:
      {
	mrv::media m = d->view->background();

	for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x )
	    {
	      CMedia::Pixel p = orig->pixel(x,y);
	      p = RGBAfunc( p );
	      if ( d->view->show_background() )
		{
		  p.r = p.g = p.b = 1.0f;
		}
	      else
		{
		  p.r = p.g = p.b = p.a;
		}
	      result->pixel( x, y, p );
	    }
	}
      }
      break;
    case kAlphaOverlay:
      for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x )
	    {
	      CMedia::Pixel p = orig->pixel(x,y);
	      p = RGBAfunc( p );
	      p.r = p.a * 0.5f + p.r * 0.5f;
	      result->pixel( x, y, p );
	    }
	}
      break;
    case kLumma:
      for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x )
	    {
	      CMedia::Pixel p = orig->pixel(x,y);
	      p = RGBAfunc( p );
	      p.r = p.g = p.b = (p.r + p.g + p.b)/3.0f;
	      result->pixel( x, y, p );
	    }
	}
      break;
    case kRGB:
    default:
      for ( y = yl; y < yh; ++y )
	{
	  for ( x = xl; x < xh; ++x )
	    {
	      CMedia::Pixel p = orig->pixel(x,y);
	      p = RGBAfunc( p );
	      result->pixel( x, y, p );
	    }
	}
      break;
    }

  // delete temporary icc image/tile
#if 0
  if ( d->view->use_lut() )
    {
      delete [] fp;
    }
#endif

}

  void minmax_cb( mrv::DrawEngine::MinMaxData* d )
  {
    using mrv::CMedia;

    unsigned int xl = d->rect.x();
    unsigned int yl = d->rect.y();

    unsigned int xh = xl + d->rect.w();
    unsigned int yh = yl + d->rect.h();

    const CMedia* img = d->image;
    mrv::image_type_ptr frame = img->hires();
    if ( !frame ) return;

    d->pMin = std::numeric_limits< float >::max();
    d->pMax = std::numeric_limits< float >::min();

    for ( unsigned int y = yl ; y < yh; ++y )
      {
	for ( unsigned int x = xl; x < xh; ++x )
	  {
	    const CMedia::Pixel& p = frame->pixel( x, y );

            if ( half(p.r).isFinite() )
            {
               if ( p.r > d->pMax ) d->pMax = p.r;
               if ( p.r < d->pMin ) d->pMin = p.r;
            }

            if ( half(p.g).isFinite()  )
            {
               if ( p.g < d->pMin ) d->pMin = p.g;
               if ( p.g > d->pMax ) d->pMax = p.g;
            }

            if ( half(p.b).isFinite()  )
            {
               if ( p.b < d->pMin ) d->pMin = p.b;
               if ( p.b > d->pMax ) d->pMax = p.b;
            }

	  }
      }

    if ( d->pMin == std::numeric_limits<float>::max() )
        d->pMin = 0.0f;

    if (d->pMax <= d->pMin)
	d->pMax = d->pMin + 1;

  }


} // namespace


namespace mrv {

  DrawEngine::ShaderType DrawEngine::_hardwareShaders = DrawEngine::kAuto;
  bool DrawEngine::_has_yuv  = false;
  bool DrawEngine::_has_yuva = false;


  static const char* kShaderType[] = 
    {
      "None",
      "Auto",
      "OpenGL 2.0 GLSL",
      "NVidia NV30",
      "ARB Fragment Program 1",
    };

  const char* DrawEngine::shader_type_name()
  {
    return kShaderType[ _hardwareShaders ];
  }

  DrawEngine::DrawEngine(const mrv::ImageView* v) :
    _view(v),
    _normMin( 0 ),
    _normMax( 1.0f ),
    _background_resized( 0 )
  {
  }


  DrawEngine::~DrawEngine()
  {
    delete _background_resized;
  }



  CMedia* const DrawEngine::background()
  {
    if ( _background_resized ) return _background_resized;
    else {
      mrv::media bg = _view->background();
      if ( !bg ) return NULL;

      return bg->image();
    }
  }


  void DrawEngine::display( image_type_ptr& result,
			    const image_type_ptr& src,
			    CMedia* img )
  {
    Mutex& m = img->video_mutex();
    SCOPED_LOCK( m );

    image_type_ptr pic      = src;
    int64_t frame           = pic->frame();
    unsigned int dw         = pic->width();
    unsigned int dh         = pic->height();
    unsigned short channels = pic->channels();
    image_type::Format    format = pic->format();

    unsigned int x, y;

    if ( _view->normalize() )
      {
	float vMin[2];
	float vMax[2];
	for ( x = 0; x < 2; ++x )
	  {
	    vMin[x] = std::numeric_limits<float>::max();
	    vMax[x] = std::numeric_limits<float>::min();
	  }

	for ( y = 0; y < dh; ++y )
	  {
	    for ( x = 0; x < dw; ++x )
	      {
		const CMedia::Pixel& rp = pic->pixel(x,y);

		if ( isnan( rp.a ) )
		  continue;

		if ( rp.a < vMin[1] ) vMin[1] = rp.a;
		if ( rp.a > vMax[1] ) vMax[1] = rp.a;

		if ( isfinite( rp.r ) && !isnan(rp.r) )
		  {
		    if ( rp.r < vMin[0] ) vMin[0] = rp.r;
		    if ( rp.r > vMax[0] ) vMax[0] = rp.r;
		  }
		if ( isfinite( rp.g ) && !isnan(rp.g) )
		  {
		    if ( rp.g < vMin[0] ) vMin[0] = rp.g;
		    if ( rp.g > vMax[0] ) vMax[0] = rp.g;
		  }

		if ( isfinite( rp.b ) && !isnan(rp.b) )
		  {
		    if ( rp.b < vMin[0] ) vMin[0] = rp.b;
		    if ( rp.b > vMax[0] ) vMax[0] = rp.b;
		  }
	      }
	  }

	for ( x = 0; x < 2; ++x )
	  {
	    vMax[x] -= vMin[x];
	    if ( vMax[x] == 0.0f ) vMax[x] = 1.0f;
	  }

	mrv::image_type_ptr p( new image_type( frame,
					       dw,
					       dh,
					       channels,
					       format,
					       src->pixel_type() ) );

	for ( y = 0; y < dh; ++y )
	  {
	    for ( x = 0; x < dw; ++x )
	      {
		CMedia::Pixel rp = pic->pixel(x, y);

		if ( isfinite( rp.r ) )
		  rp.r = (rp.r - vMin[0]) / vMax[0];
		if ( isfinite( rp.g ) )
		  rp.g = (rp.g - vMin[0]) / vMax[0];
		if ( isfinite( rp.b ) )
		  rp.b = (rp.b - vMin[0]) / vMax[0];
		rp.a = (rp.a - vMin[1]) / vMax[1];

		p->pixel( x, y, rp );
	      }
	  }

	pic = p;
      }

    // Based on window size, spawn several threads or just jump to
    // process tile.
    const mrv::Recti& rect = img->damage_rectangle();
    unsigned int xl = rect.x();
    unsigned int yl = rect.y();

    unsigned int xh = rect.r();
    unsigned int yh = rect.b();

    unsigned int xw = xh - xl;
    unsigned int yw = yh - yl;

    unsigned int num = xw * yw;

   if ( num < numPixelsPerThread )
   {
	DisplayData display_data;
	display_data.view   = _view;
	display_data.orig   = pic;
	display_data.result = result;
	display_data.image  = img;
	display_data.rect   = rect;
	display_data.alpha  = result->has_alpha();
	display_cb( &display_data );
      }
    else
      {
	unsigned int x, y;


	unsigned int ny, nx;
	for ( y = yl; y < yh; y = ny )
	  {
	    ny   = y + heightPerThread;
	    unsigned int byh  = yh < ny? yh : ny;

	    boost::thread* thread_id;
	    for ( x = xl; x < xh; x = nx )
	      {
		nx  = x + widthPerThread;
		int bxh = xh < nx? xh : nx;

		DisplayData* display_data = new DisplayData;
		display_data->view   = _view;
		display_data->orig   = pic;
		display_data->result = result;
		display_data->image  = img;
		display_data->rect   = mrv::Recti( x, y, bxh-x, byh-y );
		display_data->alpha  = result->has_alpha();

		thread_id = new boost::thread( boost::bind( display_cb,
							    display_data ) );

		buckets.insert( std::make_pair( thread_id, display_data ) );
	      }
	  }


	BucketList::iterator i = buckets.begin();
	BucketList::iterator e = buckets.end();

	// Make sure all threads finish before proceeding
	for ( ; i != e; ++i )
	  {
	    i->first->join();

	    DisplayData*  opaque = (DisplayData*) i->second;
	    delete opaque;
	  }
	buckets.clear();
      }

  }

  /**
   * Prepare an image for display on an 8-bit device.
   *
   * @param data   pre-allocated buffer to store 8-bit data
   * @param img    image to display
   * @param alpha  if set, create 8-bit buffer with an alpha channel
   */
  image_type_ptr DrawEngine::display( const image_type_ptr& src,
				      CMedia* img )
  {
    if ( !src ) return src;


    mrv::image_type_ptr result( new image_type( src->frame(),
						src->width(),
						src->height(),
						src->channels(),
						src->format(),
						image_type::kByte ) );

    display( result, src, img );


    //   //
    //   // @todo: move this to GLEngine
    //   //
    //   // Scale 8-bit data to compensate for pixel ratio
    //   if ( !_drawQuads && _showPixelRatio && _pixelRatio != 1.0f )
    //     {
    //       int dh1 = (int) (dh / _pixelRatio);

    //       // scale pixels to match ratio
    //       boost::uint8_t* scaled = new boost::uint8_t[ dw * dh1 * ( 3 + 1 * alpha ) ];

    //       GLenum format = GL_RGB;
    //       if ( alpha ) format = GL_RGBA;

    //       gluScaleImage( format,
    // 		     dw, dh,  GL_UNSIGNED_BYTE, data.get(),
    // 		     dw, dh1, GL_UNSIGNED_BYTE, scaled
    // 		     );

    //       data = boost::shared_array< boost::uint8_t >(scaled);
    //     }

    _view->main()->uiPixelBar->redraw();

    return result;
  }


  /// Find min/max values for an image, using multithreading if possible
  void DrawEngine::minmax( float& pMin, float& pMax,
			   const CMedia* img )
  {
    unsigned int xh = img->width();
    unsigned int yh = img->height();

    MinMaxData data;
    data.image  = img;
    data.rect   = mrv::Recti( 0, 0, xh, yh );

    minmax_cb( &data );

    pMin = data.pMin;
    pMax = data.pMax;

  }


  void DrawEngine::minmax() {
    _normMin = std::numeric_limits< float >::max();
    _normMax = std::numeric_limits< float >::min();
    {
        const mrv::media& m = _view->foreground();
        if ( m )
            minmax( _normMin, _normMax, m->image() );
    }
    {
        const mrv::media& m = _view->background();
        if ( m ) 
            minmax( _normMin, _normMax, m->image() );
    }
  }
  
} // namespace mrv
