/**
 * @file   mrvPlayback.cpp
 * @author gga
 * @date   Fri Jul  6 17:43:07 2007
 * 
 * @brief  This file implements a callback that is used to play videos or
 *         image sequences.  This callback is usually run as part of a new
 *         thread.
 * 
 * 
 */

#include <cstdio>

#include <iostream>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_array.hpp>
using namespace std;

#include "core/CMedia.h"
#include "core/aviImage.h"
#include "core/mrvMath.h"
#include "core/mrvTimer.h"
#include "core/mrvThread.h"

#include "gui/mrvIO.h"
#include "gui/mrvReel.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "mrViewer.h"

#include "mrvPlayback.h"


namespace 
{
  const char* kModule = "play";
}

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0


#if 0
#  define DEBUG_DECODE
#  define DEBUG_VIDEO
#  define DEBUG_AUDIO
#endif

// #define DEBUG_THREADS


#if defined(WIN32) || defined(WIN64)

struct timespec {
   time_t tv_sec;        /* seconds */
   long   tv_nsec;       /* nanoseconds */
};

void nanosleep( const struct timespec* req, struct timespec* rem )
{
   Sleep( DWORD(1000 * req->tv_sec + req->tv_nsec / 1e7f) );
}

#endif



namespace mrv {


  enum EndStatus {
  kEndIgnore,
  kEndStop,
  kEndNextImage,
  kEndChangeDirection,
  kEndLoop,
  };


