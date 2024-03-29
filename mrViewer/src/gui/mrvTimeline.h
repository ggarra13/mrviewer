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

#include "core/mrvRectangle.h"
#include "gui/mrvSlider.h"
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


class Timeline : public mrv::Slider
{
public:
    typedef CMedia::Mutex   Mutex;

public:
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

    double maximum() const {
        return Fl_Slider::maximum();
    }
    void maximum( double x );
    double minimum() const {
        return Fl_Slider::minimum();
    }
    void minimum( double x );

    double display_minimum() const {
        return _display_min;
    }
    void display_minimum( double x );
    void undo_display_minimum();
    int64_t undo_minimum() const { return _undo_display_min; }

    double display_maximum() const {
        return _display_max;
    }
    void display_maximum( double x );
    void undo_display_maximum();
    int64_t undo_maximum() const { return _undo_display_max; }

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

    int draw_coordinate( double p, int w );
    int slider_position( double p, int w );

    size_t index( const int64_t frame ) const;
    mrv::media media_at( const int64_t frame ) const;
    CMedia* image_at( const int64_t frame ) const;

    int64_t global_to_local( const int64_t frame ) const;

    void draw_annotation( const bool t ) {
        _draw_annotation = t;
    }
    bool draw_annotation() const {
        return _draw_annotation;
    }

    void draw_cache( const bool t ) {
        _draw_cache = t;
    }
    bool draw_cache() const {
        return _draw_cache;
    }

    void clear_thumb() { fg.reset(); }

    void show_thumb();

    ImageBrowser* browser() const;

protected:
    bool draw(const mrv::Recti& sr, int flags, bool slot);
    void draw_ticks(const mrv::Recti& r, int min_spacing);

    void draw_selection( const mrv::Recti& r);
    void draw_annotations( CMedia* img, int64_t pos, int64_t size,
                           int64_t mn, int64_t mx, int64_t frame,
                           const mrv::Recti& r );
    void draw_cacheline( CMedia* img, int64_t pos, int64_t size,
                         int64_t mn, int64_t mx, int64_t frame,
                         const mrv::Recti& r );


    static mrv::Timecode::Display _display;
    bool   _edl;
    bool _draw_annotation;
    bool _draw_cache;
    int64_t _tc;
    double _fps;
    std::atomic<double> _display_min;
    std::atomic<double> _display_max;
    int64_t _undo_display_min;
    int64_t _undo_display_max;

    int event_x, event_y;
    int64_t frame;
    mrv::media fg;
    CMedia*    image;
    Fl_Window* win;
    ViewerUI* uiMain;
};


void change_timeline_display( ViewerUI* uiMain );
}


#endif // mrvTimeline_h
