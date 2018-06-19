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
 * @file   mrvTimeline.cpp
 * @author gga
 * @date   Fri Oct 13 13:36:04 2006
 *
 * @brief  An fltk::Widget to draw a timeline.
 *
 *
 */

#include <cassert>
#include <cmath>  // for fabs()

#include <fltk/draw.h>
#include <fltk/Flags.h>

#include "core/mrvColor.h"
#include "core/mrvI8N.h"
#include "core/mrvThread.h"

#include "gui/mrvImageBrowser.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvImageView.h"
#include "mrViewer.h"
#include "gui/mrvMedia.h"
#include "gui/mrvMediaList.h"

namespace
{
// Maximum number of frames to show cacheline.  Setting it too high can
// impact GUI playback when the image/movies are that long.
unsigned kMAX_FRAMES = 5000;
double kMinFrame = std::numeric_limits<double>::min();
double kMaxFrame = std::numeric_limits<double>::max();
}


namespace mrv
{

mrv::Timecode::Display Timeline::_display = Timecode::kFrames;

Timeline::Timeline( int x, int y, int w, int h, char* l ) :
fltk::Slider( x, y, w, h, l ),
_draw_cache( true ),
_edl( false ),
_tc( 0 ),
_fps( 24 ),
_display_min( kMaxFrame ),
_display_max( kMinFrame ),
uiMain( NULL )
{
    fltk::Slider::minimum( 1 );
    fltk::Slider::maximum( 50 );
}

Timeline::~Timeline()
{
    _draw_cache = false;
    _edl = false;
    uiMain = NULL;
}

  mrv::ImageBrowser* Timeline::browser() const
  {
     assert( uiMain != NULL );
     assert( uiMain->uiReelWindow != NULL );
     if ( uiMain == NULL ) return NULL;
     if ( uiMain->uiReelWindow == NULL ) return NULL;
     return uiMain->uiReelWindow->uiBrowser;
  }

void Timeline::display_minimum( const double& x )
{
    if ( x >= minimum() ) _display_min = x;

    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMinDisplay %g"), x );
        uiMain->uiView->send_network( buf );
    }
}

