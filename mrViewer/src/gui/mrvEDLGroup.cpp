
#include <limits>

#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/Color.h>
#include <fltk/Window.h>

#include "gui/mrvMediaTrack.h"
#include "gui/mrvEDLGroup.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvElement.h"
#include "gui/mrvImageBrowser.h"

namespace mrv {

EDLGroup::EDLGroup(int x, int y, int w, int h) :
fltk::Browser(x,y,w,h),
_current_media_track( 0 ),
_dragX( 0 ),
_dragY( 0 )
{
}

EDLGroup::~EDLGroup()
{
   _audio_track.clear();
}

void EDLGroup::current_media_track( size_t i )
{
   if ( i >= children() )
      throw("Invalid index to current_media_track");
   _current_media_track = i;
   redraw();
}

// Add a media track and return its index
size_t EDLGroup::add_media_track( size_t r )
{
   size_t e = children();

   mrv::media_track* o = new mrv::media_track(x(), y() +
					      70 * e,
					      w(), 68);
 
   o->main( timeline()->main() );
   o->index( r );

   this->add( o );

   e = children() - 1;
   _current_media_track = e;
   
   return e;
}

// Add an audio only track and return its index
size_t EDLGroup::add_audio_track()
{
   // _audio_track.push_back( mrv::audio_track_ptr() );
   return _audio_track.size() - 1;
}

// Return the number of media tracks
size_t EDLGroup::number_of_media_tracks()
{
   return children();
}

// Return the number of audio only tracks
size_t EDLGroup::number_of_audio_tracks()
{
   return _audio_track.size();
}


// Return an audio track at index i
audio_track_ptr& EDLGroup::audio_track( int i )
{
   return _audio_track[i];
}

// Remove a media track at index i
void EDLGroup::remove_media_track( int i )
{
   remove( i );
}

// Remove an audio track at index i
void EDLGroup::remove_audio_track( int i )
{
   _audio_track.erase( _audio_track.begin() + i );
}

void EDLGroup::layout()
{
   fltk::Group::layout();
}

int EDLGroup::handle( int event )
{

   switch( event )
   {
      case fltk::PUSH:
	 {
	    if ( fltk::event_key() == fltk::MiddleButton )
	    {
	       _dragX = fltk::event_x();
	       return 1;
	    }

	    if ( fltk::event_key() == fltk::LeftButton )
	    {
	       _dragX = fltk::event_x();
	       _dragY = fltk::event_y();
	    }

	    int ok = fltk::Group::handle( event );
	    if ( ok ) return ok;

	    // if ( fltk::event_key() == fltk::LeftButton )
	    // {
	    //    mrv::media_track* t = (mrv::media_track*) child(0);
	    //    mrv::media m = t->media_at(1);
	    //    mrv::Element* e = ImageBrowser::new_item( m ); 
	    //    media_track::selected( e );
	    //    return 1;
	    // }
	    return 0;
	    break;
	 }
      case fltk::ENTER:
	 focus(this);
	 window()->show();
	 return 1;
	 break;
      case fltk::FOCUS:
	 return 1;
	 break;
      case fltk::KEY:
	 {
	    int key = fltk::event_key();
	    

	    if ( key == 'f' )
	    {
	       size_t i = 0;
	       size_t e = children();

	       int64_t tmin = std::numeric_limits<int64_t>::max();
	       int64_t tmax = std::numeric_limits<int64_t>::min();

	       for ( ; i != e; ++i )
	       {

		  mrv::media_track* o = (mrv::media_track*)child(i);
		  mrv::Element* e = o->selected();
		  if ( e )
		  {
		     mrv::media m = e->element();
		     int64_t tmi = m->position();
		     int64_t tma = m->position() + m->image()->duration();
		     if ( tmi < tmin ) tmin = tmi;
		     if ( tma > tmax ) tmax = tma;
		     break;
		  }
		  else
		  {
		     int64_t tmi = o->minimum();
		     int64_t tma = o->maximum();
		     if ( tmi < tmin ) tmin = tmi;
		     if ( tma > tmax ) tmax = tma;
		  }
	       }

	       mrv::Timeline* t = timeline();
	       t->minimum( tmin );
	       t->maximum( tmax );
	       t->redraw();
	       redraw();
	       return 1;
	    }
	 }
	 break;
      case fltk::MOUSEWHEEL:
	if ( fltk::event_dy() < 0.f )
	  {
	     zoom( 0.5 );
	  }
	else
	  {
	     zoom( 2.0 );
	  }
	return 1;
	break;
      case fltk::DRAG:
	 if ( fltk::event_key() == fltk::LeftButton )
	 {
	    _dragX = fltk::event_x();
	    _dragY = fltk::event_y();

	    parent()->redraw();
	 }
	 else if ( fltk::event_key() == fltk::MiddleButton )
	 {
	    int diff = ( fltk::event_x() - _dragX );
	    double amt = diff / (double) w();
	    mrv::Timeline* t = timeline();
	    double avg = t->maximum() - t->minimum() + 1;
	    amt *= avg;

	    t->minimum( t->minimum() - amt );
	    t->maximum( t->maximum() - amt );
	    t->redraw();
	    redraw();

	    _dragX = fltk::event_x();
	    return 1;
	 }
      default:
	 break;
   }

   return fltk::Group::handle( event );
}

void EDLGroup::zoom( double z )
{

   mrv::Timeline* t = timeline();

   double pct = (double) fltk::event_x() / (double)w();

   int64_t tmin = t->minimum();
   int64_t tmax = t->maximum();


   int64_t tdiff = tmax - tmin;
   int64_t tcur = tmin + pct * tdiff;


   tmax *= z;
   tmin *= z;

   int64_t tlen = tmax - tmin;

   tmax = tcur + ( 1.0 - pct ) * tlen;
   tmin = tcur - tlen * pct;


   if ( tmin < 0.0 ) tmin = 0.0;

   t->minimum( tmin );
   t->maximum( tmax );
   t->redraw();


   size_t i = 0;
   size_t e = children();
   for ( ; i != e; ++i )
   {
      mrv::media_track* c = (mrv::media_track*)child(i);
      c->zoom( z );
   }

}

void EDLGroup::refresh()
{
   for ( size_t i = 0; i < children(); ++i )
   {
      mrv::media_track* o = (mrv::media_track*) child(i);
      o->refresh();
   }
}

void EDLGroup::draw()
{

   fltk::setcolor( fltk::GRAY20 );
   fltk::fillrect( x(), y(), w(), h() );

   fltk::Group::draw();

   // mrv::Element* e = mrv::media_track::selected();
   // if ( e )
   // {
   //    fltk::push_matrix();
   //    fltk::translate( _dragX, _dragY );
   //    e->draw();
   //    int ww, hh;
   //    fltk::measure( e->label(), ww, hh );
   //    fltk::drawtext( e->label(), 0, 0 );
   //    fltk::pop_matrix();
   // }
}

} // namespace mrv


