
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
 *         playback.
 *
 *
 */

#include <cstdio>

#include <iostream>

extern "C" {
#include <libavutil/time.h>
#include <libavutil/avassert.h>
}

#ifdef _WIN32
# include <float.h>
# define isnan _isnan
#endif

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_array.hpp>

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

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1

/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

// #undef TRACE
// #define TRACE(x)

#define LOGT_WARNING(x) LOG_WARNING( get_thread_id() << " " << x );
#define LOGT_INFO(x) LOG_INFO( get_thread_id() << " " << x );
#define LOGT_ERROR(x) LOG_ERROR( get_thread_id() << " " << x );

//#define DEBUG_THREADS

typedef boost::recursive_mutex Mutex;



void sleep_ms(int milliseconds) // cross-platform sleep function
{
#ifdef WIN32
    Sleep(milliseconds);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    usleep(milliseconds * 1000);
#endif
}



namespace mrv {


enum EndStatus {
    kEndIgnore,
    kEndStop,
    kEndNextImage,
    kEndChangeDirection,
    kEndLoop,
};

unsigned long get_thread_id(){
    std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
    unsigned long threadNumber = 0;
    sscanf(threadId.c_str(), "%lx", &threadNumber);
    return threadNumber;
}

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
    // c->serial = serial;
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
    // c->paused = 0;
    // c->queue_serial = queue_serial;
    set_clock(c, 0, -1);
}


void sync_clock_to_slave(Clock *c, Clock *slave)
{
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);

#if __cplusplus >= 201103L
    using std::isnan;
#endif
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


inline unsigned int barrier_thread_count( const CMedia* img )
{
    unsigned r = 1;               // 1 for decode thread
    if    ( img->valid_video() )    r += 1;
    if    ( img->valid_audio() )    r += 1;
    if    ( img->valid_subtitle() ) r += 1;
    return r;
}


CMedia::DecodeStatus check_loop( const int64_t frame,
                                 CMedia* img,
                                 const mrv::Reel reel,
                                 const mrv::Timeline* timeline,
                                 int64_t& first,
                                 int64_t& last )
{
    int64_t mx = int64_t(timeline->display_maximum());
    int64_t mn = int64_t(timeline->display_minimum());


    // int64_t offset = img->first_frame() - img->start_frame();
    // last = last + offset;
    // first = first + offset;


    // std::cerr << "check loop reel " << reel->name << std::endl;
    if ( reel->edl )
    {
        CMedia::Mutex& m = img->video_mutex();
        SCOPED_LOCK( m );


        first = reel->location(img);
        last  = first + img->duration() - 1;



        if ( mx < last )  last = mx;
        if ( mn > first ) first = mn;


        last = reel->global_to_local( last );

        img->loop_end( last );
        first = reel->global_to_local( first );

        img->loop_start( first );

    }
    else
    {
        if ( img->has_picture() || img->has_audio() )
        {
            CMedia::Mutex& m = img->video_mutex();
            SCOPED_LOCK( m );


            first = img->first_frame();
            last = img->last_frame();

            if ( mx < last )  last = mx;
            if ( mn > first ) first = mn;


            img->loop_start( first );
            img->loop_end( last );

        }
    }


    if ( frame > last )
    {
        return CMedia::kDecodeLoopEnd;
    }
    else if ( frame < first )
    {
        return CMedia::kDecodeLoopStart;
    }

    return CMedia::kDecodeOK;
}

CMedia::DecodeStatus check_decode_loop( const int64_t frame,
                                        CMedia* img,
                                        const mrv::Reel reel,
                                        const mrv::Timeline* timeline )
{
    int64_t first, last;
    CMedia::DecodeStatus status = check_loop( frame, img, reel, timeline,
                                              first, last );
    if ( status == CMedia::kDecodeLoopEnd )
    {
        img->loop_at_end( last+1 );
    }
    else if ( status == CMedia::kDecodeLoopStart )
    {
        img->loop_at_start( first-1 );
    }
    return status;
}



