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
 * @file   ColorBarsImage.cpp
 * @author gga
 * @date   Thu Jul  5 18:37:58 2007
 * 
 * @brief  A simple class to create some built-in images, like
 *         a COLORBARS chart and gamma charts.
 * 
 * 
 */


#include "mrvColorBarsImage.h"

namespace mrv {


  ColorBarsImage::ColorBarsImage( const ColorBarsImage::Type c ) :
    CMedia()
  {
    _gamma = 1.0f;
    _internal = true;
    default_layers();

    _frameStart = _frame_start = 1;

    switch( c )
      {
      case kSMPTE_NTSC:
	 _fileroot = strdup( "SMPTE NTSC Color Bars" );
	image_size( 720, 480 );
	allocate_pixels(_frameStart);
	_pixel_ratio = 0.9f;
	_fps = 29.976f;
	NTSC_color_bars();
	break;
      case kPAL:
	 _fileroot = strdup( "PAL Color Bars" );
	image_size( 720, 576 );
	allocate_pixels(_frameStart);
	_fps = 25.0f;
	_pixel_ratio = 1.25f;
	NTSC_color_bars();
	break;
      case kPAL_HDTV:
	 _fileroot = strdup( "PAL HDTV Color Bars" );
	image_size( 1920, 1080 );
	allocate_pixels(_frameStart);
	_pixel_ratio = 1.0;
	_fps = 25.0f;
	NTSC_HDTV_color_bars();
	break;
      default:
      case kSMPTE_NTSC_HDTV:
	 _fileroot = strdup( "NTSC HDTV Color Bars" );
	image_size( 1920, 1080 );
	allocate_pixels(_frameStart);
	_pixel_ratio = 1.0;
	_fps = 29.976f;
	NTSC_HDTV_color_bars();
	break;
      }

    _frameEnd = _frame_end = int64_t( (_fps * 3) + 0.5f );
  }

  void ColorBarsImage::smpte_color_bars( 
					const unsigned int X,
					const unsigned int W,
					const unsigned int H,
					const float pct
					 )
  {
    unsigned int WW = width();
    unsigned int bar_w = unsigned(W / 7.0f + 0.5f);
    unsigned int bar_h = unsigned(H * pct + 0.5f);

    float hi    = 0.745098f;
    float lo    = 0.0f;
    static Pixel bars[] = {
      Pixel( hi, hi, hi ), // gray
      Pixel( hi, hi, lo ), // yellow
      Pixel( lo, hi, hi ), // cyan
      Pixel( lo, hi, lo ), // green
      Pixel( hi, lo, hi ), // magenta
      Pixel( hi, lo, lo ), // red
      Pixel( lo, lo, hi ), // blue
    };

    Pixel* pixels = (Pixel*)_hires->data().get();

    // Draw top bars
    for ( unsigned int y = 0; y < bar_h; ++y )
      {
	unsigned int offset = X + y * WW;
	Pixel* p = pixels + offset;
	for (unsigned int b = 0; b < 7; ++b )
	  {
	    for ( unsigned int c = 0; c < bar_w; ++c, ++p )
	      {
		*p = bars[b];
	      }
	  }
      }


  }

  void ColorBarsImage::smpte_bottom_bars( const unsigned int X,
					  const unsigned int Y,
					  const unsigned int W,
					  const unsigned int H )
  {
    unsigned int bbar_h = H - Y; //unsigned( H / 3 ) - mbar_h;
    unsigned int bbar_w = unsigned( W / 6 );
    unsigned int WW = width();
    float black = 0.0745098f;
    float superblack = 0.03529f;


    // bottom bars
    static Pixel bbars[] = {
      Pixel( 0.0f, 0.12941f, 0.29804f ), // blue-cyanish
      Pixel( 1.0f, 1.0f, 1.0f ),         // white
      Pixel( 0.19608f, 0.0f, 0.41569f ), // blue-reddish
      Pixel( black, black, black ),      // black
      Pixel( superblack, superblack, superblack ), // superblack
      Pixel( black, black, black ),      // black
    };

    Pixel* pixels = (Pixel*)_hires->data().get();

    // Draw bottom bars
    for ( unsigned int y = 0; y < bbar_h; ++y )
      {
	unsigned int offset = X + ( Y + y ) * WW;
	Pixel* p = pixels + offset;
	for (unsigned int b = 0; b < 6; ++b )
	  {
	    for ( unsigned int c = 0; c < bbar_w; ++c, ++p )
	      {
		*p = bbars[b];
	      }
	  }
      }

    // Draw super-black rectangle
    static Pixel sbbars[] = {
      Pixel( black, black, black ), // black
      Pixel( 0.11373f, 0.11373f, 0.11373f ), // +1 black
    };

    for ( unsigned int y = 0; y < bbar_h; ++y )
      {
	unsigned int offset = X + ( Y + y ) * WW + 4 * bbar_w + 40;
	Pixel* p = pixels + offset;
	for (unsigned int b = 0; b < 2; ++b )
	  {
	    for ( unsigned int c = 0; c < 40; ++c, ++p )
	      {
		*p = sbbars[b];
	      }
	  }
      }
  }

