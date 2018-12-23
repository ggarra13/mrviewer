/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

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
 * @file   mrvWaveform.h
 * @author gga
 * @date   Thu Nov 09 14:43:34 2006
 *
 * @brief  Displays a waveform for an image.
 *
 */

#ifndef mrvWaveform_h
#define mrvWaveform_h

#include <fltk/Widget.h>
#include <fltk/Image.h>

#include "CMedia.h"

namespace mrv
{
class ViewerUI;

class Waveform : public fltk::Widget
{
public:
    Waveform( int x, int y, int w, int h, const char* l = 0 );

    virtual void draw();

    void intensity( float x ) {
        _intensity = x;
    }

    void main( mrv::ViewerUI* m ) {
        uiMain = m;
    }

protected:
    void create_image( const mrv::image_type_ptr pic );
    void draw_grid( const fltk::Rectangle& r );
    void draw_pixels( const fltk::Rectangle& r );
    void draw_pixel( const fltk::Rectangle& r,
                     const float x,
                     const CMedia::Pixel& hsv );

    float _intensity;
    mrv::image_type_ptr in;  // input picture (used when format is not 8 bits)
    mrv::image_type_ptr out; // waveform image data
    fltk::Image* fli; // waveform b&w image
    mrv::ViewerUI* uiMain;
};

}  // namespace mrv


#endif // mrvWaveform_h
