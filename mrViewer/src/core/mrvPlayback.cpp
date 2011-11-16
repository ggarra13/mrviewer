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

#include "gui/mrvTimeline.h"
#include "gui/mrvImageView.h"
#include "mrViewer.h"

#include "mrvPlayback.h"



#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

//#  define DEBUG_THREADS

#if 0
#  define DEBUG_DECODE
#  define DEBUG_VIDEO
#  define DEBUG_AUDIO
#endif


#if defined(WIN32) || defined(WIN64)

struct timespec {
   time_t tv_sec;        /* seconds */
   long   tv_nsec;       /* nanoseconds */
};

void nanosleep( const struct timespec* req, struct timespec* rem )
{
  Sleep( 1000 * req->tv_sec + req->tv_nsec / 1e7f );
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

    boost::int64_t last = ( int64_t ) timeline->maximum();
    boost::int64_t first = ( int64_t ) timeline->minimum();


    ImageView::Looping loop = view->looping();


    switch( end )
      {
      case kLoopAtEnd:
	{
	  if ( timeline->edl() )
	    {
	      next = timeline->image_at( frame + timeline->offset(img) );
	      if ( !next )
		{
		  if ( loop == ImageView::kLooping )
		    {
		      next = timeline->image_at( int64_t(timeline->minimum()) );
		    }
		  else
		    {
		      next = img;
		    }
		}


	      assert( next != NULL );

	      if ( next != img ) 
		{
		  next->preroll( next->first_frame() );
		  next->play( CMedia::kForwards, uiMain );
		  status = kEndNextImage;
		  break;
		}
	    }

	  if ( img->first_frame() == img->last_frame() ) 
	    {
	      status = kEndStop;
	    }
	  else if ( loop == ImageView::kLooping )
	    {
	      frame  = first;
	      status = kEndLoop;
	    }
	  else if ( loop == ImageView::kPingPong )
	    {
	      frame = last - 1;
	      step  = -1;
	      img->frame( last );
	      view->playback( ImageView::kBackwards );
	      img->playback( CMedia::kBackwards );
	      status = kEndChangeDirection;
	    }
	  break;
	}
      case kLoopAtStart:
	{
	  if ( timeline->edl() )
	    {
	      next = timeline->image_at( frame + timeline->offset(img) );
	      if ( !next )
		{
		  if ( loop == ImageView::kLooping )
		    {
		      next = timeline->image_at( int64_t(timeline->maximum()) );
		    }
		  else
		    {
		      next = img;
		    }
		}

	      assert( next != NULL );

	      if ( next != img ) 
		{
		  next->preroll( next->last_frame() );
		  next->play( CMedia::kBackwards, uiMain );
		  status = kEndNextImage;
		  break;
		}
	    }

	  if ( img->first_frame() == img->last_frame() ) 
	    {
	      status = kEndStop;
	    }
	  else if ( loop == ImageView::kLooping )
	    {
	      frame  = last;
	      status = kEndLoop;
	    }
	  else if ( loop == ImageView::kPingPong )
	    {
	      frame = first + 1;
	      step  = 1;
	      img->frame( first );
	      view->playback( ImageView::kForwards );
	      img->playback( CMedia::kForwards );
	      status = kEndChangeDirection;
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
			  const CMedia* img, 
			  mrv::Timeline* timeline )
  {
     boost::int64_t last = ( boost::int64_t ) timeline->maximum();
     if ( img->last_frame() < last ) last = img->last_frame();
     boost::int64_t first = ( boost::int64_t ) timeline->minimum();
     if ( img->first_frame() > first ) first = img->first_frame();

    if ( frame > last )
      {
	return kLoopAtEnd;
      }
    else if ( frame < first )
      {	
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
    cerr << "ENTER AUDIO THREAD " << img->name() << " " << frame << endl;
#endif


    while ( !img->stopped() )
      {
	int step = (int) img->playback();
	CMedia::DecodeStatus status = img->decode_audio( frame );
	switch( status )
	  {
// 	  case CMedia::kDecodeDone:
// 	    continue;
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
	     int64_t tframe = ( frame - img->first_frame() + 
				timeline->location(img) );
	     assert( tframe < int64_t( timeline->maximum() ) );
	     timeline->value( tframe );
	  }

	frame += step;
      }

#ifdef DEBUG_THREADS
    cerr << "EXIT AUDIO THREAD " << img->name() << " at " 
	 << frame  << "  img: " << img->audio_frame() << endl;
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
	    barrier->count( barrier_thread_count( img ) );
	    // Wait until all threads loop and decode is restarted
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

    // delete the data (we don't need it anymore)
    delete data;

    int64_t frame        = img->frame();
    int64_t failed_frame = std::numeric_limits< int64_t >::min();


    mrv::Timer timer;

#ifdef DEBUG_THREADS
    cerr << "ENTER VIDEO THREAD " << img->name() << " " << frame << endl;
#endif

    while ( !img->stopped() )
      {
	int step = (int) img->playback();
	CMedia::DecodeStatus status = img->decode_video( frame );
	switch( status )
	  {
// 	  case CMedia::kDecodeDone:
// 	    continue;
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
	    img->wait_image();
	    continue;
	  case CMedia::kDecodeLoopEnd:
	  case CMedia::kDecodeLoopStart:
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


	double fps = img->play_fps();
	double delay = 1.0 / fps;
	double diff = 0.0;

	// // Calculate video-audio difference
	if ( img->has_audio() )
	{
	   double audio_clock = img->audio_clock();
	   double video_clock = img->video_clock();
	   // double audio_clock = img->audio_pts();
	   // double video_clock = img->video_pts();

	   diff = step * (video_clock - audio_clock);
	   img->avdiff( diff );
	   double absdiff = std::abs(diff);

	   /* Skip or repeat the frame. Take delay into account
	      FFPlay still doesn't "know if this is the best guess." */
	   double sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay);
	   if(absdiff < AV_NOSYNC_THRESHOLD) 
	   {
	      if (diff <= -sync_threshold ) 
	      {
	      	 fps = 999999999; // skip frame
	      } 
	      else if (diff >= sync_threshold) 
	      {
		 fps -= (diff / fps);
	      }
	   }

	}
	
	timer.setDesiredFrameRate( fps );
	timer.waitUntilNextFrameIsDue();

	img->real_fps( timer.actualFrameRate() );
	
	img->find_image( frame );

	if ( timeline->edl() )
	  {
	     int64_t tframe = ( frame - img->first_frame() + 
				timeline->location(img) );
	     assert( tframe < int64_t( timeline->maximum() ) );
	     timeline->value( tframe );
	  }



	frame += step;

      }


#ifdef DEBUG_THREADS
    cerr << "EXIT VIDEO THREAD " << img->name() 
	 << " at " << frame << "  img: " << img->frame() << endl;
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

    // delete the data (we don't need it anymore)
    delete data;


    int step = (int) img->playback();

    int64_t frame = img->dts() - step;



#ifdef DEBUG_THREADS
    cerr << "ENTER DECODE THREAD " << img->name() << " " 
	 << frame << " step " << step << endl;
#endif

    int64_t oldframe = frame;

    while( !img->stopped() )
      {

	if ( img->seek_request() )
	  {
	    img->do_seek();
	    frame = img->dts();
	  }

	frame += step;

	CMedia* next = NULL;
	CheckStatus status = check_loop( frame, img, timeline );
	if ( status != kNoChange )
	  {
	    if ( status == kLoopAtStart )
	      {
		img->loop_at_start( frame );
	      }
	    else if ( status == kLoopAtEnd )
	      {
		img->loop_at_end( frame );
	      }

	    // Lock thread until loop status is resolved on all threads
	    CMedia::Barrier* barrier = img->loop_barrier();
	    barrier->count( barrier_thread_count( img ) );
	    // Wait until all threads loop
	    barrier->wait();

	    if ( img->stopped() ) break;

	    // Do the looping, taking into account ui state
	    // and return new frame and step.
	    EndStatus end = handle_loop( frame, step, img, 
					 uiMain, timeline, status );
	    // if ( end == kEndStop || end == kEndNextImage ) continue; 

	    // std::cerr << "DEC CONTINUE " << frame << std::endl;
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
	frame = img->dts();

      }

#ifdef DEBUG_THREADS
    cerr << "EXIT DECODE THREAD " << img->name() << " " << frame 
	 << "  img: " << img->dts() << endl;
#endif

  }



} // namespace mrv
