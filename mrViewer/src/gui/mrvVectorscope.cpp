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



#include "core/mrvThread.h"
#include "core/mrvColorSpaces.h"
#include "core/mrvMath.h"
#include "core/mrvColorOps.h"

#include "gui/mrvIO.h"
#include "gui/mrvVectorscope.h"
#include "gui/mrvImageView.h"
#include "gui/mrvColorInfo.h"
#include "video/mrvDrawEngine.h"
#include "mrViewer.h"


#include <GL/glew.h>

#ifdef OSX
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#if defined(WIN32) || defined(WIN64)
#  include <GL/wglew.h>
#elif defined(LINUX)
#  include <GL/glxew.h>
#endif

#include <FL/gl.h>

#ifdef _WIN32
#define isfinite(x) _finite(x)
#endif

namespace mrv
{

Vectorscope::Vectorscope( int x, int y, int w, int h, const char* l ) :
Fl_Gl_Window( x, y, w, h, l )
{
    box( FL_NO_BOX );
    // color( FL_BLACK );
    //    buttoncolor( FL_BLACK );
    tooltip( _("Mark an area in the image with the left mouse button") );
}


void Vectorscope::draw_grid(const mrv::Recti& r)
{
    int i;
    int W = diameter_/2;
    int H = diameter_/2;

    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glClearColor( 0, 0, 0, 0 );
    glClear( GL_COLOR_BUFFER_BIT );
    glShadeModel( GL_FLAT );
    glDisable( GL_TEXTURE_2D );

    glColor4f( 1.0, 1.0, 1.0, 1.0 );

    // Draw surronding circle
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( W, H, 0 );
    glBegin(GL_LINE_LOOP);
    const int sides = 32;
    for (int j=0; j<sides; j++) {
        double ang = j*2*M_PI/sides;
        glVertex3f(cos(ang)*W,sin(ang)*W,0);
    }
    glEnd();
    glPopMatrix();


    int W2 = r.w() / 2;
    int H2 = r.h() / 2;

    int R2 = diameter_ / 2;
    float angle = 32;
    // Draw diagonal center lines
    for ( i = 0; i < 4; ++i, angle += 90 )
    {
        glPushMatrix();
        glLoadIdentity();
        glTranslatef( W2 + W, H2 + H, 0 );
        glRotatef( angle, 0, 0, 1 );
        glBegin( GL_LINES );
        glVertex2i( 0, 4 );
        glVertex2i( 0, R2 );
        glEnd();
        glPopMatrix();
    }

    // Draw cross
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( W2, H2,0 );
    glBegin( GL_LINES );
      glVertex2i( W, 0);
      glVertex2i( W, diameter_ );
      glVertex2i( 0, H );
      glVertex2i( diameter_, H );
    glEnd();
    glPopMatrix();


    int RW  = int( diameter_ * 0.05f );
    int RH  = RW;

    // Translate cursor to center of drawing
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( W2 + W, H2 + H, 0 );

    static const char* names[] = {
        "R",
        "Y",
        "G",
        "C",
        "B",
        "M"
    };

    const float coords[][2] = {
    {14, 14}, // R
    {18, 14}, // Y
    {12, 26}, // G
    {20, 20}, // C
    {24, 10}, // B
    {14, 14}, // M
    };

    // Draw rectangles with letters near them
    angle = 15;
    for ( i = 0; i < 6; ++i, angle += 60 )
    {
        glDisable( GL_TEXTURE_2D );
        glPushMatrix();
        glRotatef(angle, 0, 0, 1);
        glTranslatef( 0, int(W * 0.75f), 0 );

        glBegin( GL_LINE_LOOP );
        glVertex2i(  -RW, -RH );
        glVertex2i( RW, -RH );
        glVertex2i( RW, RH );
        glVertex2i( -RW,  RH );
        glEnd();


        // @TODO: fltk1.4 cannot draw transformed letters
        glEnable( GL_TEXTURE_2D );
        gl_font( FL_HELVETICA, 12 );
        gl_draw(names[i], coords[i][0], coords[i][1]);
        glPopMatrix();
    }

    glPopMatrix();

}

void Vectorscope::draw()
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


