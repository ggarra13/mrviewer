#ifndef mrvEDLGroup_h
#define mrvEDLGroup_h

#include <fltk/Group.h>
#include "mrvMediaTrack.h"


namespace mrv {

class Timeline;
class audio_track;
typedef audio_track* audio_track_ptr;

class EDLGroup : public fltk::Group
{
   public:
     typedef std::vector< media_track_ptr >  MediaTrack;
     typedef std::vector< audio_track_ptr >  AudioTrack;

   public:

     EDLGroup(int x, int y, int w, int h);
     ~EDLGroup();

     // Add a media track and return its index
     size_t add_media_track();

     // Add an audio only track and return its index
     size_t add_audio_track();

     // Return the number of media tracks
     size_t number_of_media_tracks();

     // Return the number of audio only tracks
     size_t number_of_audio_tracks();

     media_track_ptr& current_media_track() { 
	return _media_track[_current_media_track]; 
     }

     // Return a media track at index i
     media_track_ptr& media_track( int i );

     // Return an audio track at index i
     audio_track_ptr& audio_track( int i );

     // Remove a media track at index i
     void remove_media_track( int i );

     // Remove an audio track at index i
     void remove_audio_track( int i );

     void timeline( mrv::Timeline* t ) { _timeline = t; }
     mrv::Timeline* timeline() const { return _timeline; }

     virtual int  handle( int event );
     virtual void layout();
     virtual void draw();

   protected:
     int        _dragX;
     size_t     _current_media_track;
     MediaTrack _media_track;
     AudioTrack _audio_track;
     mrv::Timeline* _timeline;
};

}

#endif  // mrvEDLGroup_h

