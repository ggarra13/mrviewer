/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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

#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>

#include "core/CMedia.h"
#include "core/mrvRectangle.h"

class ViewerUI;

namespace mrv
{

class Waveform : public Fl_Box
{
public:
    Waveform( int x, int y, int w, int h, const char* l = 0 );

    virtual void draw();

    void intensity( float x ) {
        _intensity = x;
    }

    void main( ViewerUI* m ) {
        uiMain = m;
    }

protected:
    void create_image( const mrv::image_type_ptr pic );
    void draw_grid( const mrv::Recti& r );
    void draw_pixels( const mrv::Recti& r );
    void draw_pixel( const mrv::Recti& r,
                     const float x,
                     const CMedia::Pixel& hsv );

    float _intensity;
    mrv::image_type_ptr in;  // input picture (used when format is not 8 bits)
    mrv::image_type_ptr out; // waveform image data
    Fl_Image* fli; // waveform b&w image
    ViewerUI* uiMain;
};

}  // namespace mrv


#endif // mrvWaveform_h
