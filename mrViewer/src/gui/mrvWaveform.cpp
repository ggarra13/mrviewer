/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2018 Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

    This code is largely based on vf_waveform of the ffmpeg project, which is:
    Copyright (c) 2012-2016 Paul B Mahol
    Copyright (c) 2013 Marton Balint

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
#include "core/CMedia.h"

#include "gui/mrvIO.h"
#include "gui/mrvWaveform.h"
#include "gui/mrvImageView.h"
#include "gui/mrvColorInfo.h"
#include "video/mrvDrawEngine.h"
#include "mrViewer.h"



#ifdef _WIN32
#define isfinite(x) _finite(x)
#endif

namespace {

static const char* kModule = "waveform";

}


namespace mrv
{

Waveform::Waveform( int x, int y, int w, int h, const char* l ) :
Fl_Box( x, y, w, h, l ),
fli( NULL ),
_intensity( 0.04f )
{
    color( FL_BLACK );
    // buttoncolor( FL_BLACK );
    tooltip( _("Mark an area in the image with the left mouse button") );
}


void Waveform::draw_grid(const mrv::Recti& r)
{

}

void Waveform::draw()
{
    mrv::Recti r( w(), h() );
    draw_box();
    draw_pixels(r);
}


static void update16(uint16_t *target, int max, int intensity, int limit)
{
    if (*target <= max)
        *target += intensity;
    else
        *target = limit;
}

static void update(uint8_t *target, int max, int intensity)
{
    if (*target <= max)
        *target += intensity;
    else
        *target = 255;
}

static av_always_inline void lowpass(mrv::image_type_ptr in,
                                     mrv::image_type_ptr out,
                                     int component, int intensity,
                                     int offset_y, int offset_x)
{
    const int column = 1;
    const int mirror = 1;
    const int ssize = out->height();
    const int plane = 0;
    const int shift_w = 0;
    const int shift_h = 0;
    const int jobnr = 0;
    const int nb_jobs = 1;
    const int src_linesize = in->width();
    const int dst_linesize = out->line_size();
    const int dst_signed_linesize = dst_linesize * ( mirror == 1 ? -1 : 1 );
    const int max = 255 - intensity;
    const int src_h = AV_CEIL_RSHIFT(in->height(), shift_h);
    const int src_w = AV_CEIL_RSHIFT(in->width(), shift_w);
    const int sliceh_start = !column ? (src_h * jobnr) / nb_jobs : 0;
    const int sliceh_end = !column ? (src_h * (jobnr+1)) / nb_jobs : src_h;
    const int slicew_start = column ? (src_w * jobnr) / nb_jobs : 0;
    const int slicew_end = column ? (src_w * (jobnr+1)) / nb_jobs : src_w;
    const int step = column ? 1 << shift_w : 1 << shift_h;
    const uint8_t *src_data = (uint8_t*)in->data().get() + sliceh_start * src_linesize;
    uint8_t *dst_data = (uint8_t*) out->data().get() + (offset_y + sliceh_start * step) * dst_linesize + offset_x;
    uint8_t * const dst_bottom_line = dst_data + dst_linesize * (ssize-1);
    uint8_t * const dst_line = dst_bottom_line;
    const uint8_t *p;
    int y;

    // fprintf( stderr, "column=%d\n", column );
    // fprintf( stderr, "mirror=%d\n", mirror );
    // fprintf( stderr, "intensity=%d\n", intensity );
    // fprintf( stderr, "plane=%d shift_w=%d shift_h=%d\n",
    // 	     plane, shift_w, shift_h );
    // fprintf( stderr, "src_linesize=%d dst_linesize=%d\n",
    // 	     src_linesize, dst_linesize );
    // fprintf( stderr, "dst_signed_linesize=%d\n", dst_signed_linesize );
    // fprintf( stderr, "src_w=%d src_h=%d\n", src_w, src_h );
    // fprintf( stderr, "jobnr=%d\n\n\n\n", jobnr );
    // fprintf( stderr, "nb_jobs=%d\n\n\n\n", nb_jobs );
    // fprintf( stderr, "sliceh_start=%d sliceh_end=%d\n",
    // 	     sliceh_start, sliceh_end );
    // fprintf( stderr, "slicew_start=%d slicew_end=%d\n",
    // 	     slicew_start, slicew_end );
    // fprintf( stderr, "s->size=%d\n\n\n\n", ssize );
    // fprintf( stderr, "step=%d\n\n\n\n", step );

    if (!column && mirror)
        dst_data += ssize;

    for (y = sliceh_start; y < sliceh_end; y++) {
        const uint8_t *src_data_end = src_data + slicew_end;
        uint8_t *dst = dst_line + slicew_start * step;

        for (p = src_data + slicew_start; p < src_data_end; p++) {
            uint8_t *target;
            if (column) {
                target = dst + dst_signed_linesize * *p;
                dst += step;
                update(target, max, intensity);
            } else {
                uint8_t *row = dst_data;
                if (mirror)
                    target = row - *p - 1;
                else
                    target = row + *p;
                update(target, max, intensity);
                row += dst_linesize;
            }
        }
        src_data += src_linesize;
        dst_data += dst_linesize * step;
    }

    if (column && step > 1) {
        const int dst_h = 256;
        uint8_t *dst;
        int x, z;

        dst = (uint8_t*)out->data().get() + offset_y * dst_linesize + offset_x;
        for (y = 0; y < dst_h; y++) {
            for (x = slicew_start * step; x < slicew_end * step; x+=step) {
                for (z = 1; z < step; z++) {
                    dst[x + z] = dst[x];
                }
            }
            dst += dst_linesize;
        }
    } else if (step > 1) {
        const int dst_w = 256;
        uint8_t *dst;
        int z;

        dst = (uint8_t*)out->data().get() + (offset_y + sliceh_start * step) * dst_linesize + offset_x;
        for (y = sliceh_start * step; y < sliceh_end * step; y+=step) {
            for (z = 1; z < step; z++)
                memcpy(dst + dst_linesize * z, dst, dst_w);
            dst += dst_linesize * step;
        }
    }
}

static av_always_inline void lowpass16(mrv::image_type_ptr in,
                                       mrv::image_type_ptr out,
                                       int component, int intensity,
                                       int offset_y, int offset_x)
{
    const int column = 1;
    const int mirror = 1;
    const int ssize = out->height();
    const int smax = 256;
    const int jobnr = 0;
    const int nb_jobs = 1;
    const int plane = 0;
    const int shift_w = 0;
    const int shift_h = 0;
    const int src_linesize = in->width(); // / 2;
    const int dst_linesize = out->line_size();
    const int dst_signed_linesize = dst_linesize * (mirror == 1 ? -1 : 1);
    const int limit = smax - 1;
    const int max = limit - intensity;
    const int src_h = AV_CEIL_RSHIFT(in->height(), shift_h);
    const int src_w = AV_CEIL_RSHIFT(in->width(), shift_w);
    const int sliceh_start = !column ? (src_h * jobnr) / nb_jobs : 0;
    const int sliceh_end = !column ? (src_h * (jobnr+1)) / nb_jobs : src_h;
    const int slicew_start = column ? (src_w * jobnr) / nb_jobs : 0;
    const int slicew_end = column ? (src_w * (jobnr+1)) / nb_jobs : src_w;
    const int step = column ? 1 << shift_w : 1 << shift_h;
    const uint16_t *src_data = (const uint16_t*)in->data().get() + sliceh_start * src_linesize;
    uint8_t *dst_data = (uint8_t*) out->data().get() + (offset_y + sliceh_start * step) * dst_linesize + offset_x;
    uint8_t * const dst_bottom_line = dst_data + dst_linesize * (ssize - 1);
    uint8_t * const dst_line = (mirror ? dst_bottom_line : dst_data);
    const uint16_t *p;
    int y;

    if (!column && mirror)
        dst_data += ssize;

    for (y = sliceh_start; y < sliceh_end; y++) {
        const uint16_t *src_data_end = src_data + slicew_end;
        uint8_t *dst = dst_line + slicew_start * step;

        for (p = src_data + slicew_start; p < src_data_end; p++) {
            uint8_t *target;
            int i = 0, v = FFMIN(*p, limit);

            if (column) {
                do {
                    target = dst++ + dst_signed_linesize * v;
                    update(target, max, intensity);
                } while (++i < step);
            } else {
                uint8_t *row = dst_data;
                do {
                    if (mirror)
                        target = row - v - 1;
                    else
                        target = row + v;
                    update(target, max, intensity);
                    row += dst_linesize;
                } while (++i < step);
            }
        }
        src_data += src_linesize;
        dst_data += dst_linesize * step;
    }
}

void Waveform::create_image( const mrv::image_type_ptr pic )
{
    const unsigned W = pic->width();
    const unsigned H = pic->height();
    if ( !in || in->width() != W || in->height() != H )
    {
        try
        {
            in.reset( new image_type( 1, W, H, 1, mrv::image_type::kLumma,
                                      mrv::image_type::kByte ) );
        }
        catch( const std::bad_alloc& e )
        {
            throw;
        }
        catch( const std::runtime_error& e )
        {
            throw;
        }
    }

    for (unsigned x = 0; x < W; ++x )
    {
        for (unsigned y = 0; y < H; ++y )
        {
            CMedia::Pixel p = pic->pixel( x, y );
            p = mrv::color::rgb::to_yuv(p);
            p.b = p.r;
            in->pixel( x, y, p );
        }
    }

}

void Waveform::draw_pixels( const mrv::Recti& r )
{
    mrv::media m = uiMain->uiView->foreground();
    if (!m) {
        tooltip( _("Mark an area in the image with the SHIFT + LMB") );
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
    stepX = stepY = 1;

    assert( xmax < pic->width() );
    assert( ymax < pic->height() );


    if ( fli == NULL )
    {
        fli = new Fl_Image( pic->width(),
			    pic->height(), 1 );
    }

    if ( !out || out->width() != w() || out->height() != h() )
    {
        typedef CMedia::Mutex Mutex;
        Mutex& mtx = img->video_mutex();
        SCOPED_LOCK( mtx );

        try
        {
            out.reset( new image_type( 1, pic->width(), 256, 1,
                                       mrv::image_type::kLumma,
                                       VideoFrame::kByte ) );
        }
        catch( const std::bad_alloc& e )
        {
            LOG_ERROR( e.what() );
            return;
        }
        catch( const std::runtime_error& e )
        {
            LOG_ERROR( e.what() );
            return;
        }
    }

    memset( out->data().get(), 0, out->data_size() );
    switch( pic->format() )
    {
    case VideoFrame::kLumma:
    case VideoFrame::kITU_709_YCbCr444:
    case VideoFrame::kITU_601_YCbCr444:
    case VideoFrame::kITU_709_YCbCr422:
    case VideoFrame::kITU_601_YCbCr422:
    case VideoFrame::kITU_709_YCbCr420:
    case VideoFrame::kITU_601_YCbCr420:
    case VideoFrame::kITU_709_YCbCr444A:
    case VideoFrame::kITU_601_YCbCr444A:
    case VideoFrame::kITU_709_YCbCr422A:
    case VideoFrame::kITU_601_YCbCr422A:
    case VideoFrame::kITU_709_YCbCr420A:
    case VideoFrame::kITU_601_YCbCr420A:
        if ( pic->pixel_type() == VideoFrame::kByte )
        {
            int value = _intensity * 256;
            lowpass( pic, out, 0, value, 0, 0 );
        }
        else if ( pic->pixel_type() == VideoFrame::kShort )
        {
            int value = _intensity * 256;
            create_image( pic );
            lowpass( in, out, 0, value, 0, 0);
        }
        else
        {
            static bool warn_error = false;
            if (! warn_error )
            {
                warn_error = true;
                LOG_ERROR( _("Pixel type not well supported in waveform "
                             "monitor") );
                lowpass( pic, out, 0, 10, 0, 0);
            }
        }
        break;
    case VideoFrame::kRGB:
    case VideoFrame::kRGBA:
    case VideoFrame::kBGR:
    case VideoFrame::kBGRA:
    default:
    {
        if ( pic->pixel_type() == VideoFrame::kByte )
        {
            int value = _intensity * 256;
            lowpass( pic, out, 0, value, 0, 0 );
        }
        // else if ( pic->pixel_type() == VideoFrame::kShort )
        // {
        //     std::cerr << "lowpass16 short" << std::endl;
        //     lowpass16( pic, out, 0, 40, 0, 0); break;
        // }
        else if ( pic->pixel_type() == VideoFrame::kShort ||
                  pic->pixel_type() == VideoFrame::kInt ||
                  pic->pixel_type() == VideoFrame::kHalf ||
                  pic->pixel_type() == VideoFrame::kFloat )
        {
            try
            {
                create_image( pic );
                int value = _intensity * 256;
                //lowpass16( in, out, 0, 40, 0, 0 );
                lowpass( in, out, 0, value, 0, 0 );
            }
            catch( const std::bad_alloc& e )
            {
                LOG_ERROR( e.what() );
                return;
            }
            catch( const std::runtime_error& e )
            {
                LOG_ERROR( e.what() );
                return;
            }
        }
        break;
    }
    }

    fl_push_matrix();
    // @TODO:  fltk1.4
    fli->draw(r.x(), r.y(), r.w(), r.h());
    fl_pop_matrix();

}

}
