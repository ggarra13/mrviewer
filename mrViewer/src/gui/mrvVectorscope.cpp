

#include <fltk/draw.h>
#include <fltk/Symbol.h>

#include "core/mrvThread.h"
#include "core/mrvColorSpaces.h"

#include "mrvIO.h"
#include "mrvVectorscope.h"
#include "mrvImageView.h"
#include "mrViewer.h"



namespace mrv
{

  Vectorscope::Vectorscope( int x, int y, int w, int h, const char* l ) :
    fltk::Widget( x, y, w, h, l )
  {
    color( fltk::BLACK );
    buttoncolor( fltk::BLACK );
  }


  void Vectorscope::draw_grid(const fltk::Rectangle& r)
  {
    int i;
    int W = diameter_/2;
    int H = diameter_/2;

    fltk::setcolor( fltk::GRAY75 );

    fltk::Rectangle r2( diameter_, diameter_ );

    fltk::push_matrix();
    fltk::translate( r.w()/2 - W, r.h()/2 - H );
    fltk::drawline( W, 0, W, diameter_ );
    fltk::drawline( 0, H, diameter_, H );
    fltk::pop_matrix();

    int W2 = r.w() / 2;
    int H2 = r.h() / 2;
    int R2 = diameter_ / 2;
    int angle = 32;
    for ( i = 0; i < 4; ++i, angle += 90 )
      {
	fltk::push_matrix();
	fltk::translate( W2, H2 );
	fltk::rotate(angle);
	fltk::drawline( 0, 4, 0, R2 );
	fltk::pop_matrix();
      }

    fltk::push_matrix();
    fltk::translate( r.w()/2 - W, r.h()/2 - H );
    fltk::addchord( r2, 0, 360 );
    fltk::strokepath();
    fltk::pop_matrix();


    int RW  = int( diameter_ * 0.05f );
    int RH  = RW;

    fltk::push_matrix();
    fltk::translate( W2, H2 );

    static const char* names[] = {
      "C",
      "B",
      "M",
      "R",
      "Y",
      "G"
    };

    angle = 15;
    for ( i = 0; i < 6; ++i, angle += 60 )
      {
	fltk::push_matrix();
	fltk::rotate(angle);
	fltk::translate( 0, int(W * 0.75f) );

	fltk::drawline( -RW, -RH, RW, -RH ); 
	fltk::drawline( RW, -RH, RW, RH ); 
	fltk::drawline( -RW,  RH, RW, RH ); 
	fltk::drawline( -RW,  RH, -RW, -RH );

	fltk::translate( 0, int(W * 0.15f) );

	fltk::drawtext(names[i], 1, 0, 0);

	fltk::pop_matrix();

      }
    fltk::pop_matrix();


  }

  void Vectorscope::draw()
  {
    fltk::Rectangle r( w(), h() );
    draw_box(r);

    diameter_ = h();
    if ( w() < diameter_ ) diameter_ = w();
    diameter_ = int(diameter_ * 0.9f );

    draw_grid(r);
    draw_pixels(r);
  }




  void Vectorscope::draw_pixel( const fltk::Rectangle& r,
				const CMedia::PixelType& rgb,
				const CMedia::PixelType& hsv )
  {
    fltk::setcolor( fltk::color( (unsigned char)(rgb.r * 255), 
				 (unsigned char)(rgb.g * 255), 
				 (unsigned char)(rgb.b * 255) ) );

    fltk::push_matrix();
    fltk::translate( r.w()/2, r.h()/2 );
    fltk::rotate( -165.0f + hsv.r * 360.0f );
    fltk::scale( hsv.g * 0.375f );
    fltk::drawline( 0, diameter_, 0, diameter_ );
    fltk::pop_matrix();
  }

  void Vectorscope::draw_pixels( const fltk::Rectangle& r )
  {
    mrv::media m = uiMain->uiView->foreground();
    if (!m) return;

    CMedia* img = m->image();
    mrv::image_type_ptr pic;
    {
      CMedia::Mutex& m = img->video_mutex();
      SCOPED_LOCK(m);
      pic = img->hires();
    }
    if ( !pic ) return;

    unsigned int xmax = img->width();
    unsigned int ymax = img->height();
    unsigned int xmin = 0;
    unsigned int ymin = 0;

    mrv::Rectd selection = uiMain->uiView->selection();
    if ( selection.w() > 0 || selection.h() < 0 )
      {
	xmin = (unsigned int)(xmax * selection.x());
	ymin = (unsigned int)(ymax * selection.y());

	xmax = xmin + (unsigned int)(xmax  * selection.w());
	ymax = ymin + (unsigned int)(ymax  * selection.h());
      }

    assert( xmin <= img->width() );
    assert( ymin <= img->height() );
    assert( xmax <= img->width() );
    assert( ymax <= img->height() );

    unsigned stepX = (xmax - xmin) / w();
    unsigned stepY = (ymax - ymin) / h();
    if ( stepX < 1 ) stepX = 1;
    if ( stepY < 1 ) stepY = 1;

    
    for ( unsigned y = ymin; y < ymax; y += stepY )
      {
	for ( unsigned x = xmin; x < xmax; x += stepX )
	  {
	    const CMedia::PixelType& p = pic->pixel( x, y );
	    CMedia::PixelType hsv = color::rgb::to_hsv( p );
	    draw_pixel( r, p, hsv );
	  }
      }

//     fltk::line_style( fltk::SOLID, 1 );
  }

}
