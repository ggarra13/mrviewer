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
 * @file   slateImage.h
 * @author gga
 * @date   Sun Oct 22 22:52:27 2006
 * 
 * @brief  A simple image slate generator
 * 
 * 
 */


#ifndef slateImage_h
#define slateImage_h

#include "CMedia.h"

typedef struct _DrawingWand DrawingWand;
typedef struct _PixelWand   PixelWand;
typedef struct _MagickWand  MagickWand;

namespace mrv {

  class slateImage : public CMedia
  {
  public:
    slateImage( const CMedia* src );

    static bool test(const char* file) { return false; }

    virtual const char* const format() const { return "Slate"; }

    virtual bool has_changed() { return false; }
    virtual bool fetch( const boost::int64_t frame );

    virtual bool initialize();
    virtual bool release();


  protected:
    void draw_text( double x, double y, const char* text );
    void draw_gradient();
    void draw_bars();

    int _w, _h;
    int64_t _fstart;
    int64_t _fend;
    DrawingWand* dwand;
    PixelWand*   pwand;  
    MagickWand*   wand;
  };

} // namespace mrv


#endif // slateImage_h
