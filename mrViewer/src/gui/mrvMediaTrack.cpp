
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>
#include "gui/mrvMediaTrack.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvReelList.h"
#include "mrViewer.h"
#include "mrvEDLWindowUI.h"

namespace mrv {


media_track::media_track(int x, int y, int w, int h) : 
fltk::Widget( x, y, w, h ),
_panX( 0 ),
_zoom( 1.0 )
{
}
 
media_track::~media_track()
{
}

double media_track::frame_size() const
{
   mrv::Timeline* t = main()->uiEDLWindow->uiTimeline;
   
   double x = t->w() / double(t->maximum() - t->minimum() + 1);
   return x;
}

// Add a media at a certain frame (or append to end by default)
void media_track::add( mrv::media m, boost::int64_t frame )
{
   if ( frame == AV_NOPTS_VALUE )
   {
      if ( _position.size() == 0 )
      {
	 frame = 1;
      }
      else
      {
	 size_t e = _position.size();
	 frame = _position.back();
	 const mrv::Reel& reel = browser()->current_reel();
	 mrv::media o = reel->images[e-1];
	 frame += o->image()->duration();
      }
   }
   _position.push_back( frame );
   redraw();
}

mrv::ImageBrowser* media_track::browser() const {
   return _main->uiReelWindow->uiBrowser;
}

mrv::media media_track::media_at_position( const boost::int64_t frame )
{
   const mrv::Reel& reel = browser()->current_reel();
   size_t e = reel->images.size();

   for (size_t i = 0; i < e; ++i )
   {
      boost::int64_t start = _position[i];
      boost::int64_t end   = _position[i];
      mrv::media m = reel->images[i];
      end   += m->image()->duration();
      if ( frame >= start && frame < end )
      {
	 return m;
      }
   }

   return mrv::media();

}

// Remove a media from the track
bool media_track::remove( mrv::media m )
{
   browser()->remove( m );
   return false;
}

// Move a media in track without changing its start or end frames
// Shift surrounding media to remain attached.
void media_track::shift_media( mrv::media m, boost::int64_t frame )
{
   const mrv::Reel& reel = browser()->current_reel();

   size_t e = _position.size();
   size_t idx = 0;

   for (size_t i = 0; i < e; ++i )
   {
      if ( m == reel->images[i] )
      {
	 idx = i;
	 _position[i] = frame;
	 break;
      }
   }

   // Shift medias that come after
   for (size_t i = idx+1; i < e; ++i )
   {
      boost::int64_t end = _position[i-1] + reel->images[i-1]->image()->duration();
      _position[i] = end;
   }


   if ( idx == 0 ) {
      return;
   }

   // Shift medias that come before
   for (int i = int(idx)-1; i >= 0; --i )
   {
      boost::int64_t start = _position[i+1];
      boost::int64_t ee = _position[i] + reel->images[i]->image()->duration();
      boost::int64_t ss = _position[i];
      
      // Shift indexes of position
      _position[i] = (start - (ee - ss ) );
   }

   return;
}

void media_track::shift_media_start( mrv::media m, boost::int64_t diff )
{
   const mrv::Reel& reel = browser()->current_reel();

   int idx = 0;
   size_t e = _position.size();
   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media fg = reel->images[i];
      if ( fg == m )
      {
	 idx = i;
	 int64_t newpos = _position[i] + diff;
	 if ( newpos < _position[i] + m->image()->duration() )
	 {
	    main()->uiView->foreground( fg );
	    CMedia* img = m->image();
	    img->first_frame( img->first_frame() + diff );
	    img->seek( img->first_frame() );
	    // _position[i] += diff;
	 }
	 break;
      }
   }

   // Shift medias that come after
   for (size_t i = idx+1; i < e; ++i )
   {
      boost::int64_t end = _position[i-1] + reel->images[i-1]->image()->duration();
      _position[i] = end;
   }


   if ( idx == 0 ) {
      return;
   }

   // Shift medias that come before
   for (int i = int(idx)-1; i >= 0; --i )
   {
      boost::int64_t start = _position[i+1];
      boost::int64_t ee = _position[i] + reel->images[i]->image()->duration();
      boost::int64_t ss = _position[i];
      
      // Shift indexes of position
      _position[i] = (start - (ee - ss ) );
   }

}

bool media_track::select_media( const boost::int64_t pos )
{
   bool ok = false;
   size_t e = _position.size();
   _selected.reset();

   std::cerr << "select at pos " << pos << std::endl;

   const mrv::Reel& reel = browser()->current_reel();

   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media fg = reel->images[i];
      if ( !fg ) continue;

      CMedia* img = fg->image();
      //int64_t start = img->first_frame() - img->start_frame() + 1;

      int64_t start = _position[i];
      int64_t duration = (int64_t)img->duration();

      std::cerr << i << " start " << start << " end " << start + duration
		<< std::endl;

      if ( pos >= start && pos < start + duration)
      {
	 if ( pos < start + duration / 2 )
	    _at_start = true;
	 else
	    _at_start = false;

	 ok = true;
	 _selected = fg;
	 main()->uiView->foreground( fg );
	 break;
      }
   }
   redraw();
   return ok;
}

mrv::Timeline* media_track::timeline() const
{
   return main()->uiEDLWindow->uiTimeline;
}

