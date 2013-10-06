
#include <limits>

#include "mrvMediaTrack.h"
#include "mrvEDLGroup.h"
#include "mrvTimeline.h"

#include <fltk/events.h>
#include <fltk/draw.h>
#include <fltk/Color.h>
#include <fltk/Window.h>

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
	 return 0;
	 break;
      case fltk::ENTER:
	 focus(this);
	 break;
      case fltk::FOCUS:
	 return 1;
	 break;
      case fltk::KEY:
	 {
	    int key = fltk::event_key();
	    

	    if ( key == 'f' )
	    {
	       MediaTrack::iterator i = _media_track.begin();
	       MediaTrack::iterator e = _media_track.end();

	       int64_t tmin = std::numeric_limits<int64_t>::max();
	       int64_t tmax = std::numeric_limits<int64_t>::min();

	       for ( ; i != e; ++i )
	       {
		  int64_t tmi = (*i)->minimum();
		  int64_t tma = (*i)->maximum();
		  if ( tmi < tmin ) tmin = tmi;
		  if ( tma > tmax ) tmax = tma;
	       }

	       mrv::Timeline* t = timeline();
	       t->minimum( tmin );
	       t->maximum( tmax );
	       t->redraw();
	       redraw();
	       return 1;
	    }
	    return 0;
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

   fltk::Group::handle( event );
   return 1;
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

   MediaTrack::iterator i = _media_track.begin();
   MediaTrack::iterator e = _media_track.end();
   for ( ; i != e; ++i )
   {
      (*i)->draw();
   }


}

}


