
#include "mrvMediaTrack.h"
#include "mrvEDLGroup.h"
#include "mrvTimeline.h"

#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/Color.h>

namespace mrv {

EDLGroup::EDLGroup(int x, int y, int w, int h) :
_current_media_track( 0 ),
fltk::Group(x,y,w,h)
{
}

EDLGroup::~EDLGroup()
{
   _media_track.clear();
   _audio_track.clear();
}

// Add a media track and return its index
size_t EDLGroup::add_media_track()
{
   _media_track.push_back( mrv::media_track_ptr( new 
						 mrv::media_track(x(), y(),
								  w(), 30) ) );
   _media_track.back()->main( timeline()->main() );
   return _media_track.size() - 1;
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
   return _media_track.size();
}

     // Return the number of audio only tracks
size_t EDLGroup::number_of_audio_tracks()
{
   return _audio_track.size();
}

// Return a media track at index i
media_track_ptr& EDLGroup::media_track( int i )
{
   return _media_track[i];
}

// Return an audio track at index i
audio_track_ptr& EDLGroup::audio_track( int i )
{
   return _audio_track[i];
}

// Remove a media track at index i
void EDLGroup::remove_media_track( int i )
{
   _media_track.erase( _media_track.begin() + i );
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
	 if ( fltk::event_key() == fltk::MiddleButton )
	 {
	    _dragX = fltk::event_x();
	    return 1;
	 }
	 break;
      case fltk::MOUSEWHEEL:
	if ( fltk::event_dy() < 0.f )
	  {
	     zoom( 0.5f );
	  }
	else
	  {
	     zoom( 2.0f );
	  }
	 break;
      case fltk::DRAG:
	 if ( fltk::event_key() == fltk::MiddleButton )
	 {
	    int diff = ( fltk::event_x() - _dragX );
	    double amt = diff / (double) w();
	    mrv::Timeline* t = timeline();
	    double avg = t->maximum() - t->minimum() + 1;
	    amt *= avg;

	    t->minimum( t->minimum() - amt );
	    t->maximum( t->maximum() - amt );
	    t->redraw();

	    MediaTrack::iterator i = _media_track.begin();
	    MediaTrack::iterator e = _media_track.end();
	    for ( ; i != e; ++i )
	    {
	       (*i)->translate( amt );
	    }
	    _dragX = fltk::event_x();
	    return 1;
	 }
      default:
	 break;
   }



   MediaTrack::iterator i = _media_track.begin();
   MediaTrack::iterator e = _media_track.end();
   for ( ; i != e; ++i )
   {
      int ret = (*i)->handle( event );
      if ( ret ) return ret;
   }

   return fltk::Group::handle( event );
}

void EDLGroup::zoom( double z )
{

   mrv::Timeline* t = timeline();
   t->minimum( t->minimum() * z );
   t->maximum( t->maximum() * z );
   t->redraw();


   MediaTrack::iterator i = _media_track.begin();
   MediaTrack::iterator e = _media_track.end();
   for ( ; i != e; ++i )
   {
      (*i)->zoom( z );
   }

}

void EDLGroup::draw()
{

   fltk::setcolor( fltk::GRAY20 );
   fltk::fillrect( x(), y(), w(), h() );

   fltk::Group::draw();

   // fltk::load_identity();

   MediaTrack::iterator i = _media_track.begin();
   MediaTrack::iterator e = _media_track.end();
   for ( ; i != e; ++i )
   {
      (*i)->draw();
   }


}

}


