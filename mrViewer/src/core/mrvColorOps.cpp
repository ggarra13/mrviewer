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

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include "core/CMedia.h"
#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"

#include "core/mrvColorOps.h"

namespace mrv {

void bake_ocio( const mrv::image_type_ptr& ptr, const CMedia* img )
{
    const std::string& display = mrv::Preferences::OCIO_Display;
    const std::string& view = mrv::Preferences::OCIO_View;
   
    OCIO::ConstConfigRcPtr config = OCIO::GetCurrentConfig();
    
    OCIO::DisplayTransformRcPtr transform =
    OCIO::DisplayTransform::Create();

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

    OCIO::ConstProcessorRcPtr processor =
    config->getProcessor( transform );

    float* p = (float*)ptr->data().get();

    OCIO::PackedImageDesc baker(p, ptr->width(), ptr->height(),
                                ptr->channels() );
    processor->apply( baker );

}

} // namespace mrv
