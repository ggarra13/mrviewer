
#include <fltk/draw.h>
#include <fltk/events.h>
#include <fltk/Cursor.h>
#include "gui/mrvMediaTrack.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvReelList.h"
#include "gui/mrvImageInformation.h"
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
   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return;

   if ( frame == MRV_NOPTS_VALUE )
   {
      size_t e = reel->images.size();
      if ( e < 2 )
      {
	 frame = 1;
      }
      else
      {
	 mrv::media o = reel->images[e-2];
	 frame = o->position();
	 frame += o->image()->duration();
      }
   }

   m->position( frame );

   mrv::media o = reel->images[ reel->images.size()-1 ];
   timeline()->maximum( frame + o->image()->duration() );

   redraw();
}

mrv::ImageBrowser* media_track::browser() const {
   return _main->uiReelWindow->uiBrowser;
}

mrv::media media_track::media_at_position( const boost::int64_t frame )
{
   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return mrv::media();
   size_t e = reel->images.size();

   for (size_t i = 0; i < e; ++i )
   {
      mrv::media m = reel->images[i];
      boost::int64_t start = m->position();
      boost::int64_t end   = m->position();
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
   redraw();
   return true;
}

// Move a media in track without changing its start or end frames
// Shift surrounding media to remain attached.
void media_track::shift_media( mrv::media m, boost::int64_t frame )
{
   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return;

   size_t e = reel->images.size();
   size_t idx = 0;

   for (size_t i = 0; i < e; ++i )
   {
      if ( m == reel->images[i] )
      {
	 idx = i;
	 reel->images[i]->position( frame );
	 break;
      }
   }

   // Shift medias that come after
   for (size_t i = idx+1; i < e; ++i )
   {
      mrv::media& fg = reel->images[i-1];
      boost::int64_t end = fg->position() + fg->image()->duration();
      reel->images[i]->position( end );
   }


   if ( idx == 0 ) {
      return;
   }

   // Shift medias that come before
   for (int i = int(idx)-1; i >= 0; --i )
   {
      boost::int64_t start = reel->images[i+1]->position();
      mrv::media& o = reel->images[i];
      boost::int64_t ee = o->position() + o->image()->duration();
      boost::int64_t ss = o->position();
      
      // Shift indexes of position
      reel->images[i]->position( start - (ee - ss ) );
   }

   return;
}

void media_track::shift_media_start( mrv::media m, boost::int64_t diff )
{
   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return;

   int idx = 0;
   size_t e = reel->images.size();
   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media fg = reel->images[i];
      if ( fg == m )
      {
	 idx = i;
	 int64_t newpos = m->position() + diff;
	 if ( newpos < m->position() + m->image()->duration() )
	 {
	    CMedia* img = m->image();
	    img->first_frame( img->first_frame() + diff );
	    img->seek( img->first_frame() );
	    main()->uiView->foreground( fg );
	 }
	 break;
      }
   }

   // Shift medias that come after
   for (size_t i = idx+1; i < e; ++i )
   {
      mrv::media& o = reel->images[i-1];
      boost::int64_t end = o->position() + o->image()->duration();
      mrv::media& fg = reel->images[i];
      fg->position( end );
   }


   if ( idx == 0 ) {
      return;
   }

   // Shift medias that come before
   for (int i = int(idx)-1; i >= 0; --i )
   {
      boost::int64_t start = reel->images[i+1]->position();
      mrv::media& o = reel->images[i];
      boost::int64_t ee = o->position() + o->image()->duration();
      boost::int64_t ss = o->position();
      
      // Shift indexes of position
      o->position( start - (ee - ss ) );
   }

}

bool media_track::select_media( const boost::int64_t pos )
{
   bool ok = false;
   _selected.reset();

   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return false;

   size_t e = reel->images.size();

   for ( size_t i = 0; i < e; ++i )
   {
      mrv::media fg = reel->images[i];
      if ( !fg ) continue;

      CMedia* img = fg->image();
      //int64_t start = img->first_frame() - img->start_frame() + 1;

      int64_t start = fg->position();
      int64_t duration = (int64_t)img->duration();

      if ( pos >= start && pos < start + duration)
      {
	 if ( pos < start + duration / 2 )
	 {
	    _at_start = true;
	 }
	 else
	 {
	    _at_start = false;
	 }

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
   if ( !reel ) return;

   size_t e = reel->images.size();
   size_t i = 0;
   for ( ; i < e; ++i )
   {
      mrv::media& fg = reel->images[i];
      if ( fg == m )
      {
	 int64_t pos = m->image()->last_frame() + diff;
	 if ( pos > m->image()->first_frame() &&
	      pos <= m->image()->end_frame() )
	 {
	    m->image()->last_frame( pos );
	    m->image()->seek( pos );

	    main()->uiImageInfo->uiInfoText->refresh();
	    main()->uiImageInfo->uiInfoText->redraw();
	    break;
	 }
      }
   }

   // Shift medias that come after
   for ( ; i < e-1; ++i )
   {
      boost::int64_t start = reel->images[i]->position();
      boost::int64_t ee = reel->images[i]->image()->duration();
      
      // Shift indexes of position
      reel->images[i+1]->position(start + ee );
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
	    main()->uiImageInfo->uiInfoText->refresh();
	    // main()->uiImageInfo->uiInfoText->redraw();
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
    // case fltk::KEY:
    //    {
    // 	  int key = fltk::event_key();
    // 	  if ( key == fltk::DeleteKey ||
    // 	       key == fltk::BackSpaceKey )
    // 	  {
    // 	     remove( _selected );
    // 	     return 1;
    // 	  }
    // 	  std::cerr << "track key " << key << std::endl;
    // 	  return fltk::Widget::handle( event );
    // 	  // return 0;
    //    }
      case fltk::DRAG:
	 {
	    if ( _selected )
	    {
	       cursor( fltk::CURSOR_WE );

	       const mrv::Reel& reel = browser()->current_reel();
	       if ( !reel ) return 0;
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

int64_t media_track::minimum() const
{
   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return MRV_NOPTS_VALUE;
   if ( reel->images.size() == 0 ) return MRV_NOPTS_VALUE;
   return reel->images[0]->position();
}

int64_t media_track::maximum() const
{
   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return MRV_NOPTS_VALUE;
   size_t e = reel->images.size() - 1;
   return reel->images[e]->position() + reel->images[e]->image()->duration();
}

void media_track::draw()
{
   const mrv::Reel& reel = browser()->current_reel();
   if ( !reel ) return;

   size_t e = reel->images.size();

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

      int64_t pos = fg->position();


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
	 int yh = y() + h();
	 if ( _at_start )
	 {
	    fltk::newpath();
	    fltk::addvertex( r.x(), y() );
	    fltk::addvertex( r.x(), yh );
	    fltk::addvertex( r.x() + dw/2, yh );
	    fltk::closepath();
	    fltk::fillpath();

	 }
	 else
	 {
	    fltk::newpath();
	    fltk::addvertex( r.x()+dw, y() );
	    fltk::addvertex( r.x()+dw, yh );
	    fltk::addvertex( r.x()+dw/2, yh );
	    fltk::closepath();
	    fltk::fillpath();
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
		      dx + dw/2 - ww/2, y() + 15 );

   }

   fltk::pop_clip();
}

}  // namespace mrv
