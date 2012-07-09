/**
 * @file   smpteImage.cpp
 * @author gga
 * @date   Thu Jul  5 18:37:58 2007
 * 
 * @brief  A simple class to create some built-in images, like
 *         a SMPTE chart and gamma charts.
 * 
 * 
 */


#include "smpteImage.h"
#include "mrvIO.h"

namespace 
{
  const char* kModule = "slate";
}

namespace mrv {


  smpteImage::smpteImage( const smpteImage::Type c, const unsigned int dw,
			  const unsigned int dh ) :
    CMedia(),
    type_(c)
  {
    _fileroot = "internal";
    _internal = true;
    _gamma = 1.0f;
    default_layers();
    image_size( dw, dh );
    allocate_pixels(1);
  }


  void smpteImage::gamma_box( unsigned int X, unsigned int Y, 
			      unsigned int W, unsigned int H )
  {
    // Draw a line of dots
    PixelType c = bg;
    if ( Y % 2 == 0 ) c = fg;

    PixelType* pixels = (PixelType*)_hires->data().get();
    PixelType* p = pixels + Y * width() + X+1;
    for ( unsigned int x = X+1; x < X+W-1; x += 2, p += 2 )
      {
	*p = c;
      }

    if ( (Y+1) % 2 == 0 ) c = fg;
    else c = bg;
 
    // Draw alternating lines
    unsigned int y;
    for ( y = Y+1; y < Y+H-1; ++y )
      {
	p = pixels + y * width() + X;

	if ( y % 2 == 0 ) c = fg;
	else c = bg;

	for ( unsigned int x = X; x < X+W; ++x, ++p )
	  {
	    *p = c;
	  }
      }

    if ( y % 2 == 0 ) c = fg;
    else c = bg;

    // Draw a line of dots
    p = pixels + (Y+H-1) * width() + X;
    for ( unsigned int x = X; x < X+W; x += 2, p += 2 )
      {
	*p = c;
      } 
  }

  void smpteImage::gamma_boxes( unsigned int sx,
				unsigned int sy, 
				unsigned int W,
				unsigned int H,
				float bgc, float fgc )
  {
    // Fill image with background color
    PixelType* pixels = (PixelType*)_hires->data().get();
    for ( unsigned int y = sy; y < sy+H; ++y )
      {
	PixelType* p = pixels + y * width() + sx;
	for ( unsigned int x = 0; x < W; ++x, ++p )
	  {
	    p->r = p->g = p->b = bgc;
	    p->a = 1.0f;
	  }
      }

    unsigned int bw = unsigned(W * 0.2f);
    unsigned int bh = unsigned(H * 0.5904761904f);
    unsigned int bs = unsigned(W * 0.016393442f);

    unsigned int ibw = unsigned(bw * 0.5f);
    unsigned int ibh = unsigned(bh * 0.5f);
    unsigned int ibx = unsigned(bw * 0.25f);
    unsigned int iby = unsigned(bh * 0.25f);

    // Draw box #1
    unsigned int bx = sx + unsigned(W * 0.065573f);
    unsigned int by = sy + unsigned(H * 0.1904761904f);

    bg = PixelType(fgc, bgc, bgc);
    fg = PixelType(0.0f, bgc, bgc);
    gamma_box( bx, by, bw, bh );

    bg = PixelType( bgc, 0.0f, fgc );
    fg = PixelType( bgc, fgc, 0.0f );
    gamma_box( bx+ibx, by+iby, ibw, ibh );

    // Draw box #2
    bx += bw + bs;
    bg = PixelType( bgc, 0.0f, bgc );
    fg = PixelType( bgc, fgc, bgc );
    gamma_box( bx, by, bw, bh );

    bg = PixelType( 0.0f, bgc, fgc );
    fg = PixelType( fgc, bgc, 0.0f );
    gamma_box( bx+ibx, by+iby, ibw, ibh );

    // Draw box #3
    bx += bw + bs;
    bg = PixelType( bgc, bgc, 0.0f );
    fg = PixelType( bgc, bgc, fgc );
    gamma_box( bx, by, bw, bh );

    bg = PixelType( fgc, 0.0f, bgc );
    fg = PixelType( 0.0f, fgc, bgc );
    gamma_box( bx+ibx, by+iby, ibw, ibh );

    // Draw box #4
    bx += bw + bs;
    bg = PixelType( 0.0f, 0.0f, 0.0f );
    fg = PixelType( fgc, fgc, fgc );
    gamma_box( bx, by, bw, bh );
  }

