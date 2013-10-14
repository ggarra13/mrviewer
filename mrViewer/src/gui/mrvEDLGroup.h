#ifndef mrvEDLGroup_h
#define mrvEDLGroup_h

#include <fltk/Browser.h>

#include "mrvMediaTrack.h"

namespace mrv {

class Timeline;
class Element;
class audio_track;
typedef audio_track* audio_track_ptr;

class EDLGroup : public fltk::Group
{
   public:
     typedef std::vector< audio_track_ptr >  AudioTrack;

   public:

     EDLGroup(int x, int y, int w, int h);
     ~EDLGroup();

     // Add a media track and return its index
     size_t add_media_track( size_t reel_idx );

     // Add an audio only track and return its index
     size_t add_audio_track();

     // Return the number of media tracks
     size_t number_of_media_tracks();

     // Return the number of audio only tracks
     size_t number_of_audio_tracks();

     void current_media_track( size_t i );

     mrv::media_track* current_media_track() { 
	return (mrv::media_track*)this->child(_current_media_track); 
     }

     // Return a media track at index i
     mrv::media_track* media_track( int i )
     {
	return (mrv::media_track*)this->child(i); 
     }

     // Return an audio track at index i
     audio_track_ptr& audio_track( int i );

     // Remove a media track at index i
     void remove_media_track( int i );

     // Remove an audio track at index i
     void remove_audio_track( int i );

     void timeline( mrv::Timeline* t ) { _timeline = t; }
     mrv::Timeline* timeline() const { return _timeline; }

     void zoom( double x );

     void refresh();

     virtual int  handle( int event );
     virtual void layout();
     virtual void draw();

   protected:
     double     _zoom;
     mrv::Element* _drag;
     int        _dragX;
     int        _dragY;
     size_t     _current_media_track;
     AudioTrack _audio_track;
     mrv::Timeline* _timeline;
};

}

#endif  // mrvEDLGroup_h