EndStatus handle_loop( boost::int64_t& frame,
                       int&     step,
                       CMedia* img,
                       bool    fg,
                       bool init_time,
                       mrv::ViewerUI* uiMain,
                       const mrv::Reel  reel,
                       const mrv::Timeline* timeline,
                       const mrv::CMedia::DecodeStatus end )
{

    if ( !img || !timeline || !reel || !uiMain ) return kEndIgnore;


    mrv::PacketQueue& vp = img->video_packets();
    CMedia::Mutex& vpm1 = vp.mutex();
    SCOPED_LOCK( vpm1 ); // 1155

    mrv::PacketQueue& ap = img->audio_packets();
    CMedia::Mutex& apm1 = ap.mutex();
    SCOPED_LOCK( apm1 );

    mrv::PacketQueue& sp = img->subtitle_packets();
    CMedia::Mutex& spm1 = sp.mutex();
    SCOPED_LOCK( spm1 );

    CMedia::Mutex& m = img->video_mutex();
    SCOPED_LOCK( m );  // 1182

    CMedia::Mutex& ma = img->audio_mutex();
    SCOPED_LOCK( ma );

    mrv::ImageView* view = uiMain->uiView;

    EndStatus status = kEndIgnore;
    mrv::media c;
    CMedia* next = NULL;

    int64_t first, last;
    check_loop( frame, img, reel, timeline, first, last );

    CMedia::Looping loop = view->looping();


    switch( end )
    {
    case CMedia::kDecodeLoopEnd:
    {
        if ( reel->edl )
        {
            boost::int64_t f = frame;

            f -= img->first_frame();
            f += reel->location(img);

            if ( f <= timeline->display_maximum() )
            {
                next = reel->image_at( f );
            }


            if ( next )
            {
                f = reel->global_to_local( f );
            }
            else
            {
                if ( loop == CMedia::kLoop )
                {
                    f = boost::int64_t(timeline->display_minimum());
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
                //if ( video )
                {
                    mrv::PacketQueue& vp = next->video_packets();
                    CMedia::Mutex& vpm2 = vp.mutex();
                    SCOPED_LOCK( vpm2 ); // 1155
                    mrv::PacketQueue& ap = next->audio_packets();
                    CMedia::Mutex& apm2 = ap.mutex();
                    SCOPED_LOCK( apm2 );
                    mrv::PacketQueue& sp = next->subtitle_packets();
                    CMedia::Mutex& spm2 = sp.mutex();
                    SCOPED_LOCK( spm2 );
                    CMedia::Mutex& m2 = next->video_mutex();
                    SCOPED_LOCK( m2 );

                    if ( next->stopped() )
                    {
                        if ( img->fg_bg_barrier() )
                        {
                            // LOGT_WARNING( img->name() << " img barrier "
                            //           << img->fg_bg_barrier() << std::endl
                            //           << "passed to " << " "
                            //           << next->name()
                            //           );
                            CMedia::Barrier* b = img->fg_bg_barrier();
                            next->fg_bg_barrier( b );
                            img->fg_bg_barrier( NULL );
                        }
                        next->seek( f );
                        next->do_seek();
                        next->play( CMedia::kForwards, uiMain, fg );
                        // LOGT_WARNING( next->name() << " " << (fg ? "FG" : "BG")
                        //           << " next barrier "
                        //           << next->fg_bg_barrier() );
                    }

                    img->playback( CMedia::kStopped );
                    img->flush_all();
                    if ( img->has_video() ) img->clear_cache();
                }

                status = kEndNextImage;
                return status;
            }
            if ( img->stopped() ) return kEndNextImage;
        }

        if ( loop == CMedia::kLoop )
        {
            frame = first;
            if ( img->right_eye() ) img->right_eye()->seek( frame );
            status = kEndLoop;
            if ( init_time )
            {
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
        }
        else if ( loop == CMedia::kPingPong )
        {
            frame = last;
            step  = -1;
            status = kEndChangeDirection;
            img->playback( (CMedia::Playback) step );
            if ( init_time )
            {
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
        }
        else
        {
            if (fg) view->playback( CMedia::kStopped );
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

            if ( f >= timeline->display_minimum() )
            {
                next = reel->image_at( f );
            }


            f = reel->global_to_local( f );

            if ( !next )
            {
                if ( loop == CMedia::kLoop )
                {
                    f = boost::int64_t( timeline->display_maximum() );
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
                //if ( video )
                {
                    mrv::PacketQueue& vp = next->video_packets();
                    CMedia::Mutex& vpm = vp.mutex();
                    SCOPED_LOCK( vpm );
                    mrv::PacketQueue& ap = next->audio_packets();
                    CMedia::Mutex& apm = ap.mutex();
                    SCOPED_LOCK( apm );
                    mrv::PacketQueue& sp = next->subtitle_packets();
                    CMedia::Mutex& spm = sp.mutex();
                    CMedia::Mutex& m2 = next->video_mutex();
                    SCOPED_LOCK( m2 );

                    if ( next->stopped() )
                    {
                        if ( img->fg_bg_barrier() )
                        {
                            // LOGT_WARNING( img->name() << " img barrier "
                            //           << img->fg_bg_barrier() << std::endl
                            //           << "passed to " << " "
                            //           << next->name()
                            //           );
                            CMedia::Barrier* b = img->fg_bg_barrier();
                            next->fg_bg_barrier( b );
                            img->fg_bg_barrier( NULL );
                            // LOGT_WARNING( next->name() << " " << (fg ? "FG" : "BG")
                            //           << " next barrier "
                            //           << next->fg_bg_barrier() );
                        }
                        next->seek( f );
                        next->do_seek();
                        next->play( CMedia::kBackwards, uiMain, fg );
                    }

                    img->playback( CMedia::kStopped );
                    img->flush_all();
                    if ( img->has_video() ) img->clear_cache();
                }

                status = kEndNextImage;
                return status;
            }
            if ( img->stopped() ) return kEndNextImage;
        }

        if ( loop == CMedia::kLoop )
        {
            frame = last;
            if ( img->right_eye() ) img->right_eye()->seek( frame );
            status = kEndLoop;
            if ( init_time )
            {
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
        }
        else if ( loop == CMedia::kPingPong )
        {
            frame = first;
            step  = 1;
            img->playback( (CMedia::Playback) step );
            // std::cerr << img->name() << " loop ping pong " << first
            //        << " step " << step << std::endl;
            status = kEndChangeDirection;
            if ( init_time )
            {
                init_clock(&img->vidclk, NULL);
                init_clock(&img->audclk, NULL);
                init_clock(&img->extclk, NULL);
                set_clock(&img->extclk, get_clock(&img->extclk), false);
            }
        }
        else
        {
            img->playback( CMedia::kStopped );
            if (fg) view->playback( CMedia::kStopped );
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





    mrv::ImageView*      view = uiMain->uiView;
    mrv::Timeline*      timeline = uiMain->uiTimeline;
    mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;

    int64_t frame = img->frame() + img->audio_offset();

    int64_t failed_frame = std::numeric_limits< int64_t >::min();


    int idx = fg ? view->fg_reel() : view->bg_reel();

    mrv::Reel   reel = browser->reel_at( idx );
    if (!reel) return;

#ifdef DEBUG_THREADS
    LOGT_INFO( "ENTER " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? " << img->stopped() << " frame " << frame );
#endif
    mrv::Timer timer;


    //img->av_sync_type = CMedia::AV_SYNC_EXTERNAL_CLOCK;
    img->av_sync_type = CMedia::AV_SYNC_AUDIO_MASTER;
    init_clock(&img->vidclk, NULL);
    init_clock(&img->audclk, NULL);
    init_clock(&img->extclk, NULL);
    set_clock(&img->extclk, get_clock(&img->extclk), -1);


    while ( !img->stopped() && view->playback() != CMedia::kStopped )
    {

        int step = (int) img->playback();
        if ( step == 0 ) break;

        //DBG( "wait audio " << frame );
        img->wait_audio();




        boost::int64_t f = frame;
        // DBG( "decode audio " << frame );

        // if (!fg)
        // img->debug_audio_packets( frame, "play", true );


        CMedia::DecodeStatus status = img->decode_audio( f );
        DBG( img->name() << " decoded audio " << f << " status " << status );

        assert( img != NULL );
        assert( reel != NULL );
        assert( timeline != NULL );
        //int64_t first, last;
        //check_loop( frame, img, reel, timeline, first, last );

        switch( status )
        {
        case CMedia::kDecodeError:
            LOGT_ERROR( img->name()
                       << _(" - decode Error audio frame ") << frame );
            frame += step;
            continue;
        case CMedia::kDecodeMissingFrame:
            timer.setDesiredFrameRate( img->play_fps() );
            timer.waitUntilNextFrameIsDue();
            frame += step;
            continue;
        case CMedia::kDecodeNoStream:
            timer.setDesiredFrameRate( img->play_fps() );
            timer.waitUntilNextFrameIsDue();
            if ( fg && !img->has_picture() && reel->edl &&
                    img->is_left_eye() )
            {
                int64_t f = frame + reel->location(img) - img->first_frame();
                f -= img->audio_offset();
                view->frame( f );
            }
            frame += step;
            continue;
        case  CMedia::kDecodeLoopEnd:
        case  CMedia::kDecodeLoopStart:
        {


            DBG( img->name() << " BARRIER IN AUDIO " << frame );


            CMedia::Barrier* barrier = img->loop_barrier();
            if ( barrier )
            {
                // Wait until all threads loop and decode is restarted
                bool ok = barrier->wait();
                DBG( img->name() << " BARRIER PASSED IN AUDIO "
                     << frame );
            }


            barrier = img->fg_bg_barrier();
            if ( barrier )
            {
                // LOGT_INFO( img->name() << " FG/BG BARRIER " << barrier
                //           << " AUDIO LOCK gen: "
                //           << barrier->generation()
                //           << " count: " << barrier->count()
                //           << " threshold: " << barrier->threshold()
                //           << " used: " << barrier->used() );
                bool ok = barrier->wait();
                // LOGT_INFO( img->name() << " BARRIER " << barrier
                //           << " AUDIO FG/BG PASS gen: "
                //           << barrier->generation()
                //           << " count: " << barrier->count()
                //           << " threshold: " << barrier->threshold()
                //           << " used: " << barrier->used() );
            }


            frame -= img->audio_offset();

            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline,
                                         status );

            frame += img->audio_offset();

            // This has to come here not in handle_loop, to avoid
            // setting playback dir on decode thread
            if ( end == kEndChangeDirection  )
            {
                CMedia::Playback p = (CMedia::Playback) step;
                if ( fg ) view->playback( p );
            }

            DBG( img->name() << " AUDIO LOOP END/START HAS FRAME " << frame );
            continue;
        }
        case CMedia::kDecodeOK:
            break;
        default:
            break;
        }



        if ( ! img->has_audio() && img->has_picture() )
        {
            // if audio was turned off, follow video.
            // audio can be turned off due to user changing channels
            // or due to a problem with audio engine.
            frame = img->frame();
            continue;
        }



        if ( fg && !img->has_picture() && reel->edl && img->is_left_eye() )
        {
            int64_t offset = img->audio_offset();
            int64_t f = frame + reel->location(img) - img->first_frame();
            f -= offset;
            view->frame( f );
        }


        if ( !img->stopped() )
        {
            img->find_audio(frame);
        }

        frame += step;
    }

    Mutex& mtx = img->audio_mutex();
    SCOPED_LOCK( mtx );
    CMedia::Barrier* barrier = img->loop_barrier();
    if ( barrier ) barrier->notify_all();
    barrier = img->fg_bg_barrier();
    if ( barrier ) {
        // barrier->notify_all();
        // if ( !fg ) {
        //     delete barrier;
        // }
        // img->fg_bg_barrier( NULL );
    }

#ifdef DEBUG_THREADS
    LOGT_INFO( "EXIT " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? "  << img->stopped() << " frame " << img->audio_frame() );
    assert( img->stopped() );
#endif
    
    img->playback( CMedia::kStopped );


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
    LOGT_INFO( "ENTER SUBTITLE THREAD " << img->name() );
#endif


    while ( !img->stopped() && view->playback() != CMedia::kStopped )
    {
        int step = (int) img->playback();
        if ( step == 0 ) break;

        int64_t frame = img->frame() + step;

        CMedia::DecodeStatus status = img->decode_subtitle( frame );

        // if ( status == CMedia::kDecodeOK )
        // {
        //     int64_t first, last;
        //     check_loop( frame, img, reel, timeline, first, last );
        // }

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

            if ( img->stopped() ) continue;

            CMedia::Barrier* barrier = img->loop_barrier();
            if ( barrier )
            {
                // Wait until all threads loop and decode is restarted
                barrier->wait();
            }

            if ( img->stopped() ) continue;

            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline, status );
            continue;
        }

        double fps = img->play_fps();
        timer.setDesiredFrameRate( fps );
        timer.waitUntilNextFrameIsDue();

    }


#ifdef DEBUG_THREADS
    LOGT_INFO( "EXIT  SUBTITLE THREAD " << img->name()
              << " stopped? " << img->stopped() );
    assert( img->stopped() );
#endif

}  // subtitle_thread


//#undef DBG
// #define DBG(x)

//#define DBG(x) std::cerr << x << std::endl

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

    if ( !view || !timeline || !browser ) return;

    int idx = fg ? view->fg_reel() : view->bg_reel();


    mrv::Reel   reel = browser->reel_at( idx );
    if (!reel) return;

    if (!fg)
    {
        mrv::Reel bgreel = browser->reel_at( view->bg_reel() );
        if ( bgreel && bgreel->images.size() > 1 && bgreel->edl )
        {
            LOGT_ERROR( _("Background reel has several images and has EDL turned on.  This is not allowed.  Turning edl off.") );
            bgreel->edl = false;
        }


        mrv::Reel fgreel = browser->reel_at( view->fg_reel() );
        int64_t d = reel->duration();
        if ( fgreel->duration() > d && d > 1 &&
             view->looping() != CMedia::kNoLoop )
        {
            LOGT_WARNING( _( "Background reel duration is too short.  "
                            "Looping may not work correctly." ) );
        }
        else // if ( fgreel == reel )
        {

            mrv::media fg = view->foreground();
            mrv::media bg = view->background();
            if ( fg && bg )
            {
                CMedia* img  = fg->image();
                CMedia* bimg = bg->image();
                int64_t d = bimg->duration();
                if ( img->duration() > d && d > 1 &&
                     view->looping() != CMedia::kNoLoop )
                {
                    LOGT_WARNING( _( "Background image duration is too short.  "
                                    "Looping may not work correctly." ) );
                }

                if ( std::abs( img->play_fps() - bimg->play_fps() ) > 0.001 )
                {
                    char buf[256];
                    sprintf( buf, _( "Background image play fps ( %lg ) does "
                                     "not match foreground's ( %lg ).  Looping "
                                     "will not work correctly." ),
                             bimg->play_fps(), img->play_fps() );
                    LOGT_WARNING( buf );
                }
            }
        }
    }

    int64_t failed_frame = std::numeric_limits< int64_t >::min();
    int64_t frame;
    {
        // We lock the video mutex to make sure frame was properly updated
        boost::recursive_mutex::scoped_lock lk( img->video_mutex() );

        frame = img->frame();
    }


#ifdef DEBUG_THREADS
    LOGT_INFO( "ENTER " << (fg ? "FG" : "BG") << " VIDEO THREAD " << img->name() << " stopped? " << img->stopped() << " view playback "
              << view->playback() << " frame " << frame );
#endif


    mrv::Timer timer;
    int delay_counter = 0;
    double fps = img->play_fps();
    timer.setDesiredFrameRate( fps );

    while ( !img->stopped() && view->playback() != CMedia::kStopped )
    {
        img->wait_image();


        int step = (int) img->playback();
        if ( step == 0 ) break;

	
        CMedia::DecodeStatus status = img->decode_video( frame );

        // img->debug_video_packets( frame, img->name().c_str(), true );
        // img->debug_video_stores( frame, img->name().c_str(), true );


        switch( status )
        {
            // case CMedia::DecodeDone:
            //    continue;
        case CMedia::kDecodeError:
            LOGT_ERROR( img->name() << _(" - Decode of image frame ") << frame
                       << _(" returned ") << CMedia::decode_error( status ) );
            break;
        case CMedia::kDecodeLoopEnd:
        case CMedia::kDecodeLoopStart:
        {
            //LOG_INFO( img->name() << " BARRIER WAIT IN VIDEO frame " << frame );

            // CMedia::Barrier* barrier = img->background_barrier();
            // if ( barrier )
            // {
            //     std::cerr << "wait for img " << img->name() << std::endl;
            //     std::cerr << "barrier threshold "
            //               << barrier->threshold() << std::endl;
            //     barrier->wait();
            //     std::cerr << "waited for img " << img->name() << std::endl;
            // }

            if ( img->stopped() ) continue;

            CMedia::Barrier* barrier = img->loop_barrier();

            if ( barrier )
            {
                // LOGT_WARNING( img->name() << " BARRIER VIDEO WAIT      gen: "
                //      << barrier->generation()
                //      << " count: " << barrier->count()
                //      << " threshold: " << barrier->threshold()
                //      << " used: " << barrier->used() );
                // Wait until all threads loop and decode is restarted
                bool ok = barrier->wait();
            }

            if ( img->stopped() ) continue;

            barrier = img->fg_bg_barrier();
            if ( barrier )
            {
                // LOGT_WARNING( img->name() << " FG/BG BARRIER " << barrier << " VIDEO FG/BG LOCK gen: "
                //      << barrier->generation()
                //      << " count: " << barrier->count()
                //      << " threshold: " << barrier->threshold()
                //      << " used: " << barrier->used() );
                bool ok = barrier->wait();
                // LOGT_INFO( img->name() << " BARRIER " << barrier << " VIDEO FG/BG PASS gen: "
                //      << barrier->generation()
                //      << " count: " << barrier->count()
                //      << " threshold: " << barrier->threshold()
                //      << " used: " << barrier->used() );
            }

            if ( img->stopped() ) continue;

            barrier = img->stereo_barrier();
            // LOGT_INFO( img->name() << "@" << barrier << " wait" );
            if ( barrier )
            {
                bool ok = barrier->wait();
            }
            // LOGT_INFO( img->name() << "@" << barrier << " waited" );

            DBG( img->name() << " BARRIER PASSED IN VIDEO stopped? "
                 << img->stopped() << " frame: " << frame );

            if ( img->stopped() ) continue;

            DBG( img->name() << " VIDEO LOOP frame: " << frame );


            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline,
                                         status );

            // This has to come here not in handle_loop, to avoid
            // setting playback dir on decode thread
            if ( end == kEndChangeDirection )
            {
                CMedia::Playback p = (CMedia::Playback) step;
                if ( fg && step != 0 ) view->playback( p );
            }

            // LOGT_INFO( img->name() << " VIDEO LOOP END frame: " << frame
            //        << " step " << step );

            continue;
        }
        case CMedia::kDecodeMissingFrame:
            break;
        case CMedia::kDecodeBufferFull:
        default:
            break;
        }

        fps = img->play_fps();

        double delay = 1.0 / fps;

        double diff = 0.0;
        double bgdiff = 0.0;

        double absdiff;

        // // Calculate video-audio difference
        if ( img->has_audio() && status == CMedia::kDecodeOK )
        {

            double video_clock, master_clock;

            if ( step < 0 )
            {
                video_clock = img->video_clock();
                master_clock = img->audio_clock();
                // std::cerr  << " VC: " << video_clock
                //            << " MC: " << master_clock
                //            << " AC: " << get_clock(&img->audclk)
                //            << " EC: " << get_clock(&img->extclk)
                //            << std::endl;
                // img->debug_video_stores( frame, "play", true );
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

            absdiff = std::abs(diff);

            if ( absdiff > 10.0 ) diff = 0.0;
        }
        else
        {
            diff = 0.0;
        }

#if __cplusplus >= 201103L
        using std::isnan;
#endif
        if (! isnan(diff) )
        {

            img->avdiff( diff );

            // Skip or repeat the frame. Take delay into account
            //    FFPlay still doesn't "know if this is the best guess."
            if(absdiff < AV_NOSYNC_THRESHOLD) {
                double sdiff = step * diff;
                double sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN,
                                              FFMIN(AV_SYNC_THRESHOLD_MAX,
                                                    delay));

                if (sdiff <= -sync_threshold)
                {
                    delay = FFMAX(0.00001, delay + sdiff);
                }
                else if (sdiff >= sync_threshold &&
                         delay > AV_SYNC_FRAMEDUP_THRESHOLD)
                {
                    delay += sdiff;      // make fps slower
                }
                else if (sdiff >= sync_threshold) {
                    delay *= 2;      // make fps repeat frame
                }
            }
        }



        timer.setDesiredSecondsPerFrame( delay );
        timer.waitUntilNextFrameIsDue();


        // LOGT_INFO( "find image " << frame << " delay " << delay );
        //img->debug_video_packets( frame, "find_image", true );
        //img->debug_video_stores( frame, "find_image", true );

    
        if ( ! img->find_image( frame ) )
	{
	    LOG_ERROR( "Could not find image " << frame );
	}

        if ( reel->edl && fg && img->is_left_eye() )
        {
            int64_t f = frame + reel->location(img) - img->first_frame();
            view->frame( f );
        }

        frame += step;
    }

    Mutex& mtx = img->video_mutex();
    SCOPED_LOCK( mtx );

    CMedia::Barrier* barrier = img->loop_barrier();
    if ( barrier ) barrier->notify_all();
    barrier = img->fg_bg_barrier();
    if ( barrier ) barrier->notify_all();
    barrier = img->stereo_barrier();
    if ( barrier ) barrier->notify_all();

#ifdef DEBUG_THREADS
    LOGT_INFO( "EXIT  " << (fg ? "FG" : "BG") << " VIDEO THREAD "
              << img->name() << " stopped? " << img->stopped()
              << " view playback " << view->playback() << " at " << frame << "  img->frame: " << img->frame() );
    assert( img->stopped() );
#endif
    
    img->playback( CMedia::kStopped );


}  // video_thread




void decode_thread( PlaybackData* data )
{
    av_assert0( data != NULL );

    mrv::ViewerUI*     uiMain   = data->uiMain;
    av_assert0( uiMain != NULL );


    CMedia* img = data->image;
    av_assert0( img != NULL );



    bool fg = data->fg;

    mrv::ImageView*      view = uiMain->uiView;
    mrv::Timeline*      timeline = uiMain->uiTimeline;
    mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;
    av_assert0( timeline != NULL );

    // delete the data (we don't need it anymore)
    //delete data;

    int idx = fg ? view->fg_reel() : view->bg_reel();

    mrv::Reel   reel = browser->reel_at( idx );
    if (!reel) return;


    int step = (int) img->playback();


    int64_t frame = img->dts();

#ifdef DEBUG_THREADS
    LOGT_INFO( "ENTER " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped()  << " view playback " << view->playback()
              << " at frame " << frame
              << " step " << step );
#endif



    while ( !img->stopped() && view->playback() != CMedia::kStopped )
    {

        if ( img->seek_request() )
        {
            img->do_seek();
            frame = img->dts();
        }

        step = (int) img->playback();
        if ( step == 0 ) break;

        frame += step;
        CMedia::DecodeStatus status = check_decode_loop( frame, img, reel,
							 timeline );


        if ( status != CMedia::kDecodeOK )
        {
            if ( img->stopped() ) continue;

            CMedia::Barrier* barrier = img->loop_barrier();

            if ( barrier )
            {
                // DBG( img->name() << " BARRIER DECODE WAIT      gen: "
                //      << barrier->generation()
                //      << " count: " << barrier->count()
                //      << " threshold: " << barrier->threshold()
                //      << " used: " << barrier->used() );
                // Wait until all threads loop and decode is restarted
                bool ok = barrier->wait();
                // DBG( img->name() << " BARRIER DECODE LOCK PASS gen: "
                //      << barrier->generation()
                //      << " count: " << barrier->count()
                //      << " threshold: " << barrier->threshold()
                //      << " used: " << barrier->used() );
            }

            img->clear_packets();

            // Do the looping, taking into account ui state
            //  and return new frame and step.
            // This handle loop has to come after the barrier as decode thread
            // goes faster than video or audio threads
            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline, status );

            if ( img->stopped() ) continue;
            // img->seek( frame );
        }


        // If we could not get a frame (buffers full, usually),
        // wait a little.
        while ( !img->frame( frame ) )
        {
            if ( img->stopped() ||
		 view->playback() == CMedia::kStopped ) break;
            sleep_ms( 10 );
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
    LOGT_INFO( "EXIT  " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped() << " view playback " << view->playback() << " frame " << img->frame() << "  dts: " << img->dts() );
    assert( img->stopped() );
#endif

    img->playback( CMedia::kStopped );


}



} // namespace mrv
