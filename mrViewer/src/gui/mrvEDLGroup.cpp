
#include "mrvMediaTrack.h"
#include "mrvEDLGroup.h"

#include <fltk/draw.h>
#include <fltk/Color.h>

namespace mrv {

EDLGroup::EDLGroup(int x, int y, int w, int h) :
_current_media_track( 0 ),
fltk::Group(x,y,w,h)
{
   size_t idx = add_media_track();
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

   MediaTrack::iterator i = _media_track.begin();
   MediaTrack::iterator e = _media_track.end();
   for ( ; i != e; ++i )
   {
      int ret = (*i)->handle( event );
      if ( ret ) return ret;
   }

   return fltk::Group::handle( event );
}



void EDLGroup::draw()
{
   fltk::Group::draw();

   fltk::setcolor( fltk::GRAY20 );
   fltk::fillrect( 0, 0, w(), h() );

   MediaTrack::iterator i = _media_track.begin();
   MediaTrack::iterator e = _media_track.end();
   for ( ; i != e; ++i )
   {
      (*i)->draw();
   }

}

}


