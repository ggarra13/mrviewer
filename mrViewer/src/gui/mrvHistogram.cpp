/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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

#include "core/mrvFrame.h"

#include <math.h>
#include <limits>

#ifdef _WIN32
#define isfinite(x) _finite(x)
#endif

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include "GL/glew.h"

#include "core/CMedia.h"
#include "core/mrvThread.h"
#include "core/mrvColorOps.h"

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
Fl_Gl_Window( x, y, w, h, l ),
_channel( kRGB ),
_histtype( kLog ),
maxLumma( 0 ),
maxColor( 0 )
{
}


void Histogram::draw_grid(const mrv::Recti& r)
{
    glDisable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT );
    glShadeModel( GL_FLAT );
//     fl_color( FL_GRAY0 );

//     int X = r.x() + 2;
//     int H = r.h() / 4;
//     int H2 = ( H + FL_getsize() ) / 2;
//     fl_draw( "L", 1, X, H2 );
//     fl_draw( "R", 1, X, H+H2 );
//     fl_draw( "G", 1, X, H*2+H2 );
//     fl_draw( "B", 1, X, H*3+H2 );
}

void Histogram::draw()
{

    if ( !valid() )
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport( 0, 0, pixel_w(), pixel_h() );
        glOrtho( 0, w(), 0, h(), -1, 1 );

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        valid(1);
    }


#ifdef OSX
    mrv::Recti r( 0, 0, pixel_w()/2, pixel_h()/2 );
#else
    mrv::Recti r( 0, 0, pixel_w(), pixel_h() );
#endif
    // draw_box();



    draw_grid(r);
    draw_pixels(r);
}


void Histogram::count_pixel( const uchar* rgb )
{
    uchar r = rgb[0];
    uchar g = rgb[1];
    uchar b = rgb[2];

    ++red[ r ];
    if ( red[r] > maxColor ) maxColor = red[r];

    ++green[ g ];
    if ( green[g] > maxColor ) maxColor = green[g];

    ++blue[ b ];
    if ( blue[b] > maxColor ) maxColor = blue[b];

    unsigned int lum = unsigned(r * 0.30f + g * 0.59f + b * 0.11f);
    lumma[ lum ] += 1;
    if ( lumma[lum] > maxLumma ) maxLumma = lumma[lum];
}


