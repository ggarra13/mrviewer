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

#include <fltk/Slider.h>

#include "gui/mrvTimecode.h"
#include "gui/mrvMedia.h"


class CMedia;

namespace mrv
{
  class ViewerUI;
  class ImageBrowser;

  namespace gui {
    class media;
  }


  class Timeline : public fltk::Slider
  {
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

    Timecode::Display display() const { return _display; }
    void display(Timecode::Display x) { _display = x; redraw(); }

    bool edl() const { return _edl; }
    void edl( bool x );

    double fps() const { return _fps; }
    void fps( double x ) { _fps = x; }

    double maximum() const { return fltk::Slider::maximum(); }
    void maximum( double x );
    double minimum() const { return fltk::Slider::minimum(); }
    void minimum( double x );


    virtual void draw();

    uint64_t offset( const CMedia* img )   const;
    uint64_t location( const CMedia* img ) const { return offset(img) + 1; }

    void main( ViewerUI* m ) { uiMain = m; }
       ViewerUI* main() const { return uiMain; }


    size_t index( const int64_t frame ) const;
    mrv::media media_at( const int64_t frame ) const;
    CMedia* image_at( const int64_t frame ) const;

    int64_t global_to_local( const int64_t frame ) const;

      void draw_cache( const bool t ) { _draw_cache = t; }
      bool draw_cache() const { return _draw_cache; }

  protected:
    bool draw(const fltk::Rectangle& sr, fltk::Flags flags, bool slot);
    void draw_ticks(const fltk::Rectangle& r, int min_spacing);

    void draw_cacheline( CMedia* img, int64_t pos, int64_t size,
                         int64_t mn, int64_t mx, int64_t frame, 
                         const fltk::Rectangle& r );

    ImageBrowser* browser() const;

       static mrv::Timecode::Display _display;
    bool   _edl;
      bool _draw_cache;
    double _fps;
    double _display_min;
    double _display_max;

    ViewerUI* uiMain;
  };


  void change_timeline_display( ViewerUI* uiMain );
}


#endif // mrvTimeline_h