void media_track::shift_media_end( mrv::media m, boost::int64_t diff )
{
   const mrv::Reel& reel = browser()->current_reel();

   size_t e = _position.size();
   size_t i = 0;
   for ( ; i < e; ++i )
   {
      mrv::media fg = reel->images[i];
      if ( fg == m )
      {
	 int64_t pos = m->image()->last_frame() + diff;
	 if ( pos > m->image()->first_frame() &&
	      pos <= m->image()->end_frame() )
	 {
	    m->image()->last_frame( pos );
	    m->image()->seek( pos );
	    break;
	 }
      }
   }

   // Shift medias that come before
   for ( ; i < e-1; ++i )
   {
      boost::int64_t start = _position[i];
      boost::int64_t ee = reel->images[i]->image()->duration();
      
      // Shift indexes of position
      _position[i+1] = (start + ee );
   }
   redraw();

}


void  media_track::zoom( double x )
{
   _zoom *= x;
   redraw();
}

int media_track::handle( int event )
{
   switch( event )
   {
      case fltk::RELEASE:
	 if ( _selected && fltk::event_key() == fltk::LeftButton )
	 {
	    mrv::Timeline* t = main()->uiTimeline;
	    if ( ! t->edl() )
	    {
	       int64_t start = _selected->image()->first_frame();
	       int64_t end   = _selected->image()->last_frame();
	       t->minimum( start );
	       main()->uiStartFrame->value( start );
	       t->maximum( end );
	       main()->uiEndFrame->value( end );
	    }

	    main()->uiView->seek( _frame );
	    main()->uiView->play( _playback );
	 }
	 return 1;
	 break;
      case fltk::PUSH:
	 {
	    _dragX = fltk::event_x();
	    int x = fltk::event_x();
	    int y = fltk::event_y();

	    if ( fltk::event_key() == fltk::LeftButton )
	    {

	       if ( y > h() )
	       {
		  _selected.reset();
		  redraw();
		  return 1;
	       }

	       cursor( fltk::CURSOR_ARROW );
	       _playback = (CMedia::Playback) main()->uiView->playback();
	       main()->uiView->stop();
	       _frame = main()->uiView->frame();

	       mrv::Timeline* t = timeline();
	       double len = (t->maximum() - t->minimum() + 1);
	       double p = double(x) / double(w());
	       p = t->minimum() + p * len;
	       select_media( int64_t(p) );
	       return 1;
	    }
	    else
	    {
	       return 0;
	    }
	 }
      case fltk::DRAG:
	 {
	    if ( _selected )
	    {
	       cursor( fltk::CURSOR_WE );

	       const mrv::Reel& reel = browser()->current_reel();
	       mrv::MediaList::const_iterator i = reel->images.begin();
	       mrv::MediaList::const_iterator e = reel->images.end();
 
	       int diff = (fltk::event_x() - _dragX);
	       if ( _zoom > 1.0 ) diff *= _zoom;
	       for ( ; i != e; ++i )
	       {
		  if ( *i == _selected )
		  {
		     if ( _at_start )
			shift_media_start( _selected, diff );
		     else
			shift_media_end( _selected, diff );
		     break;
		  }
	       }
	       redraw();
	    }
	    _dragX = fltk::event_x();
	    return 1;
	 }
      default:
	 break;
   }

   return fltk::Widget::handle( event );
}

void media_track::draw()
{
   size_t e = _position.size();
   const mrv::Reel& reel = browser()->current_reel();

   fltk::load_identity();
   fltk::setcolor( fltk::GRAY33 );
   fltk::push_clip( x(), y(), w(), h() );
   fltk::fillrect( x(), y(), w(), h() );

   fltk::load_identity();

   mrv::Timeline* t = timeline();

   int ww = w();
   int rx = x() + (t->slider_size()-1)/2;

   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media fg = reel->images[i];
      if (!fg) continue;

      int64_t pos = _position[i];


      int dx = t->slider_position( pos, ww );
      int dw = t->slider_position( pos+fg->image()->duration(), ww );
      dw -= dx;
 
      fltk::Rectangle r(rx+dx, y(), dw, h() );
 
      if ( main()->uiView->foreground() == fg )
      {
	 fltk::setcolor( fltk::DARK_YELLOW );
      }
      else
      {
	 fltk::setcolor( fltk::DARK_GREEN );
      }

      fltk::fillrect( r );

      fltk::setcolor( fltk::BLACK );
      if ( _selected == fg )
   	 fltk::setcolor( fltk::WHITE );
      fltk::strokerect( r );

      if ( _selected == fg )
      {
   	 fltk::setcolor( fltk::BLUE );
	 if ( _at_start )
	 {
	    fltk::strokerect( r.x(), y(), dw/2, h() );
	 }
	 else
	 {
	    fltk::strokerect( r.x()+dw/2, y(), dw/2, h() );
	 }
      }


      fltk::setcolor( fltk::BLACK );
      if ( _selected == fg )
   	 fltk::setcolor( fltk::WHITE );

      int ww, hh;
      fltk::setfont( textfont(), 10 );
      std::string name = fg->image()->name();
      const char* const buf = name.c_str();
      fltk::measure( buf, ww, hh );

      assert( dw > ww/2 );

      fltk::drawtext( buf,
		      dx + r.w()/2 - ww/2, y() + 15 );

   }

   fltk::pop_clip();
}

}  // namespace mrv
