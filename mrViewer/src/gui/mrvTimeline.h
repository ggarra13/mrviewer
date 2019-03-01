/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuno

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
 * @file   mrvTimeline.h
 * @author gga
 * @date   Fri Oct 13 12:58:46 2006
 *
 * @brief  An fltk widget to represent a video timeline, displaying
 *         sequence edits, timecode and frame ticks.
 *
 */
#ifndef mrvTimeline_h
#define mrvTimeline_h

#include <vector>

#include <FL/Fl_Slider.H>

#include "core/mrvRectangle.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvMedia.h"


class ViewerUI;

namespace mrv
{
class CMedia;
class ImageBrowser;

namespace gui {
class media;
}


class Timeline : public Fl_Slider
{
public:
    typedef CMedia::Mutex   Mutex;

public:
    enum Ticks
    {
    TICK_ABOVE = 1,
    TICK_BELOW = 2,
    TICK_BOTH  = 3,
    NO_TICK
    };
    enum DisplayMode
    {
        kSingle,
        kEDL_Single,
        kEDL_All
    };

public:
    Timeline( int x, int y, int w, int h, char* l = 0 );
    Timeline( int w, int h, char* l = 0 );
    ~Timeline();

    inline Timecode::Display display() const {
        return _display;
    }
    inline void display(Timecode::Display x) {
        _display = x;
        redraw();
    }

    inline int tick_size() const { return _tick_size; }
    inline void tick_size(int i) { _tick_size = i; }
    
    inline bool edl() const {
        return _edl;
    }
    void edl( bool x );

    inline double fps() const {
        return _fps;
    }
    inline void fps( double x ) {
        _fps = x;
    }

    inline double maximum() const {
        return Fl_Slider::maximum();
    }
    void maximum( double x );
    inline double minimum() const {
        return Fl_Slider::minimum();
    }
    void minimum( double x );

    inline double display_minimum() const {
        return _display_min;
    }
    void display_minimum( const double& x );

    inline double display_maximum() const {
        return _display_max;
    }
    void display_maximum( const double& x );

    void timecode( const int64_t& tc ) {
        _tc = tc;
        redraw();
    }

    virtual int handle( int e );
    virtual void draw();

    uint64_t offset( const CMedia* img )   const;
    uint64_t location( const CMedia* img ) const {
        return offset(img) + 1;
    }

    void main( ViewerUI* m ) {
        uiMain = m;
    }
    ViewerUI* main() const {
        return uiMain;
    }

    int slider_position( double p, int w );

    size_t index( const int64_t frame ) const;
    mrv::media media_at( const int64_t frame ) const;
    CMedia* image_at( const int64_t frame ) const;

    int64_t global_to_local( const int64_t frame ) const;

    void draw_cache( const bool t ) {
        _draw_cache = t;
    }
    bool draw_cache() const {
        return _draw_cache;
    }

protected:
    bool draw(const mrv::Recti& sr, int flags, bool slot);
    void draw_ticks(const mrv::Recti& r, int min_spacing);

    void draw_selection( const mrv::Recti& r);
    void draw_cacheline( CMedia* img, int64_t pos, int64_t size,
                         int64_t mn, int64_t mx, int64_t frame,
                         const mrv::Recti& r );

    ImageBrowser* browser() const;

    static mrv::Timecode::Display _display;
    bool   _edl;
    bool _draw_cache;
    int   _tick_size;
    int64_t _tc;
    double _fps;
    double _display_min;
    double _display_max;

    ViewerUI* uiMain;
};


void change_timeline_display( ViewerUI* uiMain );
}


#endif // mrvTimeline_h
