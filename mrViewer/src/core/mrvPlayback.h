/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2014  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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

       PlaybackData( bool foreground, mrv::ViewerUI* const main,
		     CMedia* const img ) :
       fg( foreground ),
       uiMain( main ),
       image( img )
       {
	  assert( image  != NULL );
       }

       PlaybackData( const PlaybackData& b ) :
       fg( b.fg ),
       uiMain( b.uiMain ),
       image( b.image )
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
