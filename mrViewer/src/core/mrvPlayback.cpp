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

extern "C" {
#include <libavutil/time.h>
}

#ifdef _WIN32
# include <float.h>
# define isnan _isnan
#endif

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_array.hpp>
using namespace std;

#include "core/CMedia.h"
#include "core/aviImage.h"
#include "core/mrvMath.h"
#include "core/mrvTimer.h"
#include "core/mrvThread.h"
#include "core/mrvBarrier.h"

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

//#undef DBG
//#define DBG(x) std::cerr << x << std::endl

#if 0
#  define DEBUG_DECODE
#  define DEBUG_VIDEO
#  define DEBUG_AUDIO
#endif

// #define DEBUG_THREADS

typedef boost::recursive_mutex Mutex;
typedef boost::condition_variable Condition;


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


double get_clock(Clock *c)
{
    // if (*c->queue_serial != c->serial)
    // {
    //   return NAN;
    // } else {
    double time = av_gettime_relative() / 1000000.0;
    return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
    // }
}

void set_clock_at(Clock *c, double pts, int serial, double time)
{
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

static void set_clock(Clock *c, double pts, int serial)
{
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
}



static void set_clock_speed(Clock *c, double speed)
{
    set_clock(c, get_clock(c), c->serial);
    c->speed = speed;
}

static void init_clock(Clock *c, int *queue_serial)
{
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}


void sync_clock_to_slave(Clock *c, Clock *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        set_clock(c, slave_clock, -1);
}

void update_video_pts(CMedia* is, double pts, int64_t pos, int serial) {
    /* update current video pts */
    set_clock(&is->vidclk, pts, serial);
    sync_clock_to_slave(&is->extclk, &is->vidclk);
}

inline int get_master_sync_type(CMedia* img) {
    if (img->av_sync_type == CMedia::AV_SYNC_VIDEO_MASTER) {
        if (img->has_picture())
            return CMedia::AV_SYNC_VIDEO_MASTER;
        else
            return CMedia::AV_SYNC_AUDIO_MASTER;
    } else if (img->av_sync_type ==  CMedia::AV_SYNC_AUDIO_MASTER) {
        if (img->has_audio())
            return  CMedia::AV_SYNC_AUDIO_MASTER;
        else
            return  CMedia::AV_SYNC_EXTERNAL_CLOCK;
    } else {
        return  CMedia::AV_SYNC_EXTERNAL_CLOCK;
    }
}

inline double get_master_clock(CMedia* img)
{
    double val;

    switch (get_master_sync_type(img)) {
        case CMedia::AV_SYNC_VIDEO_MASTER:
            val = get_clock(&img->vidclk);
            break;
        case CMedia::AV_SYNC_AUDIO_MASTER:
            val = get_clock(&img->audclk);
            break;
        default:
            val = get_clock(&img->extclk);
            break;
    }
    return val;
}


unsigned int barrier_thread_count( const CMedia* img )
{
    unsigned r = 1;               // 1 for decode thread
    if    ( img->valid_video() )    r += 1;
    if    ( img->valid_audio() )    r += 1;
    if    ( img->valid_subtitle() ) r += 1;
    return r;
}



EndStatus handle_loop( boost::int64_t& frame,
                       int&     step,
                       CMedia* img, 
                       bool    fg,
                       mrv::ViewerUI* uiMain,
                       const mrv::Reel  reel,
                       const mrv::Timeline* timeline,
                       const mrv::CMedia::DecodeStatus end, 
                       const bool video = false )
{

    if ( !img || !timeline || !reel || !uiMain ) return kEndIgnore;

    CMedia::Mutex& m = img->video_mutex();
    SCOPED_LOCK( m );

    mrv::ImageView* view = uiMain->uiView;

    EndStatus status = kEndIgnore;
    mrv::media c;
    CMedia* next = NULL;

    boost::int64_t last = boost::int64_t( timeline->maximum() );
    boost::int64_t first = boost::int64_t( timeline->minimum() );

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
       if ( img->last_frame() < last )
           last = img->last_frame();
       if ( img->first_frame() > first )
           first = img->first_frame();
    }


   ImageView::Looping loop = view->looping();


   switch( end )
   {
      case CMedia::kDecodeLoopEnd:
	 {

	    if ( reel->edl )
	    {
	       boost::int64_t f = frame;

	       f -= img->first_frame();
	       f += reel->location(img);

	       if ( f <= timeline->maximum() )
	       {
		  c = reel->media_at( f );
                  next = c->image();
                  if ( next == img ) return status;
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
                     c = reel->media_at( f );
                     next = c->image();
		     f = reel->global_to_local( f );
		  }
		  else
		  {
		     next = img;
		  }
	       }

	       if ( next != img && next != NULL) 
	       {
                   if ( video )
                   {
                       CMedia::Mutex& m2 = next->video_mutex();
                       SCOPED_LOCK( m2 );

                       if ( next->stopped() )
                       {
                           next->seek( f );
                           next->play( CMedia::kForwards, uiMain, fg );
                       }

                       img->playback( CMedia::kStopped );
                       if ( img->has_video() ) img->clear_cache();
                   }

                   status = kEndNextImage;
                   return status;
	       }
	    }

            if ( loop == ImageView::kLooping )
            {
                frame = first;
                if ( img->right_eye() ) img->right_eye()->seek( frame );
                status = kEndLoop;
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
            else if ( loop == ImageView::kPingPong )
            {
                frame = last;
                step  = -1;
                // img->frame( frame );
                img->playback( CMedia::kBackwards );
                if (fg)
                    view->playback( ImageView::kBackwards );
                status = kEndChangeDirection;
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
            else
            {
                if (fg)
                    view->playback( ImageView::kStopped );
                img->playback( CMedia::kStopped );
            }
	    break;
	 }
      case CMedia::kDecodeLoopStart:
	 {
	    if ( reel->edl )
	    {
	       boost::int64_t f = frame;

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


	       if ( next != img && next != NULL )
	       {
                   if ( video )
                   {
                       CMedia::Mutex& m2 = next->video_mutex();
                       SCOPED_LOCK( m2 );

                       if ( video && next->stopped() )
                       {
                           next->seek( f );
                           next->play( CMedia::kBackwards, uiMain, fg );
                       }

                       img->playback( CMedia::kStopped );
                       if ( img->has_video() ) img->clear_cache();
                   }

		  status = kEndNextImage;
		  return status;
	       }
	    }


            if ( loop == ImageView::kLooping )
            {
                frame = last;
                if ( img->right_eye() ) img->right_eye()->seek( frame );
                status = kEndLoop;
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
            else if ( loop == ImageView::kPingPong )
            {
                frame = first;
                step = 1;
                //img->frame( frame );
                img->playback( CMedia::kForwards );
                if (fg)
                    view->playback( ImageView::kForwards );
                status = kEndChangeDirection;
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
            else
            {
                img->playback( CMedia::kStopped );
                if (fg)
                    view->playback( ImageView::kStopped );
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
      if ( img->has_video() ) img->clear_cache();
   }

   return status;
}



CMedia::DecodeStatus check_loop( const int64_t frame,
				 CMedia* img,
				 const mrv::Reel reel,
				 const mrv::Timeline* timeline )
{

   boost::int64_t last = boost::int64_t( timeline->maximum() );
   boost::int64_t first = boost::int64_t( timeline->minimum() );

   boost::int64_t f = frame;

   if ( reel->edl )
   {
       CMedia::Mutex& m = img->video_mutex();
       SCOPED_LOCK( m );

       boost::int64_t s = reel->location(img);
       boost::int64_t e = s + img->duration() - 1;

       if ( e < last )  last = e;
       if ( s > first ) first = s;

       last = reel->global_to_local( last );
       first = reel->global_to_local( first );
   }
   else
   {
       CMedia::Mutex& m = img->video_mutex();
       SCOPED_LOCK( m );

       if ( last > img->last_frame() )
           last = img->last_frame();
       else if ( img->first_frame() > first )
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

   // delete the data (we don't need it anymore)
   delete data;


   int64_t frame = img->frame() + img->audio_offset();
   

   int64_t failed_frame = std::numeric_limits< int64_t >::min();


   mrv::ImageView*      view = uiMain->uiView;
   mrv::Timeline*      timeline = uiMain->uiTimeline;
   mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;


   int idx = fg ? view->fg_reel() : view->bg_reel();

   mrv::Reel   reel = browser->reel_at( idx );
   if (!reel) return;

#ifdef DEBUG_THREADS
   cerr << "ENTER " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? " << img->stopped()
	<< " frame " << frame << endl;
#endif
   mrv::Timer timer;


   //img->av_sync_type = CMedia::AV_SYNC_EXTERNAL_CLOCK;
   img->av_sync_type = CMedia::AV_SYNC_AUDIO_MASTER;
   init_clock(&img->vidclk, NULL);
   init_clock(&img->audclk, NULL);
   init_clock(&img->extclk, NULL);
   set_clock(&img->extclk, get_clock(&img->extclk), false);


   while ( !img->stopped() && view->playback() != mrv::ImageView::kStopped )
   {

      int step = (int) img->playback();
      if ( step == 0 ) break;


      DBG( "wait audio " << frame );
      img->wait_audio();


      boost::int64_t f = frame;
      DBG( "decode audio " << frame );

      // img->debug_audio_packets( frame, "play", false );


      CMedia::DecodeStatus status = img->decode_audio( f );

      DBG( "Decode returned " << status );

      switch( status )
      {
	 case CMedia::kDecodeError:
             LOG_ERROR( img->name() 
                         << _(" - decode Error audio frame ") << frame );
             frame += step;
             continue;
	 case CMedia::kDecodeMissingFrame:
             LOG_WARNING( img->name() 
                          << _(" - decode missing audio frame ") << frame );
             timer.setDesiredFrameRate( img->play_fps() );
             timer.waitUntilNextFrameIsDue();
             frame += step;
             continue;
          case CMedia::kDecodeNoStream:
              DBG( "Decode No stream" );
              timer.setDesiredFrameRate( img->play_fps() );
             timer.waitUntilNextFrameIsDue();
             frame += step;
             continue;
          case  CMedia::kDecodeLoopEnd:
          case  CMedia::kDecodeLoopStart:
              {
                  DBG( img->name() << " BARRIER IN AUDIO " << frame );

                  CMedia::Barrier* barrier = img->loop_barrier();
                  // Wait until all threads loop and decode is restarted
                  barrier->wait();

                  DBG( img->name() << " BARRIER PASSED IN AUDIO " << frame );

                  if ( img->stopped() ) continue;

                  EndStatus end = handle_loop( frame, step, img, fg, uiMain,
                                               reel, timeline, status,
                                               !img->has_picture() );

                  frame += img->audio_offset();


                  DBG( img->name() << " AUDIO LOOP END/START HAS FRAME " << frame );
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


      if ( fg && img->has_audio_data() && reel->edl && img->is_left_eye() )
      {
          int64_t f = frame + reel->location(img) - img->first_frame();
          if ( f > timeline->maximum() )
              f = int64_t( timeline->maximum() );
          if ( f < timeline->minimum() )
              f = int64_t( timeline->minimum() );
          view->frame( f );
      }


      if ( !img->stopped() )
      {
          img->find_audio(frame);
      }

      frame += step;
   }

#ifdef DEBUG_THREADS
   cerr << endl << "EXIT " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? " 
	<< img->stopped()  
	<< " frame " << img->audio_frame() << endl;
   assert( img->stopped() );
#endif

} // audio_thread



  //
  // Main loop used to decode subtitles
  //
void subtitle_thread( PlaybackData* data )
{
   assert( data != NULL );

   mrv::ViewerUI*     uiMain   = data->uiMain;

   CMedia* img = data->image;
   assert( img != NULL );



   bool fg = data->fg;

   // delete the data (we don't need it anymore)
   delete data;


   mrv::ImageView*      view = uiMain->uiView;
   mrv::Timeline*      timeline = uiMain->uiTimeline;
   mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;

   int idx = fg ? view->fg_reel() : view->bg_reel();

   mrv::Reel   reel = browser->reel_at( idx );
   if (!reel) return;

    mrv::Timer timer;

#ifdef DEBUG_THREADS
    cerr << "ENTER SUBTITLE THREAD " << img->name() << endl;
#endif


    while ( !img->stopped() && view->playback() != mrv::ImageView::kStopped )
      {
	int step = (int) img->playback();
	if ( step == 0 ) break;

	int64_t frame = img->frame();
	CMedia::DecodeStatus status = img->decode_subtitle( frame );

	if ( frame > img->last_frame() )
        {
	   status = CMedia::kDecodeLoopEnd;
        }
	else if ( frame < img->first_frame() )
        {
	   status = CMedia::kDecodeLoopStart;
        }

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

            if ( img->stopped() ) continue;

            EndStatus end = handle_loop( frame, step, img, fg, uiMain, 
                                         reel, timeline, status );
	    continue;
	  }
	
	double fps = img->play_fps();
	timer.setDesiredFrameRate( fps );
	timer.waitUntilNextFrameIsDue();

      }


#ifdef DEBUG_THREADS
    cerr << endl << "EXIT  SUBTITLE THREAD " << img->name() 
	 << " stopped? " << img->stopped() << endl;
   assert( img->stopped() );
#endif

  }  // subtitle_thread

// #undef DBG
// #define DBG(x) std::cerr << x << std::endl

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

   mrv::ImageView*      view = uiMain->uiView;
   mrv::Timeline*      timeline = uiMain->uiTimeline;
   mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;

   // delete the data (we don't need it anymore)
   delete data;

   int idx = fg ? view->fg_reel() : view->bg_reel();

   mrv::Reel   reel = browser->reel_at( idx );
   if (!reel) return;

   int64_t frame        = img->frame();
   int64_t failed_frame = std::numeric_limits< int64_t >::min();

#ifdef DEBUG_THREADS
   cerr << "ENTER " << (fg ? "FG" : "BG") << " VIDEO THREAD " << img->name() << " stopped? " << img->stopped()
	<< " frame " << frame << " timeline frame " << timeline->value() 
        << endl;
#endif


   mrv::Timer timer;
   double fps = img->play_fps();
   timer.setDesiredFrameRate( fps );


   while ( !img->stopped() && view->playback() != mrv::ImageView::kStopped )
   {
       DBG( img->name() << " wait image " << frame );
       img->wait_image();


       // img->debug_video_packets( frame, "PLAYBACK", true );
       // img->debug_video_stores( frame, "BACK" );

       int step = (int) img->playback();
       if ( step == 0 ) break;

       DBG( img->name() << " decode image " << frame );
       CMedia::DecodeStatus status = img->decode_video( frame );
       DBG( img->name() << " decoded image " << frame << " status " 
            << CMedia::decode_error(status) );

      switch( status )
      {
          // case CMedia::DecodeDone:
          //    continue;
          case CMedia::kDecodeError:
          case CMedia::kDecodeMissingFrame:
              LOG_ERROR( img->name() << _(" - Decode of image frame ") << frame 
                         << _(" returned ") << CMedia::decode_error( status ) );
             break;
          case CMedia::kDecodeLoopEnd:
          case CMedia::kDecodeLoopStart:
	    {
               DBG( img->name() << " BARRIER PASSED IN VIDEO" );

               CMedia::Barrier* barrier = img->loop_barrier();
               // LOG_INFO( img->name() << " BARRIER VIDEO WAIT      gen: " 
               //           << barrier->generation() 
               //           << " count: " << barrier->count() 
               //           << " threshold: " << barrier->threshold() 
               //           << " used: " << barrier->used() );
               // Wait until all threads loop and decode is restarted
               bool ok = barrier->wait();

               DBG( img->name() << " BARRIER PASSED IN VIDEO stopped? "
                    << img->stopped() );

               if ( img->stopped() ) continue;

               DBG( img->name() << " VIDEO LOOP frame: " << frame );


               EndStatus end = handle_loop( frame, step, img, fg, uiMain, 
                                            reel, timeline, status, true );

               DBG( img->name() << " VIDEO LOOP END frame: " << frame 
                    << " step " << step );

	       continue;
	    }
          case CMedia::kDecodeBufferFull:
          default:
              break;
      }

      fps = img->play_fps();

      double delay = 1.0 / fps;

      double diff = 0.0;
      double bgdiff = 0.0;

      // // Calculate video-audio difference
      if ( img->has_audio() && status == CMedia::kDecodeOK )
      {

          double video_clock, master_clock;

          if ( step < 0 )
          {
              video_clock = img->video_clock();
              master_clock = img->audio_clock();
          }
          else
          {
              video_clock = get_clock(&img->vidclk);
              master_clock = get_master_clock(img);
          }

#if 0
          std::cerr  << " VC: " << video_clock 
                     << " MC: " << master_clock
                     << " AC: " << get_clock(&img->audclk)
                     << " EC: " << get_clock(&img->extclk)
                     << std::endl;
#endif


          diff = step * ( video_clock - master_clock );

          double absdiff = std::abs(diff);

          if ( absdiff > 1000.0 ) diff = 0.0;

          img->avdiff( diff );

	 // Skip or repeat the frame. Take delay into account
	 //    FFPlay still doesn't "know if this is the best guess."
	 if(absdiff < AV_NOSYNC_THRESHOLD) {
	    double sdiff = step * diff;
            double sync_threshold = delay;

	    if (sdiff <= -sync_threshold) {
	       fps = 99999999.0;
	    } else if (sdiff >= sync_threshold) {
                fps -= sdiff*2.0;      // make fps slower
	    }
	 }
      }


      timer.setDesiredFrameRate( fps );
      timer.waitUntilNextFrameIsDue();

      img->real_fps( timer.actualFrameRate() );


      DBG( img->name() << " find image " << frame );
      bool ok = img->find_image( frame );

      if ( !img->has_audio_data() && reel->edl && img->is_left_eye() )
      {
	 int64_t f = frame + reel->location(img) - img->first_frame();
         if ( frame <= img->last_frame() )
         {
             if ( fg && img->is_left_eye() )
             {
                 view->frame( f );
             }
         }
      }


      frame += step;
   }


#ifdef DEBUG_THREADS
   cerr << endl << "EXIT  " << (fg ? "FG" : "BG") << " VIDEO THREAD " 
	<< img->name() << " stopped? " << img->stopped()
	<< " at " << frame << "  img->frame: " << img->frame() << endl;
   assert( img->stopped() );
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

   mrv::ImageView*      view = uiMain->uiView;
   mrv::Timeline*      timeline = uiMain->uiTimeline;
   mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;
   assert( timeline != NULL );

   // delete the data (we don't need it anymore)
   delete data;

   int idx = fg ? view->fg_reel() : view->bg_reel();

   mrv::Reel   reel = browser->reel_at( idx );
   if (!reel) return;


   int step = (int) img->playback();


   int64_t frame = img->dts();

#ifdef DEBUG_THREADS
   cerr << "ENTER " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped() << " frame " << frame 
	<< " step " << step << endl;
#endif



   while ( !img->stopped() && view->playback() != mrv::ImageView::kStopped )
   {

      if ( img->seek_request() )
      {
	 img->do_seek();
	 frame = img->dts();
      }


      step = (int) img->playback();
      frame += step;

      CMedia::DecodeStatus status = check_loop( frame, img, reel, timeline );


      if ( status != CMedia::kDecodeOK )
      {

          CMedia::Barrier* barrier = img->loop_barrier();
          DBG( img->name() << " BARRIER DECODE WAIT      gen: " 
               << barrier->generation() 
               << " count: " << barrier->count() 
               << " threshold: " << barrier->threshold() 
               << " used: " << barrier->used() );
          // Wait until all threads loop and decode is restarted
          barrier->wait();
	  DBG( img->name() << " BARRIER DECODE LOCK PASS gen: " 
               << barrier->generation() 
               << " count: " << barrier->count() 
               << " threshold: " << barrier->threshold() 
	               << " used: " << barrier->used() );


         // Do the looping, taking into account ui state
         //  and return new frame and step.
         // This handle loop has to come after the barrier as decode thread
         // goes faster than video or audio threads

         if ( img->stopped() ) continue;

         EndStatus end = handle_loop( frame, step, img, fg,
                                      uiMain, reel, timeline, status );
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
   cerr << endl << "EXIT  " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped() << " frame " 
	<< img->frame() << "  dts: " << img->dts() << endl;
   assert( img->stopped() );
#endif

}



} // namespace mrv
