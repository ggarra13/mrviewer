/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

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
Fl_Box( x, y, w, h, l )
{
    color( FL_BLACK );
    //    buttoncolor( FL_BLACK );
    tooltip( _("Mark an area in the image with the left mouse button") );
}


void Vectorscope::draw_grid(const mrv::Recti& r)
{
    int i;
    int W = diameter_/2;
    int H = diameter_/2;

    fl_color( FL_GRAY0 );

    mrv::Recti r2( diameter_, diameter_ );

    fl_push_matrix();
    fl_translate( r.w()/2 - W, r.h()/2 - H );
    fl_line( W, 0, W, diameter_ );
    fl_line( 0, H, diameter_, H );
    fl_pop_matrix();

    int W2 = r.w() / 2;
    int H2 = r.h() / 2;
    int R2 = diameter_ / 2;
    float angle = 32;
    for ( i = 0; i < 4; ++i, angle += 90 )
    {
        fl_push_matrix();
        fl_translate( W2, H2 );
        fl_rotate(angle);
        fl_line( 0, 4, 0, R2 );
        fl_pop_matrix();
    }

    fl_push_matrix();
    fl_translate( r.w()/2 - W, r.h()/2 - H );
    fl_begin_line();
    fl_arc( r2.x(), r2.y(), r2.w(), r2.h(), 0, 360 ); // @TODO: fltk1.4 fl_chord
    fl_end_line();
    fl_pop_matrix();


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

        fl_line( -RW, -RH, RW, -RH );
        fl_line( RW, -RH, RW, RH );
        fl_line( -RW,  RH, RW, RH );
        fl_line( -RW,  RH, -RW, -RH );

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
    fl_color( fl_rgb_color( (unsigned char)(rgb.r * 255),
			    (unsigned char)(rgb.g * 255),
			    (unsigned char)(rgb.b * 255) ) );

    fl_push_matrix();
    fl_translate( r.w()/2, r.h()/2 );
    fl_rotate( -165.0f + hsv.r * 360.0f );
    fl_scale( hsv.g * 0.375f );
    fl_line( 0, diameter_, 1, diameter_+1 );
    fl_pop_matrix();
}

void Vectorscope::draw_pixels( const mrv::Recti& r )
{
    mrv::media m = uiMain->uiView->foreground();
    if (!m) {
        tooltip( _("Mark an area in the image with the left mouse button") );
        return;
    }
    CMedia* img = m->image();
    mrv::image_type_ptr pic = img->hires();
    if ( !pic ) return;

    tooltip( NULL );

    int xmin, ymin, xmax, ymax;
    bool right, bottom;
    mrv::Rectd selection = uiMain->uiView->selection();

    ColorInfo::selection_to_coord( img, selection, xmin, ymin, xmax, ymax,
                                   right, bottom );

    if ( right )
    {
        CMedia::StereoOutput stereo_output = uiMain->uiView->stereo_output();
        if ( stereo_output == CMedia::kStereoCrossed )
            pic = img->left();
        else if ( stereo_output & CMedia::kStereoSideBySide )
            pic = img->right();
        if (!pic) return;
    }
    else if ( bottom )
    {
        CMedia::StereoOutput stereo_output = uiMain->uiView->stereo_output();
        if ( stereo_output == CMedia::kStereoBottomTop )
            pic = img->left();
        else if ( stereo_output & CMedia::kStereoTopBottom )
            pic = img->right();
        if (!pic) return;
    }
    if ( xmin >= (int)pic->width() ) xmin = (int)pic->width()-1;
    if ( ymin >= (int)pic->height() ) ymin = (int)pic->height()-1;

    if ( xmax >= (int)pic->width() ) xmax = (int)pic->width()-1;
    if ( ymax >= (int)pic->height() ) ymax = (int)pic->height()-1;



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
    for ( unsigned y = ymin; y <= (unsigned)ymax; y += stepY )
    {
        for ( unsigned x = xmin; x <= (unsigned)xmax; x += stepX )
        {
            CMedia::Pixel op = pic->pixel( x, y );

            if ( uiMain->uiView->normalize() )
            {
                uiMain->uiView->normalize( op );
            }

            op.r *= gain;
            op.g *= gain;
            op.b *= gain;

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

}

}
