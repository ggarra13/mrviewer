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
    _fileroot = strdup( "Color Bars" );
    _gamma = 1.0f;
    _internal = true;
    default_layers();

    _frameStart = _frame_start = 1;

    switch( c )
      {
      case kSMPTE_NTSC:
	image_size( 720, 480 );
	allocate_pixels(_frameStart);
	_pixel_ratio = 0.9f;
	_fps = 30.0f;
	NTSC_color_bars();
	break;
      case kPAL:
	image_size( 720, 576 );
	allocate_pixels(_frameStart);
	_fps = 24.0f;
	_pixel_ratio = 1.25f;
	NTSC_color_bars();
	break;
      case kPAL_HDTV:
	image_size( 1920, 1080 );
	allocate_pixels(_frameStart);
	_pixel_ratio = 1.0;
	_fps = 24.0f;
	NTSC_HDTV_color_bars();
	break;
      default:
      case kSMPTE_NTSC_HDTV:
	image_size( 1920, 1080 );
	allocate_pixels(_frameStart);
	_pixel_ratio = 1.0;
	_fps = 30.0f;
	NTSC_HDTV_color_bars();
	break;
      }

    _frameEnd = _frame_end = int64_t( _fps * 3 );
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
    static PixelType bars[] = {
      PixelType( hi, hi, hi ), // gray
      PixelType( hi, hi, lo ), // yellow
      PixelType( lo, hi, hi ), // cyan
      PixelType( lo, hi, lo ), // green
      PixelType( hi, lo, hi ), // magenta
      PixelType( hi, lo, lo ), // red
      PixelType( lo, lo, hi ), // blue
    };

    PixelType* pixels = (PixelType*)_hires->data().get();

    // Draw top bars
    for ( unsigned int y = 0; y < bar_h; ++y )
      {
	unsigned int offset = X + y * WW;
	PixelType* p = pixels + offset;
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
    static PixelType bbars[] = {
      PixelType( 0.0f, 0.12941f, 0.29804f ), // blue-cyanish
      PixelType( 1.0f, 1.0f, 1.0f ),         // white
      PixelType( 0.19608f, 0.0f, 0.41569f ), // blue-reddish
      PixelType( black, black, black ),      // black
      PixelType( superblack, superblack, superblack ), // superblack
      PixelType( black, black, black ),      // black
    };

    PixelType* pixels = (PixelType*)_hires->data().get();

    // Draw bottom bars
    for ( unsigned int y = 0; y < bbar_h; ++y )
      {
	unsigned int offset = X + ( Y + y ) * WW;
	PixelType* p = pixels + offset;
	for (unsigned int b = 0; b < 6; ++b )
	  {
	    for ( unsigned int c = 0; c < bbar_w; ++c, ++p )
	      {
		*p = bbars[b];
	      }
	  }
      }

    // Draw super-black rectangle
    static PixelType sbbars[] = {
      PixelType( black, black, black ), // black
      PixelType( 0.11373f, 0.11373f, 0.11373f ), // +1 black
    };

    for ( unsigned int y = 0; y < bbar_h; ++y )
      {
	unsigned int offset = X + ( Y + y ) * WW + 4 * bbar_w + 40;
	PixelType* p = pixels + offset;
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
    static PixelType mbars[] = {
      PixelType( lo, lo, hi ), // blue
      PixelType( black, black, black ), // black
      PixelType( hi, lo, hi ), // magenta
      PixelType( black, black, black ), // black
      PixelType( lo, hi, hi ), // cyan
      PixelType( black, black, black ), // black
      PixelType( hi, hi, hi ), // gray
    };

    // Draw middle bars
    PixelType* pixels = (PixelType*)_hires->data().get();
    unsigned int mbar_h = unsigned( H / 12 );
    for ( unsigned int y = 0; y < mbar_h; ++y )
      {
	unsigned int offset = X + (bar_h + y) * WW;
	PixelType* p = pixels + offset;
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

    static PixelType gbars[] = {
      PixelType( midgray, midgray, midgray ), // midgray
    };

    // Draw mid gray rectangles
    PixelType* pixels = (PixelType*)_hires->data().get();
    for ( unsigned int y = 0; y < bar_h; ++y )
      {
	unsigned int offset = y * WW;
	PixelType* p = pixels + offset;
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
    static PixelType mbars1[] = {
      PixelType( 0, 1.0f, 1.0f ), // cyan
      PixelType( black, 0.14648f, 0.31180f ),      // dark blue
      PixelType( 0.75686f, 0.75686f, 0.75686f ),         // white
      PixelType( 0, 0, 1.0f ),      // blue
    };

    static PixelType mbars2[] = {
      PixelType( 1.0f, 1.0f, 0.0f ), // yellow
      PixelType( 0.21184f, black, 0.42714f ),      // dark violet
      PixelType( black, black, black ),      // black
      PixelType( 1.0f, 0.0f, 0.0f ),      // red
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
	PixelType* p = pixels + offset;
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
	PixelType* p = pixels + offset;
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
    static PixelType bbars[] = {
      PixelType( drkgray, drkgray, drkgray ), // drkgray
      PixelType( black, black, black ),      // black
      PixelType( 1.0f, 1.0f, 1.0f ),         // white
      PixelType( black, black, black ),      // black
      PixelType( superblack, superblack, superblack ), // superblack
      PixelType( black, black, black ),      // black
      PixelType( 0.03922f, 0.03922f, 0.03922f ), // light black
      PixelType( black, black, black ),      // black
      PixelType( 0.0598f, 0.0598f, 0.0598f ), // super light black
      PixelType( black, black, black ),       // black
      PixelType( drkgray, drkgray, drkgray ), // drkgray
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
	PixelType* p = pixels + offset;
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
