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

#include <math.h>

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include "core/mrvMath.h"
#include "core/CMedia.h"
#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"

#include "core/mrvColorOps.h"

namespace {
const char* kModule = "[ocio]";
}

namespace mrv {

AVPixelFormat ffmpeg_pixel_format( const mrv::image_type::Format& f,
                                   const mrv::image_type::PixelType& p )
{
    switch( f )
    {
    case mrv::image_type::kITU_601_YCbCr410A:
    case mrv::image_type::kITU_709_YCbCr410A:
    case mrv::image_type::kITU_601_YCbCr410:
    case mrv::image_type::kITU_709_YCbCr410:
        return AV_PIX_FMT_YUV410P;
    case mrv::image_type::kITU_601_YCbCr420:
    case mrv::image_type::kITU_709_YCbCr420:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_YUV420P16LE;
        return AV_PIX_FMT_YUV420P;
    case mrv::image_type::kITU_601_YCbCr420A: // @todo: not done
    case mrv::image_type::kITU_709_YCbCr420A: // @todo: not done
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_YUVA420P16LE;
        return AV_PIX_FMT_YUVA420P;
    case mrv::image_type::kITU_601_YCbCr422:
    case mrv::image_type::kITU_709_YCbCr422:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_YUV422P16LE;
        return AV_PIX_FMT_YUV422P;
    case mrv::image_type::kITU_601_YCbCr422A: // @todo: not done
    case mrv::image_type::kITU_709_YCbCr422A: // @todo: not done
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_YUVA422P16LE;
        return AV_PIX_FMT_YUVA422P;
    case mrv::image_type::kITU_601_YCbCr444:
    case mrv::image_type::kITU_709_YCbCr444:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_YUV444P16LE;
        return AV_PIX_FMT_YUV444P;
    case mrv::image_type::kITU_601_YCbCr444A: // @todo: not done
    case mrv::image_type::kITU_709_YCbCr444A: // @todo: not done
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_YUVA444P16LE;
        return AV_PIX_FMT_YUVA444P;
    case mrv::image_type::kLummaA:
        return AV_PIX_FMT_GRAY8A;
    case mrv::image_type::kLumma:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_GRAY16LE;
        return AV_PIX_FMT_GRAY8;
    case mrv::image_type::kRGB:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_RGB48LE;
        return AV_PIX_FMT_RGB24;
    case mrv::image_type::kRGBA:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_RGBA64LE;
        return AV_PIX_FMT_RGBA;
    case mrv::image_type::kBGR:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_BGR48LE;
        return AV_PIX_FMT_BGR24;
    case mrv::image_type::kBGRA:
        if ( p == mrv::image_type::kShort )
            return AV_PIX_FMT_BGRA64LE;
        return AV_PIX_FMT_BGRA;
    default:
        return AV_PIX_FMT_NONE;
    }
}

void bake_ocio( const mrv::image_type_ptr& pic, const CMedia* img )
{
    char* oldloc = av_strdup( setlocale(LC_NUMERIC, NULL ) );
    setlocale(LC_NUMERIC, "C" );

    try
    {
        const std::string& display = mrv::Preferences::OCIO_Display;
        const std::string& view = mrv::Preferences::OCIO_View;

        OCIO::ConstConfigRcPtr config = mrv::Preferences::OCIOConfig();

#if OCIO_VERSION_HEX >= 0x02000000
        OCIO::DisplayViewTransformRcPtr transform =
          OCIO::DisplayViewTransform::Create();
#else
        OCIO::DisplayTransformRcPtr transform =
          OCIO::DisplayTransform::Create();
#endif

        std::string ics = img->ocio_input_color_space();
        if ( ics.empty() )
        {
            OCIO::ConstColorSpaceRcPtr defaultcs = config->getColorSpace(OCIO::ROLE_SCENE_LINEAR);
            if(!defaultcs)
                throw std::runtime_error( _("ROLE_SCENE_LINEAR not defined." ));
            ics = defaultcs->getName();
        }

#if OCIO_VERSION_HEX >= 0x02000000
        transform->setSrc( ics.c_str() );
#else
        transform->setInputColorSpaceName( ics.c_str() );
#endif
        transform->setDisplay( display.c_str() );
        transform->setView( view.c_str() );

        OCIO::ConstProcessorRcPtr processor = config->getProcessor( transform );

#if OCIO_VERSION_HEX >= 0x02000000
        OCIO::ConstCPUProcessorRcPtr cpu =
          processor->getOptimizedCPUProcessor(OCIO::BIT_DEPTH_F32,
                                              OCIO::BIT_DEPTH_F32,
                                              OCIO::OPTIMIZATION_DEFAULT);
#endif

        float* p = (float*)pic->data().get();
        ptrdiff_t chanstride = pic->pixel_size();
        ptrdiff_t xstride = pic->pixel_size() * pic->channels();
        ptrdiff_t ystride = xstride * pic->width();
#if OCIO_VERSION_HEX >= 0x02000000
        OCIO::PackedImageDesc baker(p, pic->width(), pic->height(),
                                    pic->channels(), OCIO::BIT_DEPTH_F32,
                                    chanstride, xstride, ystride );
        cpu->apply( baker );
#else
        OCIO::PackedImageDesc baker(p, pic->width(), pic->height(),
                                    pic->channels(), chanstride, xstride,
                                    ystride );
        processor->apply( baker );
#endif
    }
    catch( OCIO::Exception& e )
    {
        LOG_ERROR( e.what() );
    }
    catch( std::exception& e )
    {
        LOG_ERROR( e.what() );
    }

    setlocale(LC_NUMERIC, oldloc );
    av_free( oldloc );
}

static SwsContext* save_ctx = NULL;

bool prepare_image( mrv::image_type_ptr& pic, const CMedia* img,
                    const image_type::Format format,
                    const image_type::PixelType pt )
{
    unsigned dw = pic->width();
    unsigned dh = pic->height();
    mrv::image_type_ptr sho = pic;

    // Memory is kept until we save the image
    mrv::image_type_ptr ptr = pic;

    const std::string& display = mrv::Preferences::OCIO_Display;
    const std::string& view = mrv::Preferences::OCIO_View;

    if ( Preferences::use_ocio && !display.empty() && !view.empty() &&
         Preferences::uiMain->uiView->use_lut() )
    {
        try {
            ptr = image_type_ptr( new image_type(
                                      img->frame(),
                                      dw, dh, 4,
                                      image_type::kRGBA,
                                      image_type::kFloat ) );
            copy_image( ptr, pic, &save_ctx );
            bake_ocio( ptr, img );
        }
        catch( const std::exception& e )
        {
            LOG_ERROR( e.what() );
            return false;
        }

    }

    if ( ! mrv::is_equal( img->gamma(), 1.0f ) )
    {
        float one_gamma = 1.0f / img->gamma();

        for ( unsigned y = 0; y < dh; ++y )
        {
            for ( unsigned x = 0; x < dw; ++x )
            {
                ImagePixel p = ptr->pixel( x, y );

                if ( p.r > 0.f && isfinite(p.r) )
                    p.r = expf( logf(p.r) * one_gamma );
                if ( p.g > 0.f && isfinite(p.g) )
                    p.g = expf( logf(p.g) * one_gamma );
                if ( p.b > 0.f && isfinite(p.b) )
                    p.b = expf( logf(p.b) * one_gamma );

                sho->pixel( x, y, p );
            }
        }
    }
    else
    {
        sho = ptr;
    }

    unsigned channels = 4;
    if ( format == image_type::kRGB ) channels = 3;
    else if ( format == image_type::kLumma ) channels = 1;
    pic = mrv::image_type_ptr( new image_type( img->frame(),
                               dw, dh, channels,
                               format, pt ) );


    for ( unsigned y = 0; y < dh; ++y )
    {
        for ( unsigned x = 0; x < dw; ++x )
        {
            ImagePixel p = sho->pixel( x, y );
            p.clamp();
            pic->pixel( x, y, p );
        }
    }

    if ( save_ctx )
    {
        sws_freeContext( save_ctx );
        save_ctx = NULL;
    }

    return true;
}


} // namespace mrv
