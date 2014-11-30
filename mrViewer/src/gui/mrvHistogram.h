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
 * @file   mrvHistogram.h
 * @author gga
 * @date   Thu Nov 09 14:43:34 2006
 * 
 * @brief  Displays a waveform for an image.
 * 
 */

#ifndef mrvHistogram_h
#define mrvHistogram_h

#include <fltk/Widget.h>

class CMedia;

namespace mrv
{
  class ViewerUI;

  class Histogram : public fltk::Widget
  {
  public:
    enum Type
      {
	kLinear,
	kLog,
	kSqrt,
      };

    enum Channel
      {
	kRGB,
	kRed,
	kGreen,
	kBlue,
	kLumma,
      };

  public:
    Histogram( int x, int y, int w, int h, const char* l = 0 );

    void channel( Channel c ) { _channel = c; redraw(); }
    Channel channel() const    { return _channel; }

    void histogram_type( Type c ) { _histtype = c; redraw(); };
    Type histogram_type() const { return _histtype; };

    virtual void draw();
//     virtual int handle( int event );


    void main( mrv::ViewerUI* m ) { uiMain = m; };
    mrv::ViewerUI* main() { return uiMain; };

  protected:
    void   draw_grid( const fltk::Rectangle& r );
    void draw_pixels( const fltk::Rectangle& r );

    void count_pixels();

    void count_pixel( const uchar* rgb );

    inline float histogram_scale( float val, float maxVal );

    Channel      _channel;
    Type         _histtype;

    float maxLumma;
    float lumma[256];

    float maxValues[3];

    float maxRed;
    float red[256];

    float maxGreen;
    float green[256];

    float maxBlue;
    float blue[256];


    CMedia* lastImg;
    int64_t    lastFrame;

    mrv::ViewerUI* uiMain;

  };

}  // namespace mrv


#endif // mrvHistogram_h