void Timeline::display_maximum( const double& x )
{
    if ( x <= maximum() ) _display_max = x;

    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMaxDisplay %g"), x );
        uiMain->uiView->send_network( buf );
    }
}

  void Timeline::minimum( double x )
  {
    fltk::Slider::minimum( x );
    _display_min = x;

    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMin %g"), x );
        uiMain->uiView->send_network( buf );
    }
  }

  void Timeline::maximum( double x )
  {
    fltk::Slider::maximum( x );
    _display_max = x;

    if ( uiMain && uiMain->uiView )
    {
        char buf[1024];
        sprintf( buf, N_("TimelineMax %g"), x );
        uiMain->uiView->send_network( buf );
    }
  }

  void Timeline::edl( bool x )
  {
    _edl = x;

    if ( _edl && uiMain && browser() )
      {
        mrv::Timecode* uiFrame = uiMain->uiFrame;

        // Calculate frame range for timeline
        minimum( 1 );
        if ( uiMain->uiStartFrame )
          uiMain->uiStartFrame->frame( 1 );
        if ( uiMain->uiFrame && uiMain->uiFrame->frame() < 1 )
          uiFrame->frame(1);

        uint64_t total = 0;

        const mrv::Reel& reel = browser()->current_reel();
        if ( !reel ) return;

        mrv::MediaList::const_iterator i = reel->images.begin();
        mrv::MediaList::const_iterator e = reel->images.end();

        for ( ; i != e; ++i )
          {
            CMedia* img = (*i)->image();
            if ( (*i)->position() == MRV_NOPTS_VALUE )
               (*i)->position( total );
            total += img->duration();
          }

        maximum( double(total) );
        if ( uiMain->uiEndFrame ) uiMain->uiEndFrame->frame( total );
        if ( uiFrame && uiFrame->frame() > int64_t(total) )
           uiFrame->frame(total);
      }

    redraw();
  }

  /*! Draw tick marks. These lines cross the passed rectangle perpendicular to
    the slider direction. In the direction parallel to the slider direction
    the box should have the same size as the area the slider moves in. */
  void Timeline::draw_ticks(const fltk::Rectangle& r, int min_spacing)
  {
    using namespace fltk;

    int x1, y1, x2, y2, dx, dy, w;
    x1 = x2 = r.x()+(slider_size()-1)/2; dx = 1;
    y1 = r.y(); y2 = r.b()-1; dy = 0;
    w = r.w();

    push_clip( r );

    if (w <= 0) return;

    double A,B;
    if ( uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() &&
         ( display_minimum() != minimum() || display_maximum() != maximum() ) )
    {
        A = display_minimum();
        B = display_maximum();
        if (A > B) {A = B; B = display_minimum();}
    }
    else
    {
        A = minimum();
        B = maximum();
        if (A > B) {A = B; B = minimum();}
    }
    //if (!finite(A) || !finite(B)) return;

    if (min_spacing < 1) min_spacing = 10; // fix for fill sliders
    // Figure out approximate size of min_spacing at zero:

    double mul = 1; // how far apart tick marks are
    double div = 1;
    int smallmod = 5; // how many tick marks apart "larger" ones are
    int nummod = 15; // how many tick marks apart numbers are

    if ( _display != Timecode::kFrames )
      {
          nummod = int(_fps);
      }

    int powincr = 10000;

    double derivative = (B-A)*min_spacing/w;
    if (derivative < step()) derivative = step();
    while (mul*5 <= derivative) mul *= 10;
    while (mul > derivative*2*div) div *= 10;
    if (derivative*div > mul*2) {mul *= 5; smallmod = 2;}
    else if (derivative*div > mul) {mul *= 2; nummod /= 2;}
    if ( nummod <= 1 ) nummod = 1;

    Color textcolor = this->textcolor();
    Color linecolor = lerp(this->color(), textcolor, .66666f);

    setcolor(linecolor);
    char buffer[128];
    for (int n = 0; ; n++) {
      // every ten they get further apart for log slider:
      if (n > powincr) {mul *= 10; n = (n-1)/10+1;}
      double v = mul*n/div;
      if (v > fabs(A) && v > fabs(B)) break;
      int sm = n%smallmod ? 3 : 0;
      if (v >= A && v <= B) {
        int t = slider_position(v, w);
        drawline(x1+dx*t+dy*sm, y1+dy*t+dx*sm, x2+dx*t, y2+dy*t);
        if (n-1 != 0 && (n-1)%nummod == 0) {
            mrv::Timecode::format( buffer, _display, boost::int64_t(v),
                                   _tc, _fps );
            char* p = buffer;
            setfont(textfont(), textsize());
            setcolor(textcolor);
            int wt = 0, ht = 0;
            measure( p, wt, ht );
            drawtext(p, float(x1+dx*t-wt/2),
                     float(y1+dy*t+getsize()-getdescent()));
            setcolor(linecolor);
        }
      }
      if (v && -v >= A && -v <= B) {
        int t = slider_position(-v, w);
        drawline(x1+dx*t+dy*sm, y1+dy*t+dx*sm, x2+dx*t, y2+dy*t);
        if (n%nummod == 0) {
            mrv::Timecode::format( buffer, _display, boost::int64_t(-v), _tc,
                                   _fps );
            char* p = buffer;
            setfont(textfont(), textsize());
            setcolor(textcolor);
            // int wt = 0, ht = 0;
            // measure( p, wt, ht );
            drawtext(p, float(x1+dx*t),
                     float(y1+dy*t+getsize()-getdescent()));
            setcolor(linecolor);
        }
      }
    }

    pop_clip();
  }

  /*!
    Subclasses can use this to redraw the moving part.
    Draw everything that is inside the box, such as the tick marks,
    the moving slider, and the "slot". The slot only drawn if \a slot
    is true. You should already have drawn the background of the slider.
  */
  bool Timeline::draw(const fltk::Rectangle& sr, fltk::Flags flags, bool slot)
  {
    using namespace fltk;

    // for back compatability, use type flag to set slider size:
    if (type()&16/*FILL*/) slider_size(0);

    Rectangle r = sr;

    // draw the tick marks and inset the slider drawing area to clear them:
    if (tick_size() && (type()&TICK_BOTH)) {
      Rectangle tr = r;
      r.move_b(-tick_size());
      switch (type()&TICK_BOTH) {
      case TICK_BOTH:
        r.y(r.y()+tick_size()/2);
        break;
      case TICK_ABOVE:
        r.y(r.y()+tick_size());
        tr.set_b(r.center_y());
        break;
      case TICK_BELOW:
        tr.set_y(r.center_y()+(slot?3:0));
        break;
      }
      setcolor(inactive(contrast(textcolor(),color()),flags));
      draw_ticks(tr, (slider_size()+1)/2);
    }

    if (slot) {
      const int slot_size_ = 6;
      Rectangle sl;
      int dx = (slider_size()-slot_size_)/2; if (dx < 0) dx = 0;
      sl.x(dx+r.x());
      sl.w(r.w()-2*dx);
      sl.y(r.y()+(r.h()-slot_size_+1)/2);
      sl.h(slot_size_);
      setbgcolor(BLACK);
      THIN_DOWN_BOX->draw(sl);
    }

    drawstyle(style(),flags|OUTPUT);
    // if user directly set selected_color we use it:
    if (style()->selection_color_) {
      setbgcolor(style()->selection_color_);
      setcolor(contrast(selection_textcolor(), style()->selection_color_));
    }

    // figure out where the slider should be:
    Rectangle s(r);
    int sglyph = ALIGN_INSIDE; // draw a box
    s.x(r.x()+slider_position(value(),r.w()));
    s.w(slider_size());
    if (!s.w()) {s.w(s.x()-r.x()); s.x(r.x());} // fill slider
    else sglyph=0; // draw our own special glyph

    if ( active() )
        draw_glyph(sglyph, s); // draw slider in new position
    return true;
  }

