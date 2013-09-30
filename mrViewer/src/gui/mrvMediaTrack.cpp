
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>
#include "gui/mrvMediaTrack.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
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
   
   double x = w() / double(t->maximum() - t->minimum() + 1);
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
	 frame = _position.back();
	 mrv::media o = _media.back();
	 frame += o->image()->duration();
      }
   }
   _position.push_back( frame );
   _media.push_back( m );
   redraw();
}

mrv::media media_track::media_at_position( const boost::int64_t frame )
{
   size_t e = _media.size();

   for (size_t i = 0; i < e; ++i )
   {
      boost::int64_t start = _position[i];
      boost::int64_t end   = _position[i];
      mrv::media m = _media[i];
      start += m->image()->first_frame();
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
   size_t e = _media.size();

   for (size_t i = 0; i < e; ++i )
   {
      if ( _media[i] == m )
      {
	 _media.erase( _media.begin() + i );
	 _position.erase( _position.begin() + i );
	 return true;
      }
   }
   return false;
}

// Move a media in track without changing its start or end frames
// Shift surrounding media to remain attached.
void media_track::shift_media( mrv::media m, boost::int64_t frame )
{

   size_t e = _media.size();
   size_t idx = 0;

   for (size_t i = 0; i < e; ++i )
   {
      if ( m == _media[i] )
      {
	 idx = i;
	 _position[i] = frame;
	 break;
      }
   }

   // Shift medias that come after
   for (size_t i = idx+1; i < e; ++i )
   {
      boost::int64_t end   = _position[i-1] + _media[i-1]->image()->duration();
      _position[i] = end;
   }


   if ( idx == 0 ) {
      return;
   }

   // Shift medias that come before
   for (int i = int(idx)-1; i >= 0; --i )
   {
      boost::int64_t start = _position[i+1];
      boost::int64_t ee = _position[i] + _media[i]->image()->duration();
      boost::int64_t ss = _position[i];
      
      // Shift indexes of position
      _position[i] = (start - (ee - ss ) );
   }

   return;
}

void media_track::shift_media_start( mrv::media m, boost::int64_t diff )
{

   int idx = 0;
   size_t e = _position.size();
   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media fg = _media[i];
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
      boost::int64_t end = _position[i-1] + _media[i-1]->image()->duration();
      _position[i] = end;
   }


   if ( idx == 0 ) {
      return;
   }

   // Shift medias that come before
   for (int i = int(idx)-1; i >= 0; --i )
   {
      boost::int64_t start = _position[i+1];
      boost::int64_t ee = _position[i] + _media[i]->image()->duration();
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

   for ( size_t i = 0; i < e; ++i )
   {
      int64_t start = _position[i];
      mrv::media fg = _media[i];
      if ( !fg ) continue;

      if ( pos >= start && pos < start + (int64_t)fg->image()->duration() )
      {
	 ok = true;
	 _selected = fg;
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

   size_t e = _position.size();
   size_t i = 0;
   for ( ; i < e; ++i )
   {
      mrv::media fg = _media[i];
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
      boost::int64_t ee = _media[i]->image()->duration();
      
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
	 if ( _selected )
	 {
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

	       MediaList::iterator i = _media.begin();
	       MediaList::iterator e = _media.end();
	       int diff = (fltk::event_x() - _dragX);
	       for ( ; i != e; ++i )
	       {
		  if ( *i == _selected )
		  {
		     shift_media_start( _selected, diff );
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


   fltk::load_identity();
   fltk::setcolor( fltk::GRAY33 );
   fltk::push_clip( x(), y(), w(), h() );
   fltk::fillrect( x(), y(), w(), h() );

   fltk::load_identity();
   fltk::translate( _panX * frame_size(), 0 );

   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media& fg = _media[i];
      if (!fg) continue;

      int64_t pos = _position[i];

      int dx = pos * frame_size();
      int dw = fg->image()->duration() * frame_size(); 

      if ( main()->uiView->foreground() == fg )
      {
	 fltk::setcolor( fltk::DARK_YELLOW );
      }
      else
      {
	 fltk::setcolor( fltk::DARK_GREEN );
      }

      fltk::fillrect( dx, y(), dw, h() );

      fltk::setcolor( fltk::BLACK );
      if ( _selected == fg )
   	 fltk::setcolor( fltk::WHITE );
      fltk::strokerect( dx, y(), dw, h() );



      int ww, hh;
      fltk::setfont( textfont(), 10 );
      const char* const buf = fg->image()->name().c_str();
      fltk::measure( buf, ww, hh );
      if ( ww < 8 ) ww = 24;

      for ( int j = ww; j < dw-ww/2; j += ww*2 )
      {
	 fltk::drawtext( buf,
			 dx + j - ww/2, y() + 15 );
      }

   }

   fltk::pop_clip();
}

}  // namespace mrv
