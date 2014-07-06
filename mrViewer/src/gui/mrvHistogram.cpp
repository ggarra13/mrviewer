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

    if ( xmax >= pic->width() ) xmax = pic->width()-1;
    if ( ymax >= pic->height() ) ymax = pic->height()-1;

    unsigned int stepY = (ymax - ymin) / w();
    unsigned int stepX = (xmax - xmin) / h();
    if ( stepX < 1 ) stepX = 1;
    if ( stepY < 1 ) stepY = 1;
    

    uchar rgb[3];
    for ( unsigned y = ymin; y < ymax; y += stepY )
      {
	for ( unsigned x = xmin; x < xmax; x += stepX )
	  {
	    const CMedia::Pixel& p = pic->pixel( x, y );
	    rgb[0] = (uchar)Imath::clamp(p.r * 255.0f, 0.f, 255.f);
	    rgb[1] = (uchar)Imath::clamp(p.g * 255.0f, 0.f, 255.f);
	    rgb[2] = (uchar)Imath::clamp(p.b * 255.0f, 0.f, 255.f);
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
	return (sqrt(1+val)/maxVal);
      case kLog:
      default:
	return (log(1+val)/maxVal);
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
	  maxL = log( 1+maxLumma );
	  maxR = log( 1+maxRed );
	  maxG = log( 1+maxGreen );
	  maxB = log( 1+maxBlue );
	  break;
      case kSqrt:
	  maxL = sqrt( 1+maxLumma );
	  maxR = sqrt( 1+maxRed );
	  maxG = sqrt( 1+maxGreen );
	  maxB = sqrt( 1+maxBlue );
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
	    fltk::drawline( x, H, x, H-int(HH*v) );
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
	    fltk::drawline( x, H, x, H-int(HH*v) );
	  }

	if ( _channel == kBlue || _channel == kRGB )
	  {
	    fltk::setcolor( fltk::BLUE );
	    v = histogram_scale( blue[idx], maxB );
	    fltk::drawline( x, H, x, H-int(HH*v) );
	  }
      }
  }

}