  unsigned int barrier_thread_count( const CMedia* img )
  {
    unsigned r = 1;               // 1 for decode thread
    if    ( img->has_picture() )  r += 1;
    if    ( img->has_audio() )    r += 1;
    if    ( img->has_subtitle() ) r += 1;
    return r;
  }
  

EndStatus handle_loop( boost::int64_t& frame,
		       int&     step,
		       CMedia* img, 
		       bool    fg,
		       mrv::ViewerUI* uiMain,
		       const mrv::Reel  reel,
		       const mrv::Timeline* timeline,
		       const mrv::CMedia::DecodeStatus end )
{

   mrv::ImageView* view = uiMain->uiView;

   EndStatus status = kEndIgnore;
   CMedia* next = NULL;
   
   boost::int64_t last = ( int64_t ) timeline->maximum();
   boost::int64_t first = ( int64_t ) timeline->minimum();


   if ( img->last_frame() < last )
      last = img->last_frame();
   if ( img->first_frame() > first )
      first = img->first_frame();


   ImageView::Looping loop = view->looping();


   switch( end )
   {
      case CMedia::kDecodeLoopEnd:
	 {

	    if ( reel->edl )
	    {
	       boost::int64_t f = frame;

	       CMedia::Mutex& m = img->video_mutex();
	       SCOPED_LOCK( m );


	       f -= img->first_frame();
	       f += reel->location(img);

	       if ( f <= timeline->maximum() )
	       {
		  next = reel->image_at( f );
	       }


	       if ( next )
	       {
		  f = reel->global_to_local( f );
	       }
	       else
	       {
		  if ( loop == ImageView::kLooping )
		  {
		     f = boost::int64_t(timeline->minimum());
		     next = reel->image_at( f );
		     f = reel->global_to_local( f );
		  }
		  else
		  {
		     next = img;
		  }
	       }

	       assert( next != NULL );

	       if ( next != img && next != NULL) 
	       {
		  CMedia::Mutex& m2 = next->video_mutex();
		  SCOPED_LOCK( m2 );

		  img->playback( CMedia::kStopped );

		  next->seek( f );
		  next->play( CMedia::kForwards, uiMain, fg );
		  status = kEndNextImage;
		  break;
	       }
	    }

	    if ( frame > last )
	    {
	       if ( loop == ImageView::kLooping )
	       {
		  frame = first;
		  status = kEndLoop;
	       }
	       else if ( loop == ImageView::kPingPong )
	       {
		  frame = last;
		  step  = -1;
		  view->playback( ImageView::kBackwards );
		  img->frame( frame );
		  img->audio_frame( frame );
		  img->playback( CMedia::kBackwards );
		  status = kEndChangeDirection;
	       }
	       else
	       {
		  view->playback( ImageView::kStopped );
		  img->playback( CMedia::kStopped );
	       }
	    }
	    break;
	 }
      case CMedia::kDecodeLoopStart:
	 {
	    if ( reel->edl )
	    {
	       boost::int64_t f = frame;

	       CMedia::Mutex& m = img->video_mutex();
	       SCOPED_LOCK( m );

	       f -= img->first_frame();
	       f += reel->location(img);

	       if ( f >= timeline->minimum() )
	       {
		  next = reel->image_at( f );
	       }

	       f = reel->global_to_local( f );
	       if ( !next )
	       {
		  if ( loop == ImageView::kLooping )
		  {
		     f = boost::int64_t( timeline->maximum() );
		     next = reel->image_at( f );
		     f = reel->global_to_local( f );
		  }
		  else
		  {
		     next = img;
		  }
	       }

	       assert( next != NULL );

	       if ( next != img && next != NULL )
	       {
		  CMedia::Mutex& m2 = next->video_mutex();
		  SCOPED_LOCK( m2 );
		  img->playback( CMedia::kStopped );

		  next->seek( f );
		  next->play( CMedia::kBackwards, uiMain, fg );
		  status = kEndNextImage;
		  break;
	       }
	    }


	    if ( frame < first )
	    {
	       if ( loop == ImageView::kLooping )
	       {
		  frame = last;
		  status = kEndLoop;
	       }
	       else if ( loop == ImageView::kPingPong )
	       {
		  frame = first;
		  step = 1;
		  view->playback( ImageView::kForwards );
		  img->frame( frame );
		  img->audio_frame( frame );
		  img->playback( CMedia::kForwards );
		  status = kEndChangeDirection;
	       }
	       else
	       {
		  img->playback( CMedia::kStopped );
		  view->playback( ImageView::kStopped );
	       }
	    }
	    break;
	 }
      default:
	 {
	    // error
	 }
   }

   if ( status == kEndStop || status == kEndNextImage )
   {
      img->playback( CMedia::kStopped );
   }

   return status;
}



CMedia::DecodeStatus check_loop( const int64_t frame,
				 CMedia* img,
				 mrv::Reel reel,
				 mrv::Timeline* timeline )
{
   boost::int64_t last = boost::int64_t( timeline->maximum() );
   boost::int64_t first = boost::int64_t( timeline->minimum() );

   boost::int64_t f = frame;

   CMedia::Mutex& m = img->video_mutex();
   SCOPED_LOCK( m );

   if ( reel->edl )
   {
      boost::int64_t s = reel->location(img);
      boost::int64_t e = s + img->duration() - 1;

      if ( e < last )  last = e;
      if ( s > first ) first = s;

      last = reel->global_to_local( last );
      first = reel->global_to_local( first );
   }
   else
   {
      if ( last > img->last_frame() )
	 last = img->last_frame();
      else if ( img->first_frame() < first )
	 first = img->first_frame();
   }

   if ( f > last )
   {
      img->loop_at_end( last+1 );
      return CMedia::kDecodeLoopEnd;
   }
   else if ( f < first )
   {
      img->loop_at_start( first-1 );
      return CMedia::kDecodeLoopStart;
   }

   return CMedia::kDecodeOK;
}

//
// Main loop used to play audio (of any image)
//
void audio_thread( PlaybackData* data )
{
   assert( data != NULL );

   mrv::ViewerUI*     uiMain   = data->uiMain;
   assert( uiMain != NULL );
   CMedia* img = data->image;
   assert( img != NULL );
   bool fg = data->fg;
   CMedia* bg = data->bg;

   // delete the data (we don't need it anymore)
   delete data;

   int64_t frame = img->frame();
   

   int64_t failed_frame = std::numeric_limits< int64_t >::min();


   mrv::ImageView*      view = uiMain->uiView;
   mrv::Timeline*      timeline = uiMain->uiTimeline;
   mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;


  int idx = fg ? view->fg_reel() : view->bg_reel();
   if ( idx < 0 ) return;

   mrv::Reel   reel = browser->reel_at( idx );
   if (!reel) return;

#ifdef DEBUG_THREADS
   cerr << "ENTER " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? " << img->stopped()
	<< " frame " << frame << endl;
#endif

   mrv::Timer timer;

   while ( !img->stopped() )
   {

      int step = (int) img->playback();
      if ( step == 0 ) break;

      assert( view->playback() == mrv::ImageView::kStopped ||
              img->playback() == view->playback() );

      img->wait_audio();

      CMedia::DecodeStatus status = img->decode_audio( frame );


      if ( frame > img->last_frame() )
         status = CMedia::kDecodeLoopEnd;
      if ( frame < img->first_frame() )
         status = CMedia::kDecodeLoopStart;

      switch( status )
      {
	 case CMedia::kDecodeError:
	    LOG_ERROR( _("Decode Error audio frame ") << frame );
	    frame += step;
	    continue;
	 case CMedia::kDecodeMissingFrame:
	    LOG_ERROR( _("Decode Missing audio frame ") << frame );
	    timer.setDesiredFrameRate( img->play_fps() );
	    timer.waitUntilNextFrameIsDue();
	    frame += step;
	    continue;
	 case  CMedia::kDecodeLoopEnd:
	 case  CMedia::kDecodeLoopStart:
	    {

	       if (! img->aborted() && !img->has_picture() )
	       {
		  EndStatus end = handle_loop( frame, step, img, fg, uiMain, 
					       reel, timeline, status );

		  if ( end == kEndIgnore )
		  {
		     break;
		  }

	       }

	       CMedia::Barrier* barrier = img->loop_barrier();
	       // Wait until all threads loop and decode is restarted
	       barrier->wait();
	       continue;
	    }
	 case CMedia::kDecodeOK:
	    break;
	 default:
	    break;
      }

      if ( ! img->has_audio() )
      {
	 // if audio was turned off, follow video.
	 // audio can be turned off due to user changing channels
	 // or due to a problem with audio engine.
	 frame = img->frame();
	 continue;
      }



      if ( img->has_audio() && reel->edl )
      { 
	 int64_t f = frame + reel->location(img) - img->first_frame();
	 if ( f > timeline->maximum() )
	    f = int64_t( timeline->maximum() );
	 if ( f < timeline->minimum() )
	    f = int64_t( timeline->minimum() );
	    timeline->value( double( f ) );
      }

      img->find_audio(frame);
      frame += step;
   }

#ifdef DEBUG_THREADS
   cerr << "EXIT " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? " 
	<< img->stopped()  
	<< " frame " << img->audio_frame() << endl;
#endif

} // audio_thread



