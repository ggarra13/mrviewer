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

    unsigned int  index( const int64_t frame ) const;
    mrv::media media_at( const int64_t frame ) const;
    CMedia* image_at( const int64_t frame ) const;

    int64_t global_to_local( const int64_t frame ) const;

  protected:
    bool draw(const fltk::Rectangle& sr, fltk::Flags flags, bool slot);
    void draw_ticks(const fltk::Rectangle& r, int min_spacing);

    ImageBrowser* browser() const;

    bool   _edl;
    mrv::Timecode::Display _display;
    double _fps;
    double _display_min;
    double _display_max;

    ViewerUI* uiMain;
  };


  void change_timeline_display( ViewerUI* uiMain );
}


#endif // mrvTimeline_h
