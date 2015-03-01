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
 * @file   mrvHistogram.cpp
 * @author gga
 * @date   Sat Nov 11 11:36:59 2006
 * 
 * @brief  Draw an image's histogram
 * 
 * 
 */

#include <cmath>
#include <limits>

#ifdef _WIN32
#define isfinite(x) _finite(x)
#endif

#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Symbol.h>

#include "GL/glew.h"

#include "core/CMedia.h"
#include "core/mrvThread.h"

#include "gui/mrvIO.h"
#include "gui/mrvHistogram.h"
#include "mrViewer.h"
#include "gui/mrvImageView.h"
#include "gui/mrvColorInfo.h"

#include "video/mrvDrawEngine.h"

#include "ImathFun.h"


namespace mrv
{

  Histogram::Histogram( int x, int y, int w, int h, const char* l ) :
    fltk::Widget( x, y, w, h, l ),
    _channel( kRGB ),
    _histtype( kLog ),
    maxLumma( 0 ),
    maxRed( 0 ),
    maxGreen( 0 ),
    maxBlue( 0 ),
    lastImg( NULL ),
    lastFrame( std::numeric_limits< int64_t >::min() )
  {
    color( fltk::BLACK );
    buttoncolor( fltk::BLACK );

    add_timeout(0.016f);
  }


  void Histogram::draw_grid(const fltk::Rectangle& r)
  {
//     fltk::setcolor( fltk::GRAY75 );

//     int X = r.x() + 2;
//     int H = r.h() / 4;
//     int H2 = ( H + fltk::getsize() ) / 2;
//     fltk::drawtext( "L", 1, X, H2 );
//     fltk::drawtext( "R", 1, X, H+H2 );
//     fltk::drawtext( "G", 1, X, H*2+H2 );
//     fltk::drawtext( "B", 1, X, H*3+H2 );
  }

  void Histogram::draw()
  {
    fltk::Rectangle r( w(), h() );
    draw_box(r);

    // draw_grid(r);
    draw_pixels(r);
  }


  void Histogram::count_pixel( const uchar* rgb )
  {
    uchar r = rgb[0];
    uchar g = rgb[1];
    uchar b = rgb[2];

    red[ r ]   += 1;
    if ( red[r] > maxRed ) maxRed = red[r];

    green[ g ] += 1;
    if ( green[g] > maxGreen ) maxGreen = green[g];

    blue[ b ]  += 1;
    if ( blue[b] > maxBlue ) maxBlue = blue[b];

    unsigned int lum = unsigned(r * 0.30f + g * 0.59f + b * 0.11f);
    lumma[ lum ] += 1;
    if ( lumma[lum] > maxLumma ) maxLumma = lumma[lum];
  }