  void ColorBarsImage::NTSC_color_bars()
  {
    unsigned int X  = 0;
    unsigned int W  = width();
    unsigned int H  = height();
    float pct = 2.0f/3.0f;

    smpte_color_bars( X, W, H, pct );

    float hi    = 0.745098f;
    float lo    = 0.0f;
    float black = 0.0745098f;

    unsigned int WW = width();
    unsigned int bar_w = unsigned(W / 7.0f + 0.5f);
    unsigned int bar_h = unsigned(H * pct + 0.5f);

    // middle bars
    static Pixel mbars[] = {
      Pixel( lo, lo, hi ), // blue
      Pixel( black, black, black ), // black
      Pixel( hi, lo, hi ), // magenta
      Pixel( black, black, black ), // black
      Pixel( lo, hi, hi ), // cyan
      Pixel( black, black, black ), // black
      Pixel( hi, hi, hi ), // gray
    };

    // Draw middle bars
    Pixel* pixels = (Pixel*)_hires->data().get();
    unsigned int mbar_h = unsigned( H / 12 );
    for ( unsigned int y = 0; y < mbar_h; ++y )
      {
	unsigned int offset = X + (bar_h + y) * WW;
	Pixel* p = pixels + offset;
	for (unsigned int b = 0; b < 7; ++b )
	  {
	    for ( unsigned int c = 0; c < bar_w; ++c, ++p )
	      {
		*p = mbars[b];
	      }
	  }
      }

    unsigned int Y = bar_h + mbar_h;
    smpte_bottom_bars( X, Y, W, H );

  }

  void ColorBarsImage::NTSC_HDTV_color_bars()
  {
    unsigned int X = unsigned( width() * 0.125f );
    unsigned int W = width() - X * 2;
    unsigned int H = height();
    float pct = 0.58f;

    smpte_color_bars( X, W, H, pct );

    unsigned int WW = width();
    unsigned int bar_h = unsigned(H * pct + 0.5f);

    float midgray = 0.41176f;
    float drkgray = 0.16863f;

    static Pixel gbars[] = {
      Pixel( midgray, midgray, midgray ), // midgray
    };

    // Draw mid gray rectangles
    Pixel* pixels = (Pixel*)_hires->data().get();
    for ( unsigned int y = 0; y < bar_h; ++y )
      {
	unsigned int offset = y * WW;
	Pixel* p = pixels + offset;
	for ( unsigned int x = 0; x < X; ++x, ++p )
	  {
	    *p = gbars[0];
	  }

	offset = (WW - X) + y * WW;
	p = pixels + offset;
	for ( unsigned int x = 0; x < X; ++x, ++p )
	  {
	    *p = gbars[0];
	  }
      }


    float black = 0.01961f;
    float superblack = 0.0f;


    // middle bars
    static Pixel mbars1[] = {
      Pixel( 0, 1.0f, 1.0f ), // cyan
      Pixel( black, 0.14648f, 0.31180f ),      // dark blue
      Pixel( 0.75686f, 0.75686f, 0.75686f ),         // white
      Pixel( 0, 0, 1.0f ),      // blue
    };

    static Pixel mbars2[] = {
      Pixel( 1.0f, 1.0f, 0.0f ), // yellow
      Pixel( 0.21184f, black, 0.42714f ),      // dark violet
      Pixel( black, black, black ),      // black
      Pixel( 1.0f, 0.0f, 0.0f ),      // red
    };

    // middle bars
    static unsigned mbars_w[] = {
      X, 
      W / 7,
      W - W / 7,
      X,
    };

    // Draw middle bars
    unsigned Y = bar_h;
    unsigned int mbar_h = unsigned( (H - bar_h - unsigned(H * 0.25f)) * 0.5f);
    for ( unsigned int y = 0; y < mbar_h; ++y )
      {
	unsigned int offset = (Y + y) * WW;
	Pixel* p = pixels + offset;
	for (unsigned int b = 0; b < 4; ++b )
	  {
	    for ( unsigned int c = 0; c < mbars_w[b]; ++c, ++p )
	      {
		*p = mbars1[b];
	      }
	  }
      }

    Y = bar_h + mbar_h;
    for ( unsigned int y = 0; y < mbar_h; ++y )
      {
	unsigned int offset = (Y + y) * WW;
	Pixel* p = pixels + offset;
	for (unsigned int b = 0; b < 4; ++b )
	  {
	    for ( unsigned int c = 0; c < mbars_w[b]; ++c, ++p )
	      {
		*p = mbars2[b];
	      }
	  }
      }

    ////////////////
    // bottom bars
    ////////////////
    static Pixel bbars[] = {
      Pixel( drkgray, drkgray, drkgray ), // drkgray
      Pixel( black, black, black ),      // black
      Pixel( 1.0f, 1.0f, 1.0f ),         // white
      Pixel( black, black, black ),      // black
      Pixel( superblack, superblack, superblack ), // superblack
      Pixel( black, black, black ),      // black
      Pixel( 0.03922f, 0.03922f, 0.03922f ), // light black
      Pixel( black, black, black ),      // black
      Pixel( 0.0598f, 0.0598f, 0.0598f ), // super light black
      Pixel( black, black, black ),       // black
      Pixel( drkgray, drkgray, drkgray ), // drkgray
    };

    static unsigned bbar_w[] = {
      X, 
      unsigned( W * 0.2 ),
      unsigned( W * 0.3 ),
      unsigned( W / 7 ),
      unsigned( W * 0.043 ),
      unsigned( W * 0.043 ),
      unsigned( W * 0.045 ),
      unsigned( W * 0.043 ),
      unsigned( W * 0.044 ),
      unsigned( W / 7 ),
      X,
    };

    // Draw bottom bars
    unsigned int bbar_h = unsigned( H * 0.25f );
    Y = H - bbar_h;
    for ( unsigned int y = 0; y < bbar_h; ++y )
      {
	unsigned int offset = (Y + y) * WW;
	Pixel* p = pixels + offset;
	for (unsigned int b = 0; b < 11; ++b )
	  {
	    for ( unsigned int c = 0; c < bbar_w[b]; ++c, ++p )
	      {
		*p = bbars[b];
	      }
	  }
      }

  }


  bool ColorBarsImage::fetch( const boost::int64_t frame )
  {
    return true;
  }

} // namespace mrv