  void smpteImage::gamma_chart()
  {
    char buf[1024];
    sprintf( buf, "Gamma %0.1f Chart", _gamma );
    _fileroot = strdup(buf);

    unsigned int y = 0;
    unsigned int H = height() / 3;
    unsigned int W = width();

    float b, f;

    // gamma1.4
    if ( _gamma == 1.4 )
      {
	b = 0.13725f;
	f = 0.22745f;
	gamma_boxes( 0, y, W, H, b, f );

	b = 0.37255f;
	f = 0.61176f;
	gamma_boxes( 0, y+H, W, H, b, f );

	b = 0.61176f;
	f = 1.0f;
	gamma_boxes( 0, y+2*H, W, H, b, f );
      }
    else if ( _gamma == 1.8f )
      {
	// gamma1.8
	b = 0.21569f;
	f = 0.31373f;
	gamma_boxes( 0, y, W, H, b, f );

	b = 0.46275f;
	f = 0.68235f;
	gamma_boxes( 0, y+H, W, H, b, f );

	b = 0.68235f;
	f = 1.0f;
	gamma_boxes( 0, y+2*H, W, H, b, f );
      }
    else if ( _gamma == 2.4f )
      {
	// gamma2.4
	b = 0.31373f;
	f = 0.41961f;
	gamma_boxes( 0, y, W, H, b, f );

	b = 0.56078f;
	f = 0.74902f;
	gamma_boxes( 0, y+H, W, H, b, f );

	b = 0.74902f;
	f = 1.0f;
	gamma_boxes( 0, y+2*H, W, H, b, f );
      }
    else
      {
	// gamma2.2
	b = 0.28235f;
	f = 0.38824f;
	gamma_boxes( 0, y, W, H, b, f );

	b = 0.53333f;
	f = 0.72941f;
	gamma_boxes( 0, y+H, W, H, b, f );

	b = 0.72941f;
	f = 1.0f;
	gamma_boxes( 0, y+2*H, W, H, b, f );
      }

  }

  void smpteImage::linear_gradient()
  {
    _fileroot = strdup( "Linear Gradient" );

    unsigned int W = width();
    unsigned int H = height();

    PixelType* pixels = (PixelType*)_hires->data().get();
    for ( unsigned int x = 0; x < W; ++x )
      {
	float val = (float) x / (float) W;
	for ( unsigned int y = 0; y < H; ++y )
	  {
	    PixelType* p = pixels + y * W + x;
	    p->r = p->g = p->b = val;
	    p->a = 0;
	  }
      }
  }

  void smpteImage::luminance_gradient()
  {
    _fileroot = strdup( "Luminance Gradient" );

    unsigned int W = width();
    unsigned int H = height();

    PixelType* pixels = (PixelType*)_hires->data().get();
    for ( unsigned int x = 0; x < W; ++x )
      {
	float val = (float) x / (float) W;
	val *= val;
	for ( unsigned int y = 0; y < H; ++y )
	  {
	    PixelType* p = pixels + y * W + x;
	    p->r = p->g = p->b = val;
	    p->a = 0;
	  }
      }
  }

  void smpteImage::checkered()
  {
    _fileroot = strdup( "Checkered" );

    unsigned int W = width();
    unsigned int H = height();



    unsigned int dw = W / 20;
    unsigned int dw2 = dw * 2;
    unsigned int dh = H / 20;
    unsigned int dh2 = dh * 2;

  
    PixelType* pixels = (PixelType*)_hires->data().get();
    for ( unsigned int x = 0; x < W; x += dw )
      {
	for ( unsigned int y = 0; y < H; y += dh )
	  {
	    unsigned xx = (x+dw < W ? x+dw : W);
	    unsigned yy = (y+dh < H ? y+dh : H);

	    float val;
	    if ( y % dh2 == 0 )
	      {
		val = (x % dw2 == 0) ? 0.6f : 0.75f;
	      }
	    else
	      {
		val = (x % dw2 == 0) ? 0.75f : 0.6f;
	      }


	    for ( unsigned int dx = x; dx < xx; ++dx )
	      {
		for ( unsigned int dy = y; dy < yy; ++dy )
		  {
		    PixelType* p = pixels + dy * W + dx;
		    p->r = p->g = p->b = p->a = val;
		  }
	      }
	  }
      }

  }

  bool smpteImage::fetch( const boost::int64_t frame )
  {
    switch( type_ )
      {
      case kGammaChart:
	gamma_chart();  break;
      case kLinearGradient:
	linear_gradient(); break;
      case kLuminanceGradient:
	luminance_gradient(); break;
      case kCheckered:
	checkered();  break;
      default:
	LOG_ERROR("Internal Image: Unknown image type");
	break;
      }

    return true;
  }

} // namespace mrv