    mrv::Recti r( Fl::box_dx(box()),
                  Fl::box_dy(box()),
                  Fl::box_dw(box()),
                  Fl::box_dh(box()) );

    diameter_ = pixel_h();
    if ( pixel_w() < diameter_ ) diameter_ = pixel_w();
#ifdef OSX
    diameter_ /= 2.0;
#endif
    diameter_ *= 0.95;

    draw_grid(r);
    draw_pixels(r);
}




void Vectorscope::draw_pixel( const mrv::Recti& r,
                              const CMedia::Pixel& rgb,
                              const CMedia::Pixel& hsv )
{
    glColor4f( rgb.r, rgb.g, rgb.b, 1.0f );

    int W2 = (r.w() + diameter_) / 2;
    int H2 = (r.h() + diameter_ )/ 2;

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_TEXTURE_3D );

    glPushMatrix();
    glTranslatef( W2, H2, 0 );
    glRotatef( 15.0 + hsv.r * 360.0f, 0, 0, 1 );
    glScalef( hsv.g * 0.375f, hsv.g * 0.375, 1 );
    glBegin( GL_POINTS );
    glVertex2i( 0, diameter_ );
    glEnd();
    glPopMatrix();
}

void Vectorscope::draw_pixels( const mrv::Recti& r )
{
    mrv::media m = uiMain->uiView->foreground();
    if (!m) {
        tooltip( _("Mark an area in the image with the left mouse button") );
        return;
    }
    CMedia* img = m->image();
    mrv::image_type_ptr pic = img->left();
    if ( !pic ) return;

    tooltip( NULL );

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
    if ( xmin >= (int)pic->width() ) xmin = (int)pic->width()-1;
    if ( ymin >= (int)pic->height() ) ymin = (int)pic->height()-1;

    if ( xmax >= (int)pic->width() ) xmax = (int)pic->width()-1;
    if ( ymax >= (int)pic->height() ) ymax = (int)pic->height()-1;



    unsigned stepX = (xmax - xmin + 1) / w();
    unsigned stepY = (ymax - ymin + 1) / h();
    if ( stepX < 1 ) stepX = 1;
    if ( stepY < 1 ) stepY = 1;

    assert( xmax < (int) pic->width() );
    assert( ymax < (int) pic->height() );

    ImageView* view = uiMain->uiView;
    mrv::DrawEngine* engine = view->engine();

    ImageView::PixelValue v = (ImageView::PixelValue)
                              uiMain->uiPixelValue->value();

    float gain = view->gain();
    float gamma = view->gamma();
    float one_gamma = 1.0f / gamma;

    bool do_gamma = true;
    if ( mrv::is_equal( one_gamma, 1.0f ) ) do_gamma = false;


    CMedia::Pixel rp;
    for ( unsigned y = ymin; y <= (unsigned)ymax; y += stepY )
    {
        for ( unsigned x = xmin; x <= (unsigned)xmax; x += stepX )
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
                Imath::V3f* ov = (Imath::V3f*) &op;
                Imath::V3f* rv = (Imath::V3f*) &rp;
                engine->evaluate( img, *ov, *rv );
            }
            else
            {
                rp = op;
            }

            if ( do_gamma && v != ImageView::kRGBA_Original )
            {
                if ( rp.r > 0.0f && isfinite(rp.r) )
                    rp.r = powf(rp.r, one_gamma);
                if ( rp.g > 0.0f && isfinite(rp.g) )
                    rp.g = powf(rp.g, one_gamma);
                if ( rp.b > 0.0f && isfinite(rp.b) )
                    rp.b = powf(rp.b, one_gamma);
            }

            CMedia::Pixel hsv = color::rgb::to_hsv( rp );
            draw_pixel( r, rp, hsv );
        }
    }

}

}