void Timeline::draw_cacheline( CMedia* img, int64_t pos, int64_t size,
                               int64_t mn, int64_t mx, int64_t frame,
                               const fltk::Rectangle& r )
{

    using namespace fltk;

    int64_t j = frame;
//    if ( !img->has_video() && pos < j ) j = pos;

    int64_t max = frame + size;
    if ( mx < max ) max = mx;

    // If too many frames, playback suffers, so we exit here
    if ( max - j > kMAX_FRAMES ) return;

    int rx = r.x() + (slider_size()-1)/2;
    int r2 = r.b()/2;
    int ww = r.w();

    CMedia::Cache c = CMedia::kLeftCache;
    setcolor( fltk::DARK_GREEN );
    line_style( SOLID, 1 );

    if ( ( img->stereo_output() != CMedia::kNoStereo &&
           img->stereo_output() != CMedia::kStereoLeft ) ||
         img->stereo_input() > CMedia::kSeparateLayersInput )
    {
        c = CMedia::kStereoCache;
        setcolor( fltk::GREEN );
    }



    int dx;

#define NO_FRAME_VALUE std::numeric_limits<int>::min()



    int64_t t2;
    while ( j <= max )
    {
        dx = NO_FRAME_VALUE;
        int64_t t = j - pos + 1;
        for ( ; j < max; ++j, ++t )
        {
            if ( img->is_cache_filled( t ) >= c )
            {
                t2 = t;
                dx = rx + slider_position( double(j), ww );
                break;
            }
        }

        if ( dx == NO_FRAME_VALUE )
            break;

        t = j - pos + 1;
        for ( ; j <= max; ++j, ++t )
        {
            if ( img->is_cache_filled( t ) < c )
            {
                int dx2 = rx + slider_position( double(j), ww );
                int wh = dx2-dx;
                fillrect( dx, r.y()+r2, wh, r2+1 );
                dx = NO_FRAME_VALUE;
                break;
            }
        }
    }

    int64_t t = j - pos;  // not +1
    if ( dx != NO_FRAME_VALUE && img->is_cache_filled( t ) >= c )
    {
        int dx2 = rx + slider_position( double(j), ww );
        int wh = dx2-dx;
        fillrect( dx, r.y()+r2, wh, r2+1 );
    }


}


