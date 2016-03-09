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


#include <FL/fl_draw.H>
// #include <FL/Symbol.h>

#include "core/mrvThread.h"
#include "core/mrvColorSpaces.h"

#include "gui/mrvIO.h"
#include "gui/mrvVectorscope.h"
#include "gui/mrvImageView.h"
#include "gui/mrvColorInfo.h"
#include "video/mrvDrawEngine.h"
#include "mrViewer.h"



#ifdef _WIN32
#define isfinite(x) _finite(x)
#endif

namespace mrv
{

  Vectorscope::Vectorscope( int x, int y, int w, int h, const char* l ) :
  Fl_Box( x, y, w, h, l ),
  uiMain( NULL )
  {
      color( FL_BLACK );
      // @todo: fltk1.3
      // buttoncolor( FL_BLACK );
  }


void Vectorscope::draw_grid(const mrv::Recti& r)
{
    int i;
    int W = diameter_/2;
    int H = diameter_/2;

    fl_color( FL_BLACK );

    mrv::Recti r2( diameter_, diameter_ );

    int W2 = r.w() / 2;
    int H2 = r.h() / 2;
    int R2 = diameter_ / 2;

    fl_line( W2, H2-R2, W2, H2+R2 );
    fl_line( W2-R2, H2, W2+R2, H2 );

    int angle = 32;
    for ( i = 0; i < 4; ++i, angle += 90 )
      {
	fl_push_matrix();
        fl_translate( W2, H2 );
	fl_rotate(angle);
        fl_begin_line();
        fl_vertex( 0, 4 );
        fl_vertex( 0, R2 );
        fl_end_line();
	fl_pop_matrix();
      }

    fl_circle( W2, H2, R2 );


    int RW  = int( diameter_ * 0.05f );
    int RH  = RW;

    fl_push_matrix();
    fl_translate( W2, H2 );

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
	fl_push_matrix();
	fl_rotate(angle);
	fl_translate( 0, int(W * 0.75f) );

        fl_begin_line();
	fl_vertex( -RW, -RH );
        fl_vertex( RW, -RH );
	fl_vertex( RW, -RH );
        fl_vertex( RW, RH );
        fl_vertex( -RW,  RH );
        fl_vertex( RW, RH );
        fl_vertex( -RW,  RH );
        fl_vertex( -RW, -RH );
        fl_end_line();

	fl_translate( 0, int(W * 0.15f) );

	fl_draw(names[i], 1, 0, 0);

	fl_pop_matrix();

      }
    fl_pop_matrix();


  }

  void Vectorscope::draw()
  {
    mrv::Recti r( w(), h() );
    draw_box();

    diameter_ = h();
    if ( w() < diameter_ ) diameter_ = w();
    diameter_ = int(diameter_ * 0.9f );

    draw_grid(r);
    draw_pixels(r);
  }




void Vectorscope::draw_pixel( const mrv::Recti& r,
                              const CMedia::Pixel& rgb,
                              const CMedia::Pixel& hsv )
{
    fl_color( (unsigned char)(rgb.r * 255), 
              (unsigned char)(rgb.g * 255), 
              (unsigned char)(rgb.b * 255) );
      
    fl_push_matrix();
    fl_translate( r.w()/2, r.h()/2 );
    fl_rotate( -165.0f + hsv.r * 360.0f );
    fl_scale( hsv.g * 0.375f );
    fl_begin_line();
    fl_vertex( 0, diameter_);
    fl_vertex( 1, diameter_+1 );
    fl_end_line();
    fl_pop_matrix();
}

void Vectorscope::draw_pixels( const mrv::Recti& r )
{
    if ( !uiMain ) return;

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

    mrv::DrawEngine* engine = uiMain->uiView->engine();

    ImageView::PixelValue v = (ImageView::PixelValue) 
                              uiMain->uiPixelValue->value();

    float gain = uiMain->uiView->gain();
    float gamma = uiMain->uiView->gamma();
    float one_gamma = 1.0f / gamma;

    CMedia::Pixel rp;
    for ( unsigned y = ymin; y <= ymax; y += stepY )
      {
	for ( unsigned x = xmin; x <= xmax; x += stepX )
	  {
	    const CMedia::Pixel& op = pic->pixel( x, y );

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

	    CMedia::Pixel hsv = color::rgb::to_hsv( rp );
	    draw_pixel( r, rp, hsv );
	  }
      }

}  // draw_pixels

}
