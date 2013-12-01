#ifndef mrvMediaTrack_h
#define mrvMediaTrack_h

#include <boost/cstdint.hpp>
#include <fltk/Group.h>

extern "C" {
#include <libavformat/avformat.h>
#define MRV_NOPTS_VALUE (int64_t) AV_NOPTS_VALUE
}


#include "mrvMedia.h"
#include "mrvMediaList.h"

namespace mrv {

class ViewerUI;
class ImageBrowser;
class Timeline;
class Element;

class media_track : public fltk::Group
{
   public:
     media_track(int x, int y, int w, int h);
     ~media_track();

     double frame_size() const;

     void reel( size_t r ) { _reel_idx = r; redraw(); }
     size_t reel() const  { return _reel_idx; }

     void zoom( double x );

     // Add a media at a certain frame (or append to end by default)
     void add( mrv::media m, boost::int64_t frame = AV_NOPTS_VALUE );

     // Replace a media at a certain frame.
     void insert( const boost::int64_t frame, mrv::media m );

     // Return an image index for a frame or -1 if no image found
     int index_at( const boost::int64_t frame );

     // Return first image index for a media or -1 if not found
     int index_for( const mrv::media m );

     // Return first image index for a media or -1 if not found
     int index_for( const std::string s );

     // Return a media based on its index in the track
     mrv::media media( unsigned idx );

     // Return a media based on its position in the track
     mrv::media media_at( const boost::int64_t frame );

     // Remove a media from the track.  Returns true if element was removed.
     bool remove( const int idx );

     // Remove a media from the track.  Returns true if element was removed.
     bool remove( const mrv::media m );


     void main( mrv::ViewerUI* m ) { _main = m; }
     mrv::ViewerUI* main() const { return _main; }

     mrv::ImageBrowser* browser() const;

     mrv::Timeline* timeline() const;
     
     // void index( size_t idx ) { _reel_idx = idx; }

     // Move a media in track without changing its start or end frames.
     // If media overlaps other media, everything is shifted
     void shift_media( mrv::media m, boost::int64_t frame );

     // Shift the media start changing its start value
     void shift_media_start( mrv::media m, boost::int64_t start );

     // Shift the media end changing its end value
     void shift_media_end( mrv::media m, boost::int64_t end );

     // Select the media 
     bool select_media( const boost::int64_t pos );

     int64_t minimum() const;
     int64_t maximum() const;


     static void selected( mrv::Element* e ) { _selected = e; }
     static mrv::Element* selected() { return _selected; }

     void refresh();

     virtual int handle( int event );
     virtual void draw();

   protected:
     mrv::ViewerUI* _main;
     size_t     _reel_idx;
     int        _dragX;
     bool       _at_start;
     static  mrv::Element* _selected;

     // static mrv::media _selected; 
     double     _zoom;
     CMedia::Playback _playback;
     int64_t    _frame;
};



}



#endif  // mrvMediaTrack_h