  //
  // Main loop used to decode subtitles
  //
void subtitle_thread( PlaybackData* data )
{
   assert( data != NULL );

   CMedia* img = data->image;
   assert( img != NULL );

   // delete the data (we don't need it anymore)
   delete data;



    mrv::Timer timer;

#ifdef DEBUG_THREADS
    cerr << "ENTER SUBTITLE THREAD " << img->name() << endl;
#endif


    while ( !img->stopped() )
      {
	int step = (int) img->playback();
	if ( step == 0 ) break;

	int64_t frame = img->frame();
	CMedia::DecodeStatus status = img->decode_subtitle( frame );

	if ( frame > img->last_frame() )
	   status = CMedia::kDecodeLoopEnd;
	if ( frame < img->first_frame() )
	   status = CMedia::kDecodeLoopStart;

	switch( status )
	  {
	  case CMedia::kDecodeError:
	  case CMedia::kDecodeMissingFrame:
	  case CMedia::kDecodeMissingSamples:
	  case CMedia::kDecodeDone:
	  case CMedia::kDecodeNoStream:
	  case CMedia::kDecodeOK:
	    img->find_subtitle( frame );
	    break;
	  case CMedia::kDecodeLoopEnd:
	  case CMedia::kDecodeLoopStart:

	    CMedia::Barrier* barrier = img->loop_barrier();
	    // Wait until all threads loop and decode is restarted
	    barrier->wait();
	    continue;
	  }
	
	double fps = img->play_fps();
	timer.setDesiredFrameRate( fps );
	timer.waitUntilNextFrameIsDue();

      }


#ifdef DEBUG_THREADS
    cerr << "EXIT  SUBTITLE THREAD " << img->name() 
	 << " stopped? " << img->stopped() << endl;
#endif

  }  // subtitle_thread


//
// Main loop used to play video (of any image)
//
void video_thread( PlaybackData* data )
{
   assert( data != NULL );

   mrv::ViewerUI*     uiMain   = data->uiMain;
   assert( uiMain != NULL );
   CMedia* img = data->image;
   assert( img != NULL );
   bool fg = data->fg;
   CMedia* bg = data->bg;

   mrv::ImageView*      view = uiMain->uiView;
   mrv::Timeline*      timeline = uiMain->uiTimeline;
   mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;

   int idx = fg ? view->fg_reel() : view->bg_reel();
   if ( idx < 0 ) {
      LOG_ERROR( "REEL INDEX IS NEGATIVE FOR FG? " << fg );
      return;
   }
   mrv::Reel   reel = browser->reel_at( idx );
   if (!reel) return;

   int64_t frame        = img->frame();
   int64_t failed_frame = std::numeric_limits< int64_t >::min();

#ifdef DEBUG_THREADS
   cerr << "ENTER " << (fg ? "FG" : "BG") << " VIDEO THREAD " << img->name() << " stopped? " << img->stopped()
	<< " frame " << frame << endl;
#endif

   // delete the data (we don't need it anymore)
   delete data;

   bool skip = false;

   mrv::Timer timer;
   double fps = img->play_fps();
   timer.setDesiredFrameRate( fps );

   while ( !img->stopped() )
   {
      if ( !skip )
	 img->wait_image();

      // img->debug_video_packets( frame, "PLAYBACK", true );
      // img->debug_video_stores( frame, "PLAYBACK" );

      int step = (int) img->playback();
      if ( step == 0 ) break;
      assert( view->playback() == mrv::ImageView::kStopped ||
              img->playback() == view->playback() );

      CMedia::DecodeStatus status;

      DBG( img->name() << "  PRE    FRAME " << frame  );
      {
	 status = img->decode_video( frame );
      }
      DBG( img->name() << "  STATUS FRAME " << frame  );

      if ( frame > img->last_frame() )
         status = CMedia::kDecodeLoopEnd;
      if ( frame < img->first_frame() )
         status = CMedia::kDecodeLoopStart;

      DBG( img->name() << "  STATUS " << status );

      switch( status )
      {
	 // case CMedia::DecodeDone:
	 //    continue;
	 case CMedia::kDecodeBufferFull:
	 case CMedia::kDecodeError:
	 case CMedia::kDecodeMissingFrame:
            break;
	 case CMedia::kDecodeLoopEnd:
	 case CMedia::kDecodeLoopStart:
	    {
	       if (! img->aborted() )
	       {
		  EndStatus end = handle_loop( frame, step, img, fg, uiMain, 
					       reel, timeline, status );


		  if ( end == kEndIgnore )
		  {
                     skip = true;
		     if ( img->has_audio() )
			frame = img->audio_frame();
		     break;
		  }

	       }

	       skip = false;

	       CMedia::Barrier* barrier = img->loop_barrier();
	       // Wait until all threads loop and decode is restarted
	       barrier->wait();
	       continue;
	    }
	 default:
	    break;
      }

      fps = img->play_fps();

      double delay = 1.0 / fps;

      double diff = 0.0;

      // // Calculate video-audio difference
      if ( img->has_audio() && status == CMedia::kDecodeOK )
      {
	 //double video_clock = img->video_pts();
	 //double audio_clock = img->audio_pts();

	 double video_clock = img->video_clock();
	 double audio_clock = img->audio_clock();
	 diff = step * (video_clock - audio_clock);



	 if ( diff > 1000.0 ) diff = 0.0;

	 img->avdiff( diff );

	 double absdiff = std::abs(diff);


	 // Skip or repeat the frame. Take delay into account
	 //    FFPlay still doesn't "know if this is the best guess."
	 double sync_threshold = delay;
	 if(absdiff < AV_NOSYNC_THRESHOLD) {
	    double sdiff = step * diff;

	    if (sdiff <= -sync_threshold) {
	       fps = 99999999.0;
	    } else if (sdiff >= delay*2) {
	       fps -= sdiff;
	    }
	 }
      }


      timer.setDesiredFrameRate( fps );
      timer.waitUntilNextFrameIsDue();

      img->real_fps( timer.actualFrameRate() );

      DBG( img->name() << " FIND IMAGE   " << frame );
      bool ok = img->find_image( frame );
      DBG( img->name() << " FOUND IMAGE? " << ok );
      if ( !img->has_audio() && reel->edl )
      {
	 int64_t f = frame + reel->location(img) - img->first_frame();


	 if ( fg )
	 {
            DBG( "FRAME " << frame << " f " << f );
	    assert( f <= timeline->maximum() );
	    assert( f >= timeline->minimum() );

	    view->frame( f );
	 }
      }

      frame += step;
   }


#ifdef DEBUG_THREADS
   cerr << "EXIT  " << (fg ? "FG" : "BG") << " VIDEO THREAD " 
	<< img->name() << " stopped? " << img->stopped()
	<< " at " << frame << "  img->frame: " << img->frame() << endl;
#endif

}  // video_thread





void decode_thread( PlaybackData* data )
{
   assert( data != NULL );

   mrv::ViewerUI*     uiMain   = data->uiMain;
   assert( uiMain != NULL );


   CMedia* img = data->image;
   assert( img != NULL );

   bool fg = data->fg;
   CMedia* bg = data->bg;

   mrv::ImageView*      view = uiMain->uiView;
   mrv::Timeline*      timeline = uiMain->uiTimeline;
   mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;
   assert( timeline != NULL );

   int idx = fg ? view->fg_reel() : view->bg_reel();
   if ( idx < 0 ) return;

   mrv::Reel   reel = browser->reel_at( idx );
   if (!reel) return;


   int step = (int) img->playback();

   DBG( "img playback=" << img->playback() 
        << " view playback=" << view->playback() );
   assert( view->playback() == mrv::ImageView::kStopped ||
           img->playback() == view->playback() );

   int64_t frame = img->dts();

#ifdef DEBUG_THREADS
   cerr << "ENTER " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped() << " frame " << frame 
	<< " step " << step << endl;
#endif

   // delete the data (we don't need it anymore)
   delete data;






   while( !img->stopped() )
   {

      if ( img->seek_request() )
      {
	 img->do_seek();
	 frame = img->dts();
      }

      step = (int) img->playback();
      if ( step == 0 ) break;

      assert( img->playback() == CMedia::kStopped ||
              img->playback() == view->playback() );

      frame += step;
      
      CMedia* next = NULL;
      CMedia::DecodeStatus status = check_loop( frame, img, reel, timeline );
      if ( status != CMedia::kDecodeOK )
      {
	 // {
	 //    CMedia::Barrier* barrier = img->bg_barrier();
	 //    if ( fg )
	 //    {
	 //       barrier->wait();
	 //    }
	 //    else
	 //    {
	 //       barrier->notify_all();
	 //    }
	 // }

	 // Lock thread until loop status is resolved on all threads
	 CMedia::Barrier* barrier = img->loop_barrier();
	 int thread_count = barrier_thread_count( img );
	 barrier->count( thread_count );
	 // Wait until all threads loop or exit
	 barrier->wait();

	 if ( ! img->aborted() )
	 {
	    // Do the looping, taking into account ui state
	    // and return new frame and step.
	    EndStatus end = handle_loop( frame, step, img, fg,
					 uiMain, reel, timeline, status );
	 }

	 if ( img->stopped() ) break;
	
      }
      

 
      // If we could not get a frame (buffers full, usually),
      // wait a little.
      if ( !img->frame( frame ) )
      {
	 timespec req;
	 req.tv_sec = 0;
	 req.tv_nsec = (long)( 10 * 1e7f); // 10 ms.
	 nanosleep( &req, NULL );
      }
	   
      // After read, when playing backwards or playing audio files,
      // decode position may be several frames advanced as we buffer
      // multiple frames, so get back the dts frame from image.
      if ( img->has_video() || img->has_audio() )
      {
	 frame = img->dts();
      }
      

   }

#ifdef DEBUG_THREADS
   cerr << "EXIT  " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped() << " frame " 
	<< img->frame() << "  dts: " << img->dts() << endl;
#endif

}



} // namespace mrv
