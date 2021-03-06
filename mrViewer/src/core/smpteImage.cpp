/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2020  Gonzalo Garramuño

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
const char* kModule = "smpte";
}

namespace mrv {


smpteImage::smpteImage( const smpteImage::Type c, const unsigned int dw,
                        const unsigned int dh ) :
    CMedia(),
    type_(c)
{
    _fileroot = av_strdup("internal");
    _internal = true;
    _gamma = 1.0f;
    default_layers();
    image_size( dw, dh );
    allocate_pixels(_hires, 1);
    _depth = _hires->pixel_type();

}


void smpteImage::gamma_box( mrv::image_type_ptr& canvas,
                            unsigned int X, unsigned int Y,
                            unsigned int W, unsigned int H )
{
    // Draw a line of dots
    Pixel c = bg;
    if ( Y % 2 == 0 ) c = fg;

    Pixel* pixels = (Pixel*)canvas->data().get();
    Pixel* p = pixels + Y * width() + X+1;
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

void smpteImage::gamma_boxes(
                             mrv::image_type_ptr& canvas,
                             unsigned int sx,
                              unsigned int sy,
                              unsigned int W,
                              unsigned int H,
                              float bgc, float fgc )
{
    // Fill image with background color
    Pixel* pixels = (Pixel*)canvas->data().get();
    for ( unsigned int y = sy; y < sy+H; ++y )
    {
        Pixel* p = pixels + y * width() + sx;
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

    bg = Pixel(fgc, bgc, bgc);
    fg = Pixel(0.0f, bgc, bgc);
    gamma_box( _hires, bx, by, bw, bh );

    bg = Pixel( bgc, 0.0f, fgc );
    fg = Pixel( bgc, fgc, 0.0f );
    gamma_box( _hires, bx+ibx, by+iby, ibw, ibh );

    // Draw box #2
    bx += bw + bs;
    bg = Pixel( bgc, 0.0f, bgc );
    fg = Pixel( bgc, fgc, bgc );
    gamma_box( _hires, bx, by, bw, bh );

    bg = Pixel( 0.0f, bgc, fgc );
    fg = Pixel( fgc, bgc, 0.0f );
    gamma_box( _hires, bx+ibx, by+iby, ibw, ibh );

    // Draw box #3
    bx += bw + bs;
    bg = Pixel( bgc, bgc, 0.0f );
    fg = Pixel( bgc, bgc, fgc );
    gamma_box( _hires, bx, by, bw, bh );

    bg = Pixel( fgc, 0.0f, bgc );
    fg = Pixel( 0.0f, fgc, bgc );
    gamma_box( _hires, bx+ibx, by+iby, ibw, ibh );

    // Draw box #4
    bx += bw + bs;
    bg = Pixel( 0.0f, 0.0f, 0.0f );
    fg = Pixel( fgc, fgc, fgc );
    gamma_box( _hires, bx, by, bw, bh );
}

void smpteImage::gamma_chart( mrv::image_type_ptr& canvas )
{
    char buf[1024];
    sprintf( buf, "Gamma %0.1f Chart", _gamma );
    _fileroot = av_strdup(buf);

    unsigned int y = 0;
    unsigned int H = height() / 3;
    unsigned int W = width();

    float b, f;

    // gamma1.4
    if ( _gamma == 1.4 )
    {
        b = 0.13725f;
        f = 0.22745f;
        gamma_boxes( _hires, 0, y, W, H, b, f );

        b = 0.37255f;
        f = 0.61176f;
        gamma_boxes( _hires, 0, y+H, W, H, b, f );

        b = 0.61176f;
        f = 1.0f;
        gamma_boxes( _hires, 0, y+2*H, W, H, b, f );
    }
    else if ( _gamma == 1.8f )
    {
        // gamma1.8
        b = 0.21569f;
        f = 0.31373f;
        gamma_boxes( _hires, 0, y, W, H, b, f );

        b = 0.46275f;
        f = 0.68235f;
        gamma_boxes( _hires, 0, y+H, W, H, b, f );

        b = 0.68235f;
        f = 1.0f;
        gamma_boxes( _hires, 0, y+2*H, W, H, b, f );
    }
    else if ( _gamma == 2.4f )
    {
        // gamma2.4
        b = 0.31373f;
        f = 0.41961f;
        gamma_boxes( _hires, 0, y, W, H, b, f );

        b = 0.56078f;
        f = 0.74902f;
        gamma_boxes( _hires, 0, y+H, W, H, b, f );

        b = 0.74902f;
        f = 1.0f;
        gamma_boxes( _hires, 0, y+2*H, W, H, b, f );
    }
    else
    {
        // gamma2.2
        b = 0.28235f;
        f = 0.38824f;
        gamma_boxes( _hires, 0, y, W, H, b, f );

        b = 0.53333f;
        f = 0.72941f;
        gamma_boxes( _hires, 0, y+H, W, H, b, f );

        b = 0.72941f;
        f = 1.0f;
        gamma_boxes( _hires, 0, y+2*H, W, H, b, f );
    }

}

void smpteImage::linear_gradient( mrv::image_type_ptr& canvas )
{
    _fileroot = av_strdup( "Linear Gradient" );

    unsigned int W = width();
    unsigned int H = height();

    Pixel* pixels = (Pixel*)canvas->data().get();
    for ( unsigned int x = 0; x < W; ++x )
    {
        float val = (float) x / (float) W;
        for ( unsigned int y = 0; y < H; ++y )
        {
            Pixel* p = pixels + y * W + x;
            p->r = p->g = p->b = val;
            p->a = 0;
        }
    }
}

void smpteImage::luminance_gradient( mrv::image_type_ptr& canvas )
{
    _fileroot = av_strdup( "Luminance Gradient" );

    unsigned int W = width();
    unsigned int H = height();

    Pixel* pixels = (Pixel*)canvas->data().get();
    for ( unsigned int x = 0; x < W; ++x )
    {
        float val = (float) x / (float) W;
        val *= val;
        for ( unsigned int y = 0; y < H; ++y )
        {
            Pixel* p = pixels + y * W + x;
            p->r = p->g = p->b = val;
            p->a = 0;
        }
    }
}

void smpteImage::checkered( mrv::image_type_ptr& canvas )
{
    _fileroot = av_strdup( "Checkered" );

    unsigned int W = width();
    unsigned int H = height();



    unsigned int dw = W / 20;
    unsigned int dw2 = dw * 2;
    unsigned int dh = H / 20;
    unsigned int dh2 = dh * 2;


    Pixel* pixels = (Pixel*)canvas->data().get();
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
                    Pixel* p = pixels + dy * W + dx;
                    p->r = p->g = p->b = p->a = val;
                }
            }
        }
    }

}

bool smpteImage::fetch( mrv::image_type_ptr& canvas,
                        const boost::int64_t frame )
{

    switch( type_ )
    {
    case kGammaChart:
        gamma_chart( _hires );
        break;
    case kLinearGradient:
        linear_gradient( _hires );
        break;
    case kLuminanceGradient:
        luminance_gradient( _hires );
        break;
    case kCheckered:
        checkered( _hires );
        break;
    default:
        LOG_ERROR("Internal Image: Unknown image type");
        break;
    }

    return true;
}

} // namespace mrv
