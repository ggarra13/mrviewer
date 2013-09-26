#ifndef mrvMediaTrack_h
#define mrvMediaTrack_h

#include <boost/cstdint.hpp>
#include <fltk/Widget.h>

extern "C" {
#include <libavformat/avformat.h>
#define MRV_NOPTS_VALUE (int64_t) AV_NOPTS_VALUE
}

#include "mrvMedia.h"
#include "mrvMediaList.h"

namespace mrv {

class ViewerUI;

class media_track : public fltk::Widget
{
   public:
     typedef std::vector< boost::int64_t > Positions;

   public:
     media_track(int x, int y, int w, int h);
     ~media_track();


     // Add a media at a certain frame (or append to end by default)
     void add( mrv::media m, boost::int64_t frame = AV_NOPTS_VALUE );

     // Return a media based on its index
     mrv::media media_at( const unsigned i ) { 
	return _media[i];
     }

     // Return a media based on its position in the track
     mrv::media media_at_position( const boost::int64_t frame );

     // Remove a media from the track.  Returns true if element was removed.
     bool remove( mrv::media m );

     void main( mrv::ViewerUI* m ) { _main = m; }
     mrv::ViewerUI* main() const { return _main; }

     // Move a media in track without changing its start or end frames.
     // If media overlaps other media, everything is shifted
     void shift_media( mrv::media m, boost::int64_t frame );

     // Shift the media start changing its start value
     void shift_media_start( mrv::media m, boost::int64_t start );

     // Shift the media end changing its end value
     void shift_media_end( mrv::media m, boost::int64_t end );

     // Select the media 
     bool select_media( const boost::int64_t pos );

     void translate( double x ) { _panX += x; redraw(); }

     void zoom_factor( double f = 1.0 );

     virtual int handle( int event );
     virtual void draw();

   protected:
     mrv::ViewerUI* _main;
     int        _dragX;
     mrv::media _selected;
     MediaList  _media;
     Positions  _position;
     double     _panX;
     double     _zoom;
     CMedia::Playback _playback;
};

typedef boost::shared_ptr< media_track > media_track_ptr;


}



#endif  // mrvMediaTrack_h
