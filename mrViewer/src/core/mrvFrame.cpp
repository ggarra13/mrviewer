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
 * @file   mrvFrame.cpp
 * @author gga
 * @date   Sun Jan 13 13:51:37 2008
 *
 * @brief
 *
 *
 */

#include <iostream>
#include <limits>    // for quietNaN

#include <half.h>


extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


#include "core/mrvFrame.h"
#include "core/CMedia.h"
#include "core/mrvColorOps.h"


namespace
{

unsigned int
scaleInt(float f, unsigned int i)
{
    return unsigned(f * i + 0.5);
}

// @todo: do not hard-code luminance weights
float yw[3] = { 0.2126f, 0.7152f, 0.0722f };

}  // namespace

#include "core/mrvI8N.h"
#include "core/mrvAlignedData.h"
#include "core/mrvFrame_u8.inl"
#include "core/mrvFrame_u16.inl"
#include "core/mrvFrame_u32.inl"
#include "core/mrvFrame_h16.inl"
#include "core/mrvFrame_f32.inl"


namespace mrv {

const char* const VideoFrame::ptype[] = {
    "Byte",
    "Short",
    "Int",
    "Half",
    "Float",
    "Double",
};

const char* const VideoFrame::fmts[] = {
    "Lumma",
    "LummaA",
    "BGR",
    "BGRA",
    "RGB",
    "RGBA",

    "ITU_601_YCbCr410",
    "ITU_601_YCbCr410A",
    "ITU_601_YCbCr420",
    "ITU_601_YCbCr420A",
    "ITU_601_YCbCr422",
    "ITU_601_YCbCr422A",
    "ITU_601_YCbCr444",
    "ITU_601_YCbCr444A",

    "ITU_709_YCbCr410",
    "ITU_709_YCbCr410A", // @todo: not done
    "ITU_709_YCbCr420",
    "ITU_709_YCbCr420A", // @todo: not done
    "ITU_709_YCbCr422",
    "ITU_709_YCbCr422A", // @todo: not done
    "ITU_709_YCbCr444",
    "ITU_709_YCbCr444A", // @todo: not done

    "YByRy410",
    "YByRy410A", // @todo: not done
    "YByRy420",
    "YByRy420A", // @todo: not done
    "YByRy422",
    "YByRy422A", // @todo: not done
    "YByRy444",
    "YByRy444A", // @todo: not done
};



/**
 * Return the size of a pixel in memory.
 *
 * @return size of pixel in memory
 */
unsigned short VideoFrame::pixel_size() const
{
    switch( _type )
    {
    case kByte:
        return sizeof(char);
    case kShort:
        return sizeof(short);
    case kInt:
        return sizeof(int);
    case kHalf:
        return sizeof(half);
    case kFloat:
        return sizeof(float);
    default:
        throw std::runtime_error( _("Unknown mrv::Frame pixel type") );
    }
}



size_t VideoFrame::line_size() const
{
    size_t size = 0;

    unsigned W2;
    unsigned W = _width;

    switch( _format )
    {
    case kLumma:
        size = W;
        break;

    case kLummaA:
        size = W * 2;
        break;

    case kYByRy410A:
    case kITU_709_YCbCr410A:
    case kITU_601_YCbCr410A:
        size = W; // alpha

    case kYByRy410:
    case kITU_709_YCbCr410:
    case kITU_601_YCbCr410:
        size += W;   // Y
        size += W;   // Cb+Cr
        break;

    case kITU_709_YCbCr420A:
    case kITU_601_YCbCr420A:
    case kYByRy420A:
    case kYUVA:
        size = W; // alpha
    case kYUV:
    case kYByRy420:
    case kITU_709_YCbCr420:
    case kITU_601_YCbCr420:
        size += W;   // Y
        W2 = (_width  + 1) / 2;
        size += 2 * W2;  // U and V
        break;

    case kITU_709_YCbCr422:
    case kITU_601_YCbCr422:
        size += W;   // Y
        W2 = (_width  + 1) / 2;
        size += 2 * W2;  // U and V
        break;

    case kITU_709_YCbCr422A:
    case kITU_601_YCbCr422A:
        size += W;   // alpha
        size += W;   // Y
        W2 = (_width  + 1) / 2;
        size += 2 * W2;  // U and V
        break;

    default:
        size = W * _channels;
        break;
    }

    // multiply size by that of pixel type.  We assume all
    // channels use same pixel type.
    return size * pixel_size();
}

size_t VideoFrame::data_size() const
{
    size_t size = 0;

    unsigned W2, H2, WH2;
    unsigned WH = _width * _height;

    switch( _format )
    {
    case kLummaA:
    case kLumma:
        size = WH * _channels;
        break;

    case kYByRy410A:
    case kITU_709_YCbCr410A:
    case kITU_601_YCbCr410A:
        size = WH; // alpha

    case kYByRy410:
    case kITU_709_YCbCr410:
    case kITU_601_YCbCr410:
        size += WH;   // Y
        size += WH;   // Cb+Cr
        break;

    case kITU_709_YCbCr420A:
    case kITU_601_YCbCr420A:
    case kYByRy420A:
    case kYUVA:
        size = WH; // alpha
    case kYUV:
    case kYByRy420:
    case kITU_709_YCbCr420:
    case kITU_601_YCbCr420:
        size += WH;   // Y
        W2 = (_width  + 1) / 2;
        H2 = (_height + 1) / 2;
        WH2 = W2 * H2;
        size += 2 * WH2;  // U and V
        break;

    case kITU_709_YCbCr422:
    case kITU_601_YCbCr422:
        size += WH;   // Y
        W2 = (_width  + 1) / 2;
        WH2 = W2 * _height;
        size += 2 * WH2;  // U and V
        break;

    default:
        size = WH * _channels;
        break;
    }

    // multiply size by that of pixel type.  We assume all
    // channels use same pixel type.
    return size * pixel_size();
}


VideoFrame::~VideoFrame()
{
    CMedia::memory_used -= data_size();
    //assert0( CMedia::memory_used >= 0 );
    if ( CMedia::memory_used < 0 ) CMedia::memory_used = 0;
}

/**
 * Allocate a frame aligned to 16 bytes in memory.
 *
 */
void VideoFrame::allocate()
{
    size_t size = data_size();
    mrv::aligned16_uint8_t* ptr = new mrv::aligned16_uint8_t[ size ];
    _data.reset( ptr );
    CMedia::memory_used += size;
}

/**
 * Return pixel value of a certain coordinate
 *
 * @param x valid x coordinate of video frame
 * @param y valid y coordinate of video frame
 *
 * @return Image pixel values
 */
ImagePixel VideoFrame::pixel( const unsigned int x,
                              const unsigned int y ) const
{
    if ( !_data ) {
        return ImagePixel(std::numeric_limits<float>::quiet_NaN(),
                          std::numeric_limits<float>::quiet_NaN(),
                          std::numeric_limits<float>::quiet_NaN(),
                          std::numeric_limits<float>::quiet_NaN());
    }

    av_assert0( x < _width  );
    av_assert0( y < _height );


    switch( _type )
    {
    case kByte:
        return pixel_u8( x, y );
    case kShort:
        return pixel_u16( x, y );
    case kInt:
        return pixel_u32( x, y );
    case kHalf:
        return pixel_h16( x, y );
    case kFloat:
        return pixel_f32( x, y );
    default:
        throw std::runtime_error( _("Unknown mrv::Frame pixel type") );
    }
}

/**
 * Set the value of a pixel
 *
 * @param x valid x coordinate of video frame
 * @param y valid y coordinate of video frame
 * @param p pixel values to set x,y with
 */
void VideoFrame::pixel( const unsigned int x, const unsigned int y,
                        const ImagePixel& p )
{
    if ( !_data )
        throw std::runtime_error( _("mrv::Frame No pixel data to change") );

    av_assert0( x < _width  );
    av_assert0( y < _height );

    switch( _type )
    {
    case kByte:
        return pixel_u8( x, y, p );
    case kShort:
        return pixel_u16( x, y, p );
    case kInt:
        return pixel_u32( x, y, p );
    case kHalf:
        return pixel_h16( x, y, p );
    case kFloat:
        return pixel_f32( x, y, p );
    default:
        throw std::runtime_error( _("Unknown mrv::Frame pixel type") );
    }
}

/**
 * Scale video frame in X
 *
 * @param f scale factor
 *
 * @return scaled video frame
 */
VideoFrame* VideoFrame::scaleX(float f) const
{
    unsigned int dw1 = scaleInt( f, _width );
    if ( dw1 < 1 ) dw1 = 1;

    //
    // Create a scaled frame
    //
    VideoFrame* scaled = new VideoFrame( _frame,
                                         dw1,
                                         _height,
                                         _channels,
                                         _format,
                                         _type,
                                         _repeat );


    if ( dw1 > 1 )
        f = float (_width - 1) / float (dw1 - 1);
    else
        f = 1;

    for (unsigned int x = 0; x < dw1; ++x)
    {
        float x1 = float(x) * f;
        size_t xs = static_cast< unsigned int>( x1 );
        size_t xt = std::min( xs + 1, _width - 1 );

        float t = x1 - float(xs);
        assert( t >= 0.0f && t <= 1.0f );

        float s = 1.0f - t;
        assert( s >= 0.0f && s <= 1.0f );

        for (unsigned int y = 0; y < _height; ++y)
        {
            const ImagePixel& ps = pixel(xs, y);
            const ImagePixel& pt = pixel(xt, y);

            assert( ps.r >= 0.f && ps.r <= 1.0f );
            assert( ps.g >= 0.f && ps.g <= 1.0f );
            assert( ps.b >= 0.f && ps.b <= 1.0f );
            assert( ps.a >= 0.f && ps.a <= 1.0f );

            assert( pt.r >= 0.f && pt.r <= 1.0f );
            assert( pt.g >= 0.f && pt.g <= 1.0f );
            assert( pt.b >= 0.f && pt.b <= 1.0f );
            assert( pt.a >= 0.f && pt.a <= 1.0f );

            ImagePixel p(
                ps.r * s + pt.r * t,
                ps.g * s + pt.g * t,
                ps.b * s + pt.b * t,
                ps.a * s + pt.a * t
            );

            // assert( p.r >= 0.f && p.r <= 1.0f );
            // assert( p.g >= 0.f && p.g <= 1.0f );
            // assert( p.b >= 0.f && p.b <= 1.0f );
            // assert( p.a >= 0.f && p.a <= 1.0f );


            scaled->pixel( x, y, p );
        }
    }

    return scaled;
}

/**
 * Scale video frame in Y using a box filter.
 *
 * @param f value to scale
 *
 * @return scaled video frame
 */
VideoFrame* VideoFrame::scaleY(float f) const
{
    unsigned int dh1 = scaleInt( f, _height );
    if ( dh1 < 1 ) dh1 = 1;

    //
    // Create a scaled frame
    //
    VideoFrame* scaled = new VideoFrame( _frame,
                                         _width,
                                         dh1,
                                         _channels,
                                         _format,
                                         _type,
                                         _repeat );

    if ( dh1 > 1 )
        f = float(_height - 1) / float(dh1 - 1);
    else
        f = 1;

    for (unsigned int y = 0; y < dh1; ++y)
    {
        float y1 = float(y) * f;
        size_t ys = static_cast< size_t>( y1 );
        size_t yt = std::min( ys + 1, _height - 1 );
        float t = y1 - float(ys);
        assert( t >= 0.0f && t <= 1.0f );
        float s = 1.0f - t;
        assert( s >= 0.0f && s <= 1.0f );

        for (unsigned int x = 0; x < _width; ++x)
        {
            const ImagePixel& ps = pixel(x, ys);
            const ImagePixel& pt = pixel(x, yt);

            // assert( ps.r >= 0.f && ps.r <= 1.0f );
            // assert( ps.g >= 0.f && ps.g <= 1.0f );
            // assert( ps.b >= 0.f && ps.b <= 1.0f );
            // assert( ps.a >= 0.f && ps.a <= 1.0f );

            // assert( pt.r >= 0.f && pt.r <= 1.0f );
            // assert( pt.g >= 0.f && pt.g <= 1.0f );
            // assert( pt.b >= 0.f && pt.b <= 1.0f );
            // assert( pt.a >= 0.f && pt.a <= 1.0f );

            ImagePixel p(
                ps.r * s + pt.r * t,
                ps.g * s + pt.g * t,
                ps.b * s + pt.b * t,
                ps.a * s + pt.a * t
            );


            // assert( p.r >= 0.f && p.r <= 1.0f );
            // assert( p.g >= 0.f && p.g <= 1.0f );
            // assert( p.b >= 0.f && p.b <= 1.0f );
            // assert( p.a >= 0.f && p.a <= 1.0f );

            scaled->pixel( x, y, p );
        }
    }

    return scaled;
}

/**
 * Quickly resize video frame without any filtering or smoothing of any kind.
 *
 * @param w width of resulting video frame
 * @param h height of resulting video frame
 *
 * @return resized video frame
 */
VideoFrame* VideoFrame::quick_resize( unsigned int w, unsigned int h ) const
{
    double fx, fy;
    if ( w == 0 || width() == 0 )
    {
        fx = 1.0;
    }
    else
    {
        fx = (double)width() / (double) w;
    }

    if ( h == 0 || height() == 0 )
    {
        fy = 1.0;
    }
    else
    {
        fy = (double)height() / (double) h;
    }

    //
    PixelType type = kByte;
    if ( pixel_type() == kHalf || pixel_type() == kFloat ) type = kFloat;

    VideoFrame* scaled = new VideoFrame( _frame, w, h, 3, kRGB, type );

    for ( unsigned y = 0; y < h; ++y )
    {
        unsigned y2 = unsigned( y*fy );
        assert( y2 < height() );
        for ( unsigned x = 0; x < w; ++x )
        {
            assert( x*fx < width() );
            Pixel p = this->pixel( unsigned( x*fx ), y2 );
            p.clamp();
            scaled->pixel( x, y, p );
        }
    }
    return scaled;
}

/**
 * Resize video frame with box filtering.
 *
 * @param w width of resulting video frame
 * @param h height of resulting video frame
 *
 * @return resized video frame
 */
VideoFrame* VideoFrame::resize( unsigned int w, unsigned int h ) const
{

    double f;
    if ( w == 0 || width() == 0 )
        f = 1.0;
    else
        f = (double) w / (double)width();

    VideoFrame* scaledX = scaleX( float(f) );

    if ( h == 0 || height() == 0 )
        f = 1.0;
    else
        f = (double) h / (double)height();

    VideoFrame* scaled  = scaledX->scaleY( float(f) );
    delete scaledX;

    return scaled;
}

/**
 * Return whether picture has alpha or not.
 *
 * @return true if it has alpha, false if not
 */
bool VideoFrame::has_alpha() const
{
    switch( _format )
    {
    case kLummaA:
    case kBGRA:
    case kRGBA:
    case kYUVA:
    case kITU_601_YCbCr410A:
    case kITU_601_YCbCr420A:
    case kITU_601_YCbCr422A:
    case kITU_601_YCbCr444A:
    case kITU_709_YCbCr410A:
    case kITU_709_YCbCr420A:
    case kITU_709_YCbCr422A:
    case kITU_709_YCbCr444A:
    case kYByRy410A:
    case kYByRy420A:
    case kYByRy422A:
    case kYByRy444A:
        return true;
    default:
        return false;
    }
}

/**
 * Equality operator.  Copy video frame into another video frame
 *
 * @param b Video Frame to copy from
 *
 * @return The new video frame.
 */
VideoFrame::self& VideoFrame::operator=( const VideoFrame::self& b )
{
    _frame    = b.frame();
    _pts      = b.pts();
    _repeat   = b.repeat();
    _width    = b.width();
    _height   = b.height();
    _channels = b.channels();
    _format   = b.format();
    _ctime    = time(NULL);
    _mtime    = b.mtime();
    _type     = b.pixel_type();
    _valid    = b.valid();
    allocate();
    memcpy( _data.get(), b.data().get(), b.data_size() );
    return *this;
}


void copy_image( mrv::image_type_ptr& dst, const mrv::image_type_ptr& src,
                 SwsContext* sws_ctx )
{
    unsigned dw = src->width();
    unsigned dh = src->height();
    dst->repeat( src->repeat() );
    dst->frame( src->frame() );
    dst->pts( src->pts() );
    dst->ctime( time(NULL) );
    dst->mtime( time(NULL) );
    av_assert0( dst->channels() > 0 );
    av_assert0( dst->width() > 0 );
    av_assert0( dst->height() > 0 );
    if ( src->pixel_type() == dst->pixel_type() &&
         src->channels() == dst->channels() &&
         src->format() == dst->format() &&
         dw == dst->width() && dh == dst->height() )
    {
        memcpy( dst->data().get(), src->data().get(), src->data_size() );
    }
    else
    {
        image_type_ptr tmp = src;
        if ( src->format() > image_type::kRGBA &&
             ( dst->format() == image_type::kRGB ||
               dst->format() == image_type::kRGBA ) )
        {
            // YUV format, we need to convert to rgba
            try
            {
                tmp.reset( new image_type( src->frame(),
                                           dw, dh,
                                           4,
                                           image_type::kRGBA,
                                           image_type::kByte ) );
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

            AVPixelFormat fmt = ffmpeg_pixel_format( src->format(),
                                                     src->pixel_type() );
            sws_ctx = sws_getCachedContext(sws_ctx,
                                           dw, dh,
                                           fmt, dw, dh,
                                           AV_PIX_FMT_RGBA, 0,
                                           NULL, NULL, NULL);
            if ( !sws_ctx )
            {
                LOG_ERROR( _("Not enough memory for color transform") );
                return;
            }

            uint8_t* buf = (uint8_t*)src->data().get();
            uint8_t* src_data[4] = {NULL, NULL, NULL, NULL};
            int src_linesize[4] = { 0, 0, 0, 0 };
            av_image_fill_arrays( src_data, src_linesize, buf, fmt, dw, dh, 1 );

            uint8_t* tmpbuf = (uint8_t*)tmp->data().get();
            uint8_t* tmp_data[4] = {NULL, NULL, NULL, NULL};
            int tmp_linesize[4] = { 0, 0, 0, 0 };
            av_image_fill_arrays( tmp_data, tmp_linesize, tmpbuf,
                                  AV_PIX_FMT_RGBA, dw, dh, 1 );

            sws_scale( sws_ctx, src_data, src_linesize, 0, dh,
                       tmp_data, tmp_linesize );
        }

        av_assert0( dw <= dst->width() );
        av_assert0( dh <= dst->height() );
        for ( unsigned y = 0; y < dh; ++y )
        {
            for ( unsigned x = 0; x < dw; ++x )
            {
                const ImagePixel& p = tmp->pixel( x, y );
                dst->pixel( x, y, p );
            }
        }
    }
}

void AudioFrame::sum_memory() noexcept
{
    CMedia::memory_used += _size;
}

AudioFrame::~AudioFrame()
{
    delete [] _data;
    _data = NULL;
    CMedia::memory_used -= _size;
    //assert0( CMedia::memory_used >= 0 );
    if ( CMedia::memory_used < 0 ) CMedia::memory_used = 0;
}

} // namespace mrv
