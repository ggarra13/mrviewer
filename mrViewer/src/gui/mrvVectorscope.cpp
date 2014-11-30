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


#include <fltk/draw.h>
#include <fltk/Symbol.h>

#include "core/mrvThread.h"
#include "core/mrvColorSpaces.h"

#include "gui/mrvIO.h"
#include "gui/mrvVectorscope.h"
#include "gui/mrvImageView.h"
#include "gui/mrvColorInfo.h"
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
				const CMedia::Pixel& rgb,
				const CMedia::Pixel& hsv )
  {
    fltk::setcolor( fltk::color( (unsigned char)(rgb.r * 255), 
				 (unsigned char)(rgb.g * 255), 
				 (unsigned char)(rgb.b * 255) ) );

    fltk::push_matrix();
    fltk::translate( r.w()/2, r.h()/2 );
    fltk::rotate( -165.0f + hsv.r * 360.0f );
    fltk::scale( hsv.g * 0.375f );
    fltk::drawline( 0, diameter_, 1, diameter_+1 );
    fltk::pop_matrix();
  }

  void Vectorscope::draw_pixels( const fltk::Rectangle& r )
  {
    mrv::media m = uiMain->uiView->foreground();
    if (!m) return;

    CMedia* img = m->image();
    mrv::image_type_ptr pic = img->hires();
    if ( !pic ) return;


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

    if ( xmin >= pic->width() ) xmin = pic->width()-1;
    if ( ymin >= pic->height() ) ymin = pic->height()-1;

    if ( xmax >= pic->width() ) xmax = pic->width()-1;
    if ( ymax >= pic->height() ) ymax = pic->height()-1;


    unsigned stepX = (xmax - xmin + 1) / w();
    unsigned stepY = (ymax - ymin + 1) / h();
    if ( stepX < 1 ) stepX = 1;
    if ( stepY < 1 ) stepY = 1;

    assert( xmax < pic->width() ); 
    assert( ymax < pic->height() ); 

    for ( unsigned y = ymin; y <= ymax; y += stepY )
      {
	for ( unsigned x = xmin; x <= xmax; x += stepX )
	  {
	    const CMedia::Pixel& p = pic->pixel( x, y );
	    CMedia::Pixel hsv = color::rgb::to_hsv( p );
	    draw_pixel( r, p, hsv );
	  }
      }

  }

}
