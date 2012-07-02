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
#include "gui/mrvTimeline.h"
#include "gui/mrvImageView.h"
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
    kEndStop,
    kEndNextImage,
    kEndChangeDirection,
    kEndLoop,
  };

  enum CheckStatus {
    kNoChange,
    kLoopAtStart,
    kLoopAtEnd
  };

  unsigned int barrier_thread_count( const CMedia* img )
  {
    unsigned r = 1;               // 1 for decode thread
    if    ( img->has_picture() )  r += 1;
    if    ( img->has_audio() )    r += 1;
    if    ( img->has_subtitle() ) r += 1;
    return r;
  }
  

  EndStatus handle_loop( int64_t& frame,
			 int&     step,
			 CMedia* img, 
			 mrv::ViewerUI* uiMain,
			 const mrv::Timeline* timeline,
			 const CheckStatus end )
  {
    mrv::ImageView* view = uiMain->uiView;

    EndStatus status = kEndStop;
    CMedia* next = NULL;
 
    boost::int64_t offset = timeline->offset( img );
    boost::int64_t last = ( int64_t ) timeline->maximum();
    boost::int64_t first = ( int64_t ) timeline->minimum();


    if ( img->last_frame() < last )
       last = img->last_frame();
    if ( img->first_frame() > first )
       first = img->first_frame();


    ImageView::Looping loop = view->looping();


    switch( end )
      {
      case kLoopAtEnd:
	{
	  if ( timeline->edl() )
	  {
	     boost::int64_t f;
	     next = timeline->image_at( frame + offset );
	     f = timeline->global_to_local( frame + offset );
	     if ( !next )
	     {
		if ( loop == ImageView::kLooping )
		{
		   f = boost::int64_t(timeline->minimum());
		   next = timeline->image_at( f );
		   f = timeline->global_to_local( f );
		}
		else
		{
		   next = img;
		}
	     }
	     
	     assert( next != NULL );
	     
	     if ( next != img && next != NULL) 
	     {
		next->preroll( f );
		next->play( CMedia::kForwards, uiMain );
		status = kEndNextImage;
		break;
	     }
	  }

	  if ( loop == ImageView::kLooping )
	  {
	     frame = first + 1;
	     status = kEndLoop;
	  }
	  else if ( loop == ImageView::kPingPong )
	  {
	     frame = last - 1;
	     step  = -1;
	     view->playback( ImageView::kBackwards );
	     img->frame( frame );
	     img->playback( CMedia::kBackwards );
	     status = kEndChangeDirection;
	  }
	  else
	  {
	      view->playback( ImageView::kStopped );
	      img->playback( CMedia::kStopped );
	  }
	  break;
	}
      case kLoopAtStart:
	{
	  if ( timeline->edl() )
	    {
	       boost::int64_t f;
	       next = timeline->image_at( frame + offset );
	       f = timeline->global_to_local( frame + offset );
	       if ( !next )
	       {
		  if ( loop == ImageView::kLooping )
		  {
		     f = boost::int64_t( timeline->maximum() );
		     next = timeline->image_at( f );
		     f = timeline->global_to_local( f );
		  }
		  else
		  {
		     next = img;
		  }
	       }

	      assert( next != NULL );

	      if ( next != img && next != NULL ) 
		{
		   next->preroll( f );
		   next->play( CMedia::kBackwards, uiMain );
		   status = kEndNextImage;
		   break;
		}
	    }

	  if ( loop == ImageView::kLooping )
	    {
	       frame = last - 1;
	      status = kEndLoop;
	    }
	  else if ( loop == ImageView::kPingPong )
	    {
	       frame = first;
	       step = 1;
	       view->playback( ImageView::kForwards );
	       img->frame( frame );
	       img->playback( CMedia::kForwards );
	       status = kEndChangeDirection;
	    }
	  else
	  {
	      view->playback( ImageView::kStopped );
	      img->playback( CMedia::kStopped );
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



  CheckStatus check_loop( int64_t& frame,
			  CMedia* img, 
			  mrv::Timeline* timeline )
  {
     boost::int64_t last = ( boost::int64_t ) timeline->maximum();
     boost::int64_t first = ( boost::int64_t ) timeline->minimum();
     boost::int64_t f = frame;
     boost::int64_t offset = timeline->offset(img);

     if ( timeline->edl() )
     {
	boost::int64_t s = img->first_frame() + offset;
	boost::int64_t e = img->last_frame() + offset;
	if ( e < last )  last = e;
	if ( s > first ) first = s;
	f += offset;
     }
     
    if ( f > last )
      {
	 img->loop_at_end( frame-1 );
	 return kLoopAtEnd;
      }
    else if ( f < first )
      {	
	 img->loop_at_start( frame+1 );
	 return kLoopAtStart;
      }

    return kNoChange;
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

    // delete the data (we don't need it anymore)
    delete data;

    int64_t frame = img->audio_frame();
    int64_t failed_frame = std::numeric_limits< int64_t >::min();


    mrv::Timeline*   timeline = uiMain->uiTimeline;
    assert( timeline != NULL );


#ifdef DEBUG_THREADS
    cerr << "ENTER AUDIO THREAD " << img->name() << " " << data 
	 << " frame " << frame << endl;
#endif


    while ( !img->stopped() )
      {
	int step = (int) img->playback();
	if ( step == 0 ) break;
	CMedia::DecodeStatus status = img->decode_audio( frame );
	switch( status )
	  {
 	  case CMedia::kDecodeDone:
 	    break;
	  case CMedia::kDecodeError:
	    frame += step;
	    img->audio_frame( frame );
	    continue;
	  case CMedia::kDecodeMissingFrame:
	    if ( failed_frame == frame ) {
	      // Failed to find frame twice, skip this frame
	      frame += step;
	      continue;
	    }
	    failed_frame = frame;
	    img->wait_audio();
	    continue;
	  case  CMedia::kDecodeLoopEnd:
	  case  CMedia::kDecodeLoopStart:
	    {
	      CMedia::Barrier* barrier = img->loop_barrier();
	      barrier->count( barrier_thread_count( img ) );
	      // Wait until all threads loop and decode is restarted
	      barrier->wait();
	      continue;
	    }
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

	img->find_audio( frame );

	

	if ( !img->has_picture() && timeline->edl() )
	  { 
	     int64_t f = frame + timeline->location(img) - img->first_frame();
	     if ( f > timeline->maximum() )
		f = int64_t( timeline->maximum() );
	     if ( f < timeline->minimum() )
		f = int64_t( timeline->minimum() );
	     timeline->value( double( f ) );
	  }

	frame += step;
      }

#ifdef DEBUG_THREADS
    cerr << "EXIT AUDIO THREAD " << img->name() << " " << data 
	 << " frame " << img->audio_frame() << endl;
#endif

  } // audio_thread



  //
  // Main loop used to play audio (of any image)
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
	    barrier->count( barrier_thread_count( img ) );
	    barrier->wait();
	    continue;
	  }
	
	double fps = img->play_fps();
	timer.setDesiredFrameRate( fps );
	timer.waitUntilNextFrameIsDue();

      }


#ifdef DEBUG_THREADS
    cerr << "EXIT SUBTITLE THREAD " << img->name() 
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

    mrv::Timeline*   timeline = uiMain->uiTimeline;
    assert( timeline != NULL );

    int64_t frame        = img->frame();
    int64_t failed_frame = std::numeric_limits< int64_t >::min();

#ifdef DEBUG_THREADS
    cerr << "ENTER VIDEO THREAD " << img->name() << " " << data 
	 << " frame " << frame << endl;
#endif

    // delete the data (we don't need it anymore)
    delete data;



    mrv::Timer timer;
    double fps = img->play_fps();
    timer.setDesiredFrameRate( fps );

    while ( !img->stopped() )
      {
	img->wait_image();


	int step = (int) img->playback();
	if ( step == 0 ) break;

	CMedia::DecodeStatus status = img->decode_video( frame );

	switch( status )
	  {
	     // case CMedia::kDecodeDone:
	     //    continue;
	  case CMedia::kDecodeError:
	    frame += step;
	    continue;
	  case CMedia::kDecodeMissingFrame:
	    if ( failed_frame == frame ) {
	      // Failed to find frame twice, skip this frame
	      frame += step;
	      continue;
	    }
	    failed_frame = frame;
	    continue;
	  case CMedia::kDecodeLoopEnd:
	  case CMedia::kDecodeLoopStart:
	     {
	      CMedia::Barrier* barrier = img->loop_barrier();
	      // Wait until all threads loop and decode is restarted
	      barrier->count( barrier_thread_count( img ) );
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
	if ( img->has_audio() )
	{
	   // double video_clock = img->video_pts();
	   // double audio_clock = img->audio_pts();

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

	img->find_image( frame );

	if ( timeline->edl() )
	  {
	     assert( img != NULL );
	     int64_t f = frame + timeline->location(img) - img->first_frame();

	     assert( f <= timeline->maximum() );
	     assert( f >= timeline->minimum() );


	     timeline->value( double( f ) );
	  }


	frame += step;

      }


#ifdef DEBUG_THREADS
    cerr << "EXIT VIDEO THREAD " << img->name() << " " << data
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

    mrv::Timeline*   timeline = uiMain->uiTimeline;
    assert( timeline != NULL );

    int step = (int) img->playback();


    int64_t frame = img->dts() - step;

#ifdef DEBUG_THREADS
    cerr << "ENTER DECODE THREAD " << img->name() << " " << data << " frame "
	 << frame << " step " << step << endl;
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
	frame += step;

	CMedia* next = NULL;
	CheckStatus status = check_loop( frame, img, timeline );
	if ( status != kNoChange )
	  {

	    // Lock thread until loop status is resolved on all threads
	    CMedia::Barrier* barrier = img->loop_barrier();
	    barrier->count( barrier_thread_count( img ) );
	    // Wait until all threads loop or exit
	    barrier->wait();

	    if ( img->stopped() ) break;

	    // Do the looping, taking into account ui state
	    // and return new frame and step.
	    EndStatus end = handle_loop( frame, step, img, 
					 uiMain, timeline, status );
	   
	    if ( img->stopped() ) { 
	       // img->frame( frame );
	       break;
	    }

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
	   frame = img->dts();

      }

#ifdef DEBUG_THREADS
    cerr << "EXIT DECODE THREAD " << img->name() << " " << data << " frame " 
	 << frame << "  dts: " << img->dts() << endl;
#endif

  }



} // namespace mrv
