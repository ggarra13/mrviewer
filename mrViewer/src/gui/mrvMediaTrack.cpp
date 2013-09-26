
#include <fltk/draw.h>
#include <fltk/events.h>
#include "gui/mrvMediaTrack.h"
#include "gui/mrvImageView.h"
#include "mrViewer.h"

namespace mrv {

static float frame_size = 9.69;

media_track::media_track(int x, int y, int w, int h) : 
fltk::Widget( x, y, w, h ),
_panX( 0 )
{
}
 
media_track::~media_track()
{
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
      if ( frame >= start && frame <= end )
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
   CMedia::Playback playback = (CMedia::Playback) main()->uiView->playback();
   main()->uiView->stop();

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
   CMedia::Playback playback = (CMedia::Playback) main()->uiView->playback();
   main()->uiView->stop();

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
	    m->image()->first_frame( m->image()->first_frame() + diff );
	    _position[i] += diff;
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

      if ( pos >= start && pos <= start + (int64_t)fg->image()->duration() )
      {
	 ok = true;
	 _selected = fg;
	 break;
      }
   }
   redraw();
   return ok;
}

void media_track::shift_media_end( mrv::media m, boost::int64_t diff )
{
   CMedia::Playback playback = (CMedia::Playback) main()->uiView->playback();
   main()->uiView->stop();

   size_t e = _position.size();
   size_t i = 0;
   for ( ; i < e; ++i )
   {
      mrv::media fg = _media[i];
      if ( fg == m )
      {
	 int64_t pos = m->image()->last_frame() + diff;
	 if ( pos > m->image()->first_frame() )
	 {
	    m->image()->last_frame( pos );
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

int media_track::handle( int event )
{
   switch( event )
   {
      case fltk::PUSH:
	 {
	    _dragX = fltk::event_x();
	    int x = fltk::event_x();
	    int y = fltk::event_y();

	    if ( fltk::event_key() == fltk::MiddleButton )
	    {
	       return 0;
	    }
	    else
	    {
	       select_media( (x - _panX) / frame_size );
	    }
	    return 1;
	 }
      case fltk::DRAG:
	 {
	    if ( _selected )
	    {
	       MediaList::iterator i = _media.begin();
	       MediaList::iterator e = _media.end();
	       int diff = (fltk::event_x() - _dragX);
	       for ( ; i != e; ++i )
	       {
		  if ( *i == _selected )
		  {
		     shift_media_end( _selected, diff );
		     break;
		  }
	       }
	       redraw();
	    }
	    _dragX = fltk::event_x();
	    return 1;
	 }
   }

   return fltk::Widget::handle( event );
}

void media_track::draw()
{
   size_t e = _position.size();


   fltk::setcolor( fltk::GRAY33 );
   fltk::fillrect( x(), y(), w(), h() );
   fltk::push_clip( x(), y(), w(), h() );

   fltk::translate( _panX, 0 );

   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media& fg = _media[i];
      if (!fg) continue;

      int64_t pos = _position[i];

      int dx = pos * frame_size;
      int dw = fg->image()->duration() * frame_size; 

      fltk::setcolor( fltk::DARK_GREEN );
      fltk::fillrect( dx, y(), dw, h() );

      fltk::setcolor( fltk::BLACK );
      fltk::strokerect( dx, y(), dw, h() );


      fltk::setcolor( fltk::BLACK );
      if ( _selected == fg )
   	 fltk::setcolor( fltk::WHITE );

      int ww, hh;
      fltk::measure( fg->image()->name().c_str(), ww, hh );
      fltk::drawtext( fg->image()->name().c_str(),
   		      dx + dw/2 - ww/2, y() + frame_size*2-2 );
      
   }

   fltk::pop_clip();
}

}  // namespace mrv
