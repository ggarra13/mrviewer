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
 * @file   mrvVectorscope.h
 * @author gga
 * @date   Thu Nov 09 14:43:34 2006
 * 
 * @brief  Displays a waveform for an image.
 * 
 */

#ifndef mrvVectorscope_h
#define mrvVectorscope_h

#include <fltk/Widget.h>

#include "CMedia.h"

namespace mrv
{
  class ViewerUI;

  class Vectorscope : public fltk::Widget
  {
  public:
    Vectorscope( int x, int y, int w, int h, const char* l = 0 );

    virtual void draw();

    void main( mrv::ViewerUI* m ) { uiMain = m; }

  protected:
    void draw_grid( const fltk::Rectangle& r );
    void draw_pixels( const fltk::Rectangle& r );
    void draw_pixel( const fltk::Rectangle& r,
		     const CMedia::Pixel& rgb,
		     const CMedia::Pixel& hsv );

    int diameter_;

    mrv::ViewerUI* uiMain;
  };

}  // namespace mrv


#endif // mrvVectorscope_h
