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

#ifndef mrvColorOps_h
#define mrvColorOps_h

extern "C" {
#include <libswscale/swscale.h>
}

#include "ImathMatrix.h"
#include "mrvColorControlsUI.h"

#include "core/mrvFrame.h"

namespace mrv {

class CMedia;

AVPixelFormat ffmpeg_pixel_format( const mrv::image_type::Format& f,
                                   const mrv::image_type::PixelType& p );

bool prepare_image( mrv::image_type_ptr& pic, const CMedia* img,
                    const image_type::Format format,
                    const image_type::PixelType pt );
void bake_ocio( const mrv::image_type_ptr& ptr, const CMedia* img );


#define radians(x) x * M_PI / 180.0f

inline Imath::M44f hueMatrix(float degrees)
{
    float cosA = cos(radians(degrees));
    float sinA = sin(radians(degrees));

    return Imath::M44f( cosA + (1.0 - cosA) / 3.0,
                        1./3. * (1.0 - cosA) - sqrt(1./3.) * sinA,
                        1./3. * (1.0 - cosA) + sqrt(1./3.) * sinA,
                        0.0,
                        1./3. * (1.0 - cosA) + sqrt(1./3.) * sinA,
                        cosA + 1./3.*(1.0 - cosA),
                        1./3. * (1.0 - cosA) - sqrt(1./3.) * sinA,
                        0.0,
                        1./3. * (1.0 - cosA) - sqrt(1./3.) * sinA,
                        1./3. * (1.0 - cosA) + sqrt(1./3.) * sinA,
                        cosA + 1./3. * (1.0 - cosA),
                        0.0f,
                        0.0, 0.0, 0.0, 1.0 );
}

inline Imath::M44f brightnessMatrix(float r, float g, float b)
{
    return Imath::M44f(
    r, 0.F, 0.F, 0.F,
    0.F, g, 0.F, 0.F,
    0.F, 0.F, b, 0.F,
    0.F, 0.F, 0.F, 1.F);
}

inline Imath::M44f contrastMatrix(float r, float g, float b)
{
    return Imath::M44f(
        r, 0.F, 0.F, 0.F,
        0.F, g, 0.F, 0.F,
        0.F, 0.F, b, 0.F,
        (1.F - r) / 2, (1.F - g) / 2, (1.F - b) / 2, 1.F) ;
}

inline Imath::M44f saturationMatrix(float r, float g, float b)
{
    const float s[] =
    {
    (1.F - r) * .3086F,
    (1.F - g) * .6094F,
    (1.F - b) * .0820F
    };
    return Imath::M44f(
        s[0] + r, s[0], s[0], 0.F,
        s[1], s[1] + g, s[1], 0.F,
        s[2], s[2], s[2] + b, 0.F,
        0.F, 0.F, 0.F, 1.F);
}

inline Imath::M44f colorMatrix(const ColorControlsUI* v)
{
    return  hueMatrix(v->uiHue->value()*360.0) *
      brightnessMatrix(v->uiBrightness->value(),
                              v->uiBrightness->value(),
                              v->uiBrightness->value()) *
    contrastMatrix(v->uiContrast->value(), v->uiContrast->value(),
                   v->uiContrast->value()) *
    saturationMatrix(v->uiSaturation->value(), v->uiSaturation->value(),
                     v->uiSaturation->value());
}

}


#endif
