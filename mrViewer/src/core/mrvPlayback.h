/**
 * @file   mrvPlayback.h
 * @author gga
 * @date   Fri Jul  6 17:39:35 2007
 * 
 * @brief  This file implements a callback that is used to play videos or
 *         image sequences.  This callback is usually run as part of a new
 *         thread.
 * 
 * 
 */

#ifndef mrvPlayback_h
#define mrvPlayback_h

#include <cassert>


class CMedia;


namespace mrv {

  class ViewerUI;


  enum Playback {
    kBackwards = -1,
    kStopped = 0,
    kForwards = 1,
    kScrubbing = 255
  };

  //
  // Callback data that must be filled
  //
  struct PlaybackData {
       bool fg;
       mrv::ViewerUI*   uiMain;
       CMedia*  image;
       CMedia*  bg;

       PlaybackData( bool foreground, mrv::ViewerUI* const main,
		     CMedia* const img, CMedia* background = NULL ) :
       fg( foreground ),
       uiMain( main ),
       image( img ),
       bg( background )
       {
	  assert( image  != NULL );
       }

       PlaybackData( const PlaybackData& b ) :
       fg( b.fg ),
       uiMain( b.uiMain ),
       image( b.image ),
       bg( b.bg )
       {
	  assert( image != NULL );
       }

      ~PlaybackData()
      {
      }

  };

  void audio_thread( PlaybackData* data );
  void video_thread( PlaybackData* data );
  void subtitle_thread( PlaybackData* data );
  void decode_thread( PlaybackData* data );


} // namespace mrv


#endif // mrvPlayback_h