void Timeline::draw_selection( const fltk::Rectangle& r )
{
    int rx = r.x() + (slider_size()-1)/2;
    int  dx = slider_position( _display_min, r.w() );
    int end = slider_position( _display_max, r.w() );

    setcolor( fltk::CYAN );
    Rectangle r2( rx+dx, r.y(), end-dx, r.h()-8 );
    fillrect( r2 );
}

int Timeline::handle( int e )
{
    return fltk::Slider::handle( e );
    // if ( r != 0 ) return r;
    // return uiMain->uiView->handle( e );
}

  /**
   * Main widget drawing routine
   *
   */
  void Timeline::draw()
  {
    using namespace fltk;

    Flags flags = this->flags();
    Flags f2 = flags & ~FOCUSED;
    if (pushed()) f2 |= PUSHED;
    flags &= ~HIGHLIGHT;

    drawstyle(style(),flags);

    Box* box = this->box();
    if (!box->fills_rectangle()) draw_background();


    Rectangle r(w(),h()); box->draw(r); box->inset(r);

    // Get number of frames
    double mn = minimum();
    double mx = maximum();

    if ( !uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() )
    {
        mn = display_minimum();
        mx = display_maximum();
    }

    double v  = value();

    if ( !browser() ) return;


    // Draw each rectangle for each segment
    if ( _edl )
      {

        const mrv::Reel& reel = browser()->current_reel();
        if ( !reel ) return;

        mrv::MediaList::const_iterator i = reel->images.begin();
        mrv::MediaList::const_iterator e = reel->images.end();

        _fps = 24.0;

        int ww = r.w();

        // If minimum less than 0, start boxes later
        uint64_t size = 0;
        uint64_t frame = 1;
        int rx = r.x() + (slider_size()-1)/2;

        for ( ; i != e; frame += size, ++i )
          {
             int64_t pos = (*i)->position();
             CMedia* img = (*i)->image();
             size = img->duration();

            // skip this block if outside visible timeline span
             if ( frame + size < mn || frame > mx ) continue;

             int  dx = slider_position( double(frame),      ww );
             int end = slider_position( double(frame+size), ww );

            Rectangle lr;
            lr.set( rx+dx, r.y(), end-dx, r.h() );

            // Draw a block
            if ( v >= frame && v < frame + size )
              {
                _fps = img->fps();
                setcolor( highlight_textcolor() );
              }
            else
              {
                setcolor( labelcolor() );
              }

            fillrect( lr );
          }

        if ( ( ! uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() ) &&
             ( _display_min != minimum() || _display_max != maximum() ) )
        {
            draw_selection(r);
        }

        frame = 1;
        unsigned idx = 0;
        for ( i = reel->images.begin(); i != e; frame += size, ++i )
          {
            CMedia* img = (*i)->image();

            CMedia::Mutex& m = img->video_mutex();
            SCOPED_LOCK( m );

            size = img->duration();
            int64_t pos = (*i)->position() - img->first_frame();


            // skip this block if outside visible timeline span
            if ( frame + size < mn || frame > mx ) continue;

            if ( _draw_cache )
            {
                draw_cacheline( img, pos, size, int64_t(minimum()),
                                int64_t(maximum()),
                                frame, r );
            }

            int dx = rx + slider_position( double(frame), ww );

            setcolor( BLUE );
            line_style( SOLID, 3 );
            drawline( dx, r.y(), dx, r.b()-1 ); // -1 to compensate line style
            line_style( SOLID );
          }
      }
    else
    {
        if ( _draw_cache )
        {
            mrv::media m = browser()->current_image();
            if ( m )
            {
                CMedia* img = m->image();
                CMedia::Mutex& mtx = img->video_mutex();
                SCOPED_LOCK( mtx );
                boost::int64_t first = img->first_frame();
                draw_cacheline( img, 1,
                                img->duration() + img->start_number(),
                                int64_t(minimum()), int64_t(maximum()),
                                first, r );
            }
        }

        if ( ( ! uiMain->uiPrefs->uiPrefsTimelineSelectionDisplay->value() ) &&
             ( _display_min != minimum() || _display_max != maximum() ) )
        {
            draw_selection(r);
        }

    }


    draw( r, f2, r.y()==0 );

    // draw the focus indicator inside the box:
    drawstyle(style(),flags);
    box->draw_symbol_overlay(r);
  }

  /**
   * Given an image, return its offset from frame 1 when in edl mode
   *
   * @param img image to search in edl list
   *
   * @return offset in timeline
   */
  uint64_t Timeline::offset( const CMedia* img ) const
  {
    if ( !img ) return 0;

    mrv::Reel reel = browser()->current_reel();
    if (!reel) return 0;

    return reel->offset( img );
  }

  /**
   * Given a frame, return its image index in browser when in edl mode
   *
   * @param f frame to search in edl list
   *
   * @return index of image in image browser list
   */
  size_t Timeline::index( const int64_t f ) const
  {
    const mrv::Reel& reel = browser()->current_reel();
    if (!reel) return 0;

    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    double mn = minimum();
    double mx = maximum();
    if ( mn > mx )
    {
       double t = mx;
       mx = mn; mn = t;
    }

    if ( f < boost::int64_t(mn) ) return 0;
    if ( f > boost::int64_t(mx) ) return unsigned(e - i);

    int64_t  t = 1;
    size_t r = 0;
    for ( ; i != e; ++i, ++r )
      {
        CMedia* img = (*i)->image();
        uint64_t size = img->duration();
        t += size;
        if ( t > f ) break;
      }
    if ( r >= reel->images.size() ) r = reel->images.size() - 1;
    return r;
  }

  /**
   * Given a frame, return the image in browser for that frame when in edl mode
   *
   * @param f frame to search in edl list
   *
   * @return image at that point in the timeline or NULL
   */
  mrv::media Timeline::media_at( const int64_t f ) const
  {
    mrv::Reel reel = browser()->current_reel();
    if (!reel) return mrv::media();

    return reel->media_at( f );
  }

  CMedia* Timeline::image_at( const int64_t f ) const
  {
    mrv::media m = media_at( f );
    if ( !m ) return NULL;
    return m->image();
  }
  /**
   * Given an image, return its offset from frame 1 when in edl mode
   *
   * @param img image to search in edl list
   *
   * @return offset in timeline
   */
int64_t Timeline::global_to_local( const int64_t frame ) const
  {
    mrv::Reel reel = browser()->current_reel();
    if (!reel) return 0;

    return reel->global_to_local( frame );
  }

  void change_timeline_display( mrv::ViewerUI* uiMain )
  {
    int i = uiMain->uiTimecodeSwitch->value();
    const char* label = uiMain->uiTimecodeSwitch->child(i)->label();

    char buf[3]; buf[1] = ':'; buf[2] = 0;
    buf[0] = label[0];

    uiMain->uiTimecodeSwitch->copy_label( buf );

    mrv::Timecode::Display d = (mrv::Timecode::Display) i;
    uiMain->uiFrame->display( d );
    uiMain->uiStartFrame->display( d );
    uiMain->uiEndFrame->display( d );
    uiMain->uiTimeline->display( d );
  }

} // namespace mrv
