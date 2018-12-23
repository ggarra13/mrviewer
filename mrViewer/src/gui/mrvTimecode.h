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
 * @file   mrvTimecode.h
 * @author
 * @date   Fri Oct 13 07:51:42 2006
 *
 * @brief  fltk2 value input displaying value as either frame (normal
 *         value input) or as timecode data in hh:mm:ss|ff(+) format.
 *
 *
 */

#ifndef mrvTimecode_h
#define mrvTimecode_h

#include <fltk/ValueInput.h>

#include "core/mrvInteger.h"


namespace mrv
{

class ViewerUI;

class Timecode : public fltk::FloatInput
{
public:
    enum Display
    {
        kFrames,
        kSeconds,
        kTime,
        kTimecodeNonDrop,
        kTimecodeDropFrame
    };

public:
    Timecode( int x, int y, int w, int h, const char* l = 0 );
    Timecode( int w, int h, const char* l = 0 );

    inline Display display()          {
        return _display;
    }
    void display( Display b );

    inline double fps() const       {
        return _fps;
    }
    inline void fps(double x)       {
        _fps = x;
        update();
    }
    inline void fps(unsigned int x) {
        _fps = double(x);
        update();
    }

    inline int64_t maximum() const  {
        return _maximum;
    }
    inline int64_t minimum() const  {
        return _minimum;
    }
    inline int64_t step()    const  {
        return _step;
    }

    inline void maximum( const int64_t x) {
        _maximum = x;
    }
    inline void minimum( const int64_t x) {
        _minimum = x;
    }
    inline void step( const int64_t x)    {
        _step = x;
    }

    inline int64_t frame() const {
        return _frame;
    }
    inline void frame( const int64_t& f ) {
        value(f);
    }

    inline void timecode( const int64_t& f ) {
        _tc_frame = f;
        update();
    }
    inline int64_t timecode() const          {
        return _tc_frame;
    }

    int64_t value() const;
    void value( const int64_t f );
    void value( const int hours, const int mins, const int secs,
                const int frames );

    static int format( char* buf, const Display display,
                       const int64_t& frame, const int64_t& tc,
                       const double fps, const bool withFrames = false );

    virtual int handle( int e );
    virtual bool replace(int, int, const char*, int);

    void main( ViewerUI* m ) {
        uiMain = m;
    }
    ViewerUI* main() const {
        return uiMain;
    }


protected:
    void update();

    static bool valid_drop_frame( int hours, int mins, int secs, int frames,
                                  double fps );

    ViewerUI*     uiMain;
    Display      _display;
    double       _fps;
    int64_t      _frame, _tc_frame, _minimum, _maximum, _step;
};

} // namespace mrv


#endif // mrvTimecode_h