void Histogram::count_pixels()
{
    media m = uiMain->uiView->foreground();
    if (!m) {
        tooltip( _("Mark an area in the image with the left mouse button") );
        return;
    }

    CMedia* img = m->image();
    if (!img) {
        return;
    }

    mrv::image_type_ptr pic = img->left();
    if (!pic) return;

    tooltip( NULL );

    // @todo: avoid recalculating on stopped image
    //        commented out as it was preventing histogram update of renders
    //     if ( img == lastImg && frame == lastFrame )
    //       return;



    maxColor = maxLumma = 0;
    memset( red,   0, sizeof(float) * 256 );
    memset( green, 0, sizeof(float) * 256 );
    memset( blue,  0, sizeof(float) * 256 );
    memset( lumma, 0, sizeof(float) * 256 );


    int off[2];
    int xmin, ymin, xmax, ymax;
    bool right, bottom;

    mrv::Rectd selection = uiMain->uiView->selection();

    ColorInfo::selection_to_coord( img, selection, xmin, ymin, xmax, ymax,
                                   off, right, bottom );

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

    // Check if xmin/ymin is below 0 is done in selection_to_coord

    if ( xmin >= (int)pic->width() ) xmin = (int) pic->width()-1;
    if ( ymin >= (int)pic->height() ) ymin = (int) pic->height()-1;

    if ( xmax >= (int)pic->width() ) xmax = (int) pic->width()-1;
    if ( ymax >= (int)pic->height() ) ymax =(int)  pic->height()-1;


    unsigned int stepY = (ymax - ymin + 1) / w();
    unsigned int stepX = (xmax - xmin + 1) / h();
    if ( stepX < 1 ) stepX = 1;
    if ( stepY < 1 ) stepY = 1;

    ImageView* view = uiMain->uiView;
    mrv::DrawEngine* engine = view->engine();
    ImageView::PixelValue v = (ImageView::PixelValue)
                              uiMain->uiPixelValue->value();

    float gain = view->gain();
    float gamma = view->gamma();
    float one_gamma = 1.0f / gamma;

    CMedia::Pixel rp;
    uchar rgb[3];
    for ( int y = ymin; y <= ymax; y += stepY )
    {
        for ( int x = xmin; x <= xmax; x += stepX )
        {
            CMedia::Pixel op = pic->pixel( x, y );

            if ( view->normalize() )
            {
                view->normalize( op );
            }

            op.r *= gain;
            op.g *= gain;
            op.b *= gain;

            ColorControlsUI* cc = uiMain->uiColorControls;
            if ( cc->uiActive->value() )
            {
                const Imath::M44f& m = colorMatrix(cc);
                Imath::V3f* iop = (Imath::V3f*)&op;
                *iop *= m;
            }

            if ( view->use_lut() && v == ImageView::kRGBA_Full )
            {
                Imath::V3f* iop = (Imath::V3f*)&op;
                Imath::V3f* irp = (Imath::V3f*)&rp;
                engine->evaluate( img, (*iop), (*irp) );
            }
            else
            {
                rp = op;
            }

            if ( v != ImageView::kRGBA_Original )
            {
                if ( rp.r > 0.0f && isfinite(rp.r) )
                    rp.r = powf(rp.r, one_gamma);
                if ( rp.g > 0.0f && isfinite(rp.g) )
                    rp.g = powf(rp.g, one_gamma);
                if ( rp.b > 0.0f && isfinite(rp.b) )
                    rp.b = powf(rp.b, one_gamma);
            }
            rgb[0] = (uchar)Imath::clamp(rp.r * 255.0f, 0.f, 255.f);
            rgb[1] = (uchar)Imath::clamp(rp.g * 255.0f, 0.f, 255.f);
            rgb[2] = (uchar)Imath::clamp(rp.b * 255.0f, 0.f, 255.f);
            count_pixel( rgb );
        }
    }
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

void Histogram::draw_pixels( const mrv::Recti& r )
{
    count_pixels();

    // Draw the pixel info
    int W = r.w() - 8;
    int H = r.h() - 4;
    int HH = H - 4;
    int idx;
    float v;

    float maxC, maxL;

    switch( _histtype )
    {
    case kLog:
        maxL = logf( 1+maxLumma );
        maxC = logf( 1+maxColor );
        break;
    case kSqrt:
        maxL = sqrtf( 1+maxLumma );
        maxC = sqrtf( 1+maxColor );
        break;
    default:
        maxL = maxLumma;
        maxC = maxColor;
        break;
    }

    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glLineWidth( 4 );
    glBegin( GL_LINES );

    for ( int i = 0; i <= W; ++i )
    {
        int x = i + 4;

        idx = int( ((float) i / (float) W) * 255 );
        if ( _channel == kLumma )
        {
            glColor3f( 1.0, 1.0, 1.0 );
            v = histogram_scale( lumma[idx], maxL );
            int y = int(HH*v);
            glVertex2i( x, y );
            glVertex2i( x, 0 );
            glVertex2i( x+3, 0 );
            glVertex2i( x+3, y );
        }

        if ( _channel == kRed || _channel == kRGB )
        {
            glColor3f( 1.0f, 0.0f, 0.0f );
            v = histogram_scale( red[idx], maxC );
            int y = int(HH*v);
            glVertex2i( x, y );
            glVertex2i( x, 0 );
            glVertex2i( x+3, 0 );
            glVertex2i( x+3, y );
        }

        if ( _channel == kGreen || _channel == kRGB )
        {
            glColor3f( 0.0f, 1.0f, 0.0f );
            v = histogram_scale( green[idx], maxC );
            int y = int(HH*v);
            glVertex2i( x, y );
            glVertex2i( x, 0 );
            glVertex2i( x+3, 0 );
            glVertex2i( x+3, y );
        }

        if ( _channel == kBlue || _channel == kRGB )
        {
            glColor3f( 0.0f, 0.0f, 1.0f );
            v = histogram_scale( blue[idx], maxC );
            int y = int(HH*v);
            glVertex2i( x, y );
            glVertex2i( x, 0 );
            glVertex2i( x+3, 0 );
            glVertex2i( x+3, y );
        }

    }

    glEnd();
}

}