  void Histogram::count_pixels()
  {

#if 0
    if ( GLEW_ARB_imaging )
      {
	memset( red,   0, sizeof(float) * 256 );
	memset( green, 0, sizeof(float) * 256 );
	memset( blue,  0, sizeof(float) * 256 );
	memset( lumma, 0, sizeof(float) * 256 );
	glGetHistogramEXT( GL_HISTOGRAM, GL_FALSE, GL_RED, GL_UNSIGNED_INT,
			   red );
	glGetHistogramEXT( GL_HISTOGRAM, GL_FALSE, GL_GREEN, GL_UNSIGNED_INT,
			   green );
	glGetHistogramEXT( GL_HISTOGRAM, GL_FALSE, GL_BLUE, GL_UNSIGNED_INT,
			   blue );
	glGetHistogramEXT( GL_HISTOGRAM, GL_FALSE, GL_LUMMA, 
			   GL_UNSIGNED_INT, lumma );
	unsigned int minmax[2];
	minmax[0] = minmax[1] = 0;
	glGetMinmaxEXT( GL_MINMAX, GL_FALSE, GL_RED, GL_UNSIGNED_INT,
			minmax );
	maxRed = minmax[1];
	glGetMinmaxEXT( GL_MINMAX, GL_FALSE, GL_GREEN, GL_UNSIGNED_INT,
			minmax );
	maxGreen = minmax[1];
	glGetMinmaxEXT( GL_MINMAX, GL_FALSE, GL_BLUE, GL_UNSIGNED_INT,
			minmax );
	maxBlue = minmax[1];
	glGetMinmaxEXT( GL_MINMAX, GL_FALSE, GL_LUMMA, GL_UNSIGNED_INT,
			minmax );
	maxLumma = minmax[1];
      }

#else

    media m = uiMain->uiView->foreground();
    if (!m) return;

    CMedia* img = m->image();
    if (!img) return;


    mrv::image_type_ptr pic = img->hires();
    if (!pic) return;

    // @todo: avoid recalculating on stopped image
    //        commented out as it was preventing histogram update of renders
    //     if ( img == lastImg && frame == lastFrame )
    //       return;



    maxRed = maxGreen = maxBlue = maxLumma = 0;
    memset( red,   0, sizeof(float) * 256 );
    memset( green, 0, sizeof(float) * 256 );
    memset( blue,  0, sizeof(float) * 256 );
    memset( lumma, 0, sizeof(float) * 256 );


    int xmin, ymin, xmax, ymax;
    bool right;

    mrv::Rectd selection = uiMain->uiView->selection();

    ColorInfo::selection_to_coord( img, selection, xmin, ymin, xmax, ymax,
                                   right );

    if ( right )
    {
        CMedia::StereoType stereo_type = uiMain->uiView->stereo_type();
        if ( stereo_type == CMedia::kStereoCrossed )
            pic = img->left();
        else if ( stereo_type & CMedia::kStereoSideBySide )
            pic = img->right();
        if (!pic) return;
    }

    if ( xmin >= (int)pic->width() ) xmin = (int) pic->width()-1;
    if ( ymin >= (int)pic->height() ) ymin = (int) pic->height()-1;

    if ( xmax >= (int)pic->width() ) xmax = (int) pic->width()-1;
    if ( ymax >= (int)pic->height() ) ymax =(int)  pic->height()-1;

    unsigned int stepY = (ymax - ymin + 1) / w();
    unsigned int stepX = (xmax - xmin + 1) / h();
    if ( stepX < 1 ) stepX = 1;
    if ( stepY < 1 ) stepY = 1;
    
    mrv::DrawEngine* engine = uiMain->uiView->engine();
    ImageView::PixelValue v = (ImageView::PixelValue) 
                              uiMain->uiPixelValue->value();

    float gain = uiMain->uiView->gain();
    float gamma = uiMain->uiView->gamma();
    float one_gamma = 1.0f / gamma;

    CMedia::Pixel rp;
    uchar rgb[3];
    for ( unsigned y = ymin; y <= ymax; y += stepY )
      {
	for ( unsigned x = xmin; x <= xmax; x += stepX )
	  { 
              CMedia::Pixel op = pic->pixel( x, y );

              if ( uiMain->uiView->use_lut() && v == ImageView::kRGBA_Full )
              {
                  engine->evaluate( img, 
                                    (*(Imath::V3f*)&op), 
                                    (*(Imath::V3f*)&rp) );
               }
               else
               {
                   rp = op;
               }

              if ( v != ImageView::kRGBA_Original ) 
              {
                   if ( rp.r > 0.0f && isfinite(rp.r) )
                       rp.r = powf(rp.r * gain, one_gamma);
                   if ( rp.g > 0.0f && isfinite(rp.g) )
                       rp.g = powf(rp.g * gain, one_gamma);
                   if ( rp.b > 0.0f && isfinite(rp.b) )
                       rp.b = powf(rp.b * gain, one_gamma);
              }
              rgb[0] = (uchar)Imath::clamp(rp.r * 255.0f, 0.f, 255.f);
              rgb[1] = (uchar)Imath::clamp(rp.g * 255.0f, 0.f, 255.f);
              rgb[2] = (uchar)Imath::clamp(rp.b * 255.0f, 0.f, 255.f);
              count_pixel( rgb );
	  }
      }
#endif

  }

  float Histogram::histogram_scale( float val, float maxVal )
  {
    switch( _histtype )
      {
      case kLinear:
	return (val/maxVal);
      case kSqrt:
	return (sqrtf(1+val)/maxVal);
      case kLog:
      default:
	return (logf(1+val)/maxVal);
      }
  }

  void Histogram::draw_pixels( const fltk::Rectangle& r )
  {
    count_pixels();

    // Draw the pixel info
    int W = r.w() - 8;
    int H = r.h() - 4;
    int HH = H - 4;
    int idx;
    float v;

    float maxR, maxG, maxB, maxL;

    switch( _histtype )
      {
      case kLog:
	  maxL = logf( 1+maxLumma );
	  maxR = logf( 1+maxRed );
	  maxG = logf( 1+maxGreen );
	  maxB = logf( 1+maxBlue );
	  break;
      case kSqrt:
	  maxL = sqrtf( 1+maxLumma );
	  maxR = sqrtf( 1+maxRed );
	  maxG = sqrtf( 1+maxGreen );
	  maxB = sqrtf( 1+maxBlue );
	  break;
      default:
	  maxL = maxLumma;
	  maxR = maxRed;
	  maxG = maxGreen;
	  maxB = maxBlue;
	break;
      }


    for ( int i = 0; i <= W; ++i )
      {
	int x = i + 4;

	idx = int( ((float) i / (float) W) * 255 );
	if ( _channel == kLumma )
	  {
	    fltk::setcolor( fltk::GRAY75 );
	    v = histogram_scale( lumma[idx], maxL );
            int y = int(HH*v);
	    fltk::drawline( x, H, x, H-y );
	  }

	if ( _channel == kRed || _channel == kRGB )
	  {
	    fltk::setcolor( fltk::RED );
	    v = histogram_scale( red[idx], maxR );
	    fltk::drawline( x, H, x, H-int(HH*v) );
	  }

	if ( _channel == kGreen || _channel == kRGB )
	  {
	    fltk::setcolor( fltk::GREEN );
	    v = histogram_scale( green[idx], maxG );
            int y = int(HH*v);
	    fltk::drawline( x, H, x, H-y );
	  }

	if ( _channel == kBlue || _channel == kRGB )
	  {
	    fltk::setcolor( fltk::BLUE );
	    v = histogram_scale( blue[idx], maxB );
            int y = int(HH*v);
	    fltk::drawline( x, H, x, H-y );
	  }
      }
  }

}
