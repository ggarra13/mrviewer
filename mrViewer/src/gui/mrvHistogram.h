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
 * @file   mrvHistogram.h
 * @author gga
 * @date   Thu Nov 09 14:43:34 2006
 *
 * @brief  Displays a waveform for an image.
 *
 */

#ifndef mrvHistogram_h
#define mrvHistogram_h

#include <FL/Fl_Box.H>

class ViewerUI;

namespace mrv
{

class CMedia;

class Histogram : public Fl_Box
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

    void channel( Channel c ) {
        _channel = c;
        redraw();
    }
    Channel channel() const    {
        return _channel;
    }

    void histogram_type( Type c ) {
        _histtype = c;
        redraw();
    };
    Type histogram_type() const {
        return _histtype;
    };

    virtual void draw();
//     virtual int handle( int event );


    void main( ViewerUI* m ) {
        uiMain = m;
    };
    ViewerUI* main() {
        return uiMain;
    };

protected:
    void   draw_grid( const Fl_Rect& r );
    void draw_pixels( const Fl_Rect& r );

    void count_pixels();

    void count_pixel( const uchar* rgb );

    inline float histogram_scale( float val, float maxVal );

    Channel      _channel;
    Type         _histtype;

    float maxLumma;
    float lumma[256];

    float maxColor;

    float red[256];
    float green[256];
    float blue[256];


    CMedia* lastImg;
    int64_t    lastFrame;

    ViewerUI* uiMain;

};

}  // namespace mrv


#endif // mrvHistogram_h
