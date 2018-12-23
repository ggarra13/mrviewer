/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2018  Gonzalo Garramuño

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
#include "mrViewer.h"

#include "core/mrvColorOps.h"

namespace {
const char* kModule = "[ocio]";
}

namespace mrv {

void bake_ocio( const mrv::image_type_ptr& pic, const CMedia* img )
{
    const std::string& display = mrv::Preferences::OCIO_Display;
    const std::string& view = mrv::Preferences::OCIO_View;

    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();

    OCIO::DisplayTransformRcPtr transform = OCIO::DisplayTransform::Create();

    std::string ics = img->ocio_input_color_space();
    if  ( ics.empty() )
    {
        OCIO::ConstColorSpaceRcPtr defaultcs = config->getColorSpace(OCIO::ROLE_SCENE_LINEAR);
        if(!defaultcs)
            throw std::runtime_error( _("ROLE_SCENE_LINEAR not defined." ));
        ics = defaultcs->getName();
    }

    transform->setInputColorSpaceName( ics.c_str() );
    transform->setDisplay( display.c_str() );
    transform->setView( view.c_str() );

    OCIO::ConstProcessorRcPtr processor = config->getProcessor( transform );

    float* p = (float*)pic->data().get();
    ptrdiff_t chanstride = pic->pixel_size();
    ptrdiff_t xstride = pic->pixel_size() * pic->channels();
    ptrdiff_t ystride = pic->pixel_size() * pic->channels() * pic->width();
    OCIO::PackedImageDesc baker(p, pic->width(), pic->height(),
                                pic->channels(), chanstride, xstride,
                                ystride );
    processor->apply( baker );

}

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
            copy_image( ptr, pic );
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

    return true;
}


} // namespace mrv
