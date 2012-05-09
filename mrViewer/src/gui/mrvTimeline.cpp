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

#include "gui/mrvImageBrowser.h"
#include "gui/mrvTimecode.h"
#include "gui/mrvTimeline.h"
#include "mrViewer.h"
#include "gui/mrvMedia.h"
#include "gui/mrvMediaList.h"



namespace mrv
{


  Timeline::Timeline( int x, int y, int w, int h, char* l ) :
    fltk::Slider( x, y, w, h, l ),
    _edl( false ),
    _display( Timecode::kFrames ),
    _fps( 24 ),
    _display_min(1),
    _display_max(50),
    uiMain( NULL )
  {
  }

  mrv::ImageBrowser* Timeline::browser() const
  {
    return uiMain->uiReelWindow->uiBrowser;
  }

  void Timeline::minimum( double x )
  {
    fltk::Slider::minimum( x );
  }

  void Timeline::maximum( double x )
  {
    fltk::Slider::maximum( x );
  }

  void Timeline::edl( bool x )
  {
    _edl = x;

    if ( _edl && uiMain )
      {
	mrv::Timecode* uiFrame = uiMain->uiFrame;

	// Calculate frame range for timeline
	minimum( 1 );
	if ( uiMain->uiStartFrame ) 
	  uiMain->uiStartFrame->value( 1 );
	if ( uiMain->uiFrame && uiMain->uiFrame->value() < 1 ) 
	  uiFrame->value(1);

	unsigned total = 0;

	const mrv::Reel& reel = browser()->current_reel();
	mrv::MediaList::const_iterator i = reel->images.begin();
	mrv::MediaList::const_iterator e = reel->images.end();

	for ( ; i != e; ++i )
	  {
	    CMedia* img = (*i)->image();
	    total += img->last_frame() - img->first_frame() + 1;
	  }

	maximum( total );
	if ( uiMain->uiEndFrame ) uiMain->uiEndFrame->value( total );
	if ( uiFrame && uiFrame->value() > total ) uiFrame->value(total);
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
    double A = minimum();
    double B = maximum();
    if (A > B) {A = B; B = minimum();}
    //if (!finite(A) || !finite(B)) return;

    if (min_spacing < 1) min_spacing = 10; // fix for fill sliders
    // Figure out approximate size of min_spacing at zero:

    double mul = 1; // how far apart tick marks are
    double div = 1;
    int smallmod = 5; // how many tick marks apart "larger" ones are
    int nummod = 10; // how many tick marks apart numbers are
    
    if ( _display != Timecode::kFrames ) 
      {
	mul = _fps;
      }

    // if ( _display != Timecode::kFrames ) nummod = int(_fps);
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
	if (n%nummod == 0) {
	  mrv::Timecode::format( buffer, _display, boost::int64_t(v), _fps );
	  char* p = buffer;
	  setfont(textfont(), textsize());
	  setcolor(textcolor);
	  int wt = 0, ht;
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
	  mrv::Timecode::format( buffer, _display, boost::int64_t(v), _fps );
	  char* p = buffer;
	  setfont(textfont(), textsize());
	  setcolor(textcolor);
	  int wt = 0, ht;
	  measure( p, wt, ht );
	  drawtext(p, float(x1+dx*t-w/2),
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
    draw_glyph(sglyph, s); // draw slider in new position
    return true;
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
    double v  = value();


    // Draw each rectangle for each segment
    if ( _edl )
      {
	const mrv::Reel& reel = browser()->current_reel();
	mrv::MediaList::const_iterator i = reel->images.begin();
	mrv::MediaList::const_iterator e = reel->images.end();

	_fps = 24.0;

	int ww = r.w();

	// If minimum less than 0, start boxes later
	uint64_t size = 0;
	double frame = 1;
	int rx = r.x() + (slider_size()-1)/2;

	for ( ; i != e; frame += size, ++i )
	  {
	    CMedia* img = (*i)->image();
	    size = img->last_frame() - img->first_frame() + 1;

	    // skip this block if outside visible timeline span
	    if ( frame + size < mn || frame > mx ) continue;

	    int  dx = slider_position( frame,      ww );
	    int end = slider_position( frame+size, ww );
	    
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

	frame = 1;
	for ( i = reel->images.begin(); i != e; frame += size, ++i )
	  {
	    CMedia* img = (*i)->image();
	    size = img->last_frame() - img->first_frame() + 1;

	    // skip this block if outside visible timeline span
	    if ( frame + size < mn || frame > mx ) continue;

	    int dx = rx + slider_position( frame, ww );

	    setcolor( BLUE );
	    line_style( SOLID, 3 );
	    drawline( dx, r.y(), dx, r.b()-1 ); // -1 to compensate line style
	    line_style( SOLID );
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

    const mrv::Reel& reel = browser()->current_reel();
    assert( reel );

    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    uint64_t t = 0;
    for ( ; i != e && (*i)->image() != img; ++i )
      {
	CMedia* timg = (*i)->image();
	assert( timg != NULL );

	uint64_t size = timg->last_frame() - timg->first_frame() + 1;
	t += size;
      }
    return t;
  }

  /** 
   * Given a frame, return its image index in browser when in edl mode
   * 
   * @param f frame to search in edl list
   * 
   * @return index of image in image browser list
   */
  unsigned int Timeline::index( const int64_t f ) const
  {
    const mrv::Reel& reel = browser()->current_reel();
    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    double mn = minimum();
    double mx = maximum();
    if ( mn > mx ) 
    {
       double t = mx;
       mx = mn; mn = t;
    }

    if ( f < mn ) return mn;
    if ( f > mx ) return mx;

    int64_t  t = 1;
    unsigned r = 0;
    for ( ; i != e; ++i, ++r )
      {
	CMedia* img = (*i)->image();
	uint64_t size = img->last_frame() - img->first_frame() + 1;
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
    const mrv::Reel& reel = browser()->current_reel();
    if (!reel) return mrv::media();

    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    double mn = minimum();
    double mx = maximum();
    if ( mn > mx ) 
    {
       double t = mx;
       mx = mn; mn = t;
    }

    if ( f < mn ) return mrv::media();
    if ( f > mx ) return mrv::media();

    int64_t  t = 1;
    unsigned r = 0;
    for ( ; i != e; ++i, ++r )
      {
	CMedia* img = (*i)->image();
	uint64_t size = img->last_frame() - img->first_frame() + 1;
	t += size;
	if ( t > f ) break;
      }
    if ( r >= reel->images.size() ) return mrv::media();

    return reel->images[r];
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

    const mrv::Reel& reel = browser()->current_reel();
    assert( reel );

    mrv::MediaList::const_iterator i = reel->images.begin();
    mrv::MediaList::const_iterator e = reel->images.end();

    int64_t r = 0;
    uint64_t t = 0;
    for ( ; i != e; ++i )
      {
	CMedia* img = (*i)->image();
	assert( img != NULL );

	uint64_t size = img->last_frame() - img->first_frame() + 1;
	if ( frame > t && t+size < frame )
	   t += size;
	else if ( frame >= t )
	{
	   r = frame - int64_t(t);
	}
      }
    return r;
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
