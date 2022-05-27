/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

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

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_array.hpp>

#include <ImfIntAttribute.h>

#include "core/CMedia.h"
#include "core/aviImage.h"
#include "core/mrvMath.h"
#include "core/mrvTimer.h"
#include "core/mrvThread.h"
#include "core/mrvBarrier.h"

#include "gui/mrvIO.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvReel.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvImageView.h"
#include "gui/mrvImageBrowser.h"
#include "mrvPreferencesUI.h"
#include "mrvReelUI.h"
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


#define LOGT_WARNING(x) LOG_WARNING( std::hex << std::dec << " " << x );
#define LOGT_INFO(x) LOG_INFO( std::hex << std::dec << " " << x );
#define LOGT_ERROR(x) LOG_ERROR( std::hex << std::dec << " " << x );

#define DEBUG_THREADS

typedef boost::recursive_mutex Mutex;



namespace mrv {

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

    enum ThreadType
    {
        kDecode,
        kVideo,
        kAudio,
        kSubtitle
    };

enum EndStatus {
    kEndIgnore          = 0,
    kEndStop            = 1,
    kEndNextImage       = 2,
    kEndChangeDirection = 3,
    kEndLoop            = 4,
};

unsigned long get_thread_id(){
    std::locale::global( std::locale("C") );
    std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
    unsigned long threadNumber = 0;
    sscanf(threadId.c_str(), "%lx", &threadNumber);
    std::locale::global( std::locale("") );
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
                                 ThreadType decode,
                                 const mrv::Reel reel,
                                 const mrv::Timeline* timeline,
                                 int64_t& first,
                                 int64_t& last )
{
    int64_t mx = int64_t(timeline->display_maximum());
    int64_t mn = int64_t(timeline->display_minimum());


    int64_t gfirst, glast, gframe = frame;
    if ( reel->edl )
    {
        last  = img->out_frame();
        first = img->in_frame();

        gfirst = reel->local_to_global( first, img );
        glast  = reel->local_to_global( last,  img );
        gframe = reel->local_to_global( frame, img );

        if ( decode == kVideo )
        {
            TRACE( "-------------------------------------------------------------------------------" );
            TRACE( "CAL " << img->name() << " gfirst= " << gfirst << " out_frame= "
                   << img->out_frame() << " first_frame= " << img->first_frame() );
            TRACE( "CAL " << img->name() << " gframe= " << gframe << " in_frame= "
                   << img->in_frame() << " local frame= " << frame );
        }



    }
    else
    {
        first = img->first_frame();
        last = img->last_frame();


        gframe = frame;
        gfirst = first;
        glast  = last;
    }

    if ( mx < glast ) glast = mx;
    if ( mn > gfirst ) gfirst = mn;

    if ( decode == kVideo )
    {
        TRACE( "LCL " << img->name() << "  frame= " << frame  << "  first= "
               << first << "  last= " << last );
        TRACE( "GBL " << img->name() << " gframe= " << gframe  << " gfirst= "
               << gfirst << " glast= " << glast );
    }

    if ( gframe > glast )
    {
        if ( decode == kVideo )
            TRACE( img->name() <<  " LOOP END " << gframe << " > " << glast );
        return CMedia::kDecodeLoopEnd;
    }
    else if ( gframe < gfirst )
    {
        if ( decode == kVideo )
            TRACE( img->name() << "LOOP START " << gframe << " < " << gfirst );
        return CMedia::kDecodeLoopStart;
    }

    if ( decode == kVideo )
        TRACE( img->name() << " NO LOOP " << gframe << " > " << glast );

    return CMedia::kDecodeOK;
}

CMedia::DecodeStatus check_decode_loop( const int64_t frame,
                                        CMedia* img,
                                        const mrv::Reel reel,
                                        const mrv::Timeline* timeline )
{
    int64_t first, last;
    CMedia::DecodeStatus status = check_loop( frame, img, kDecode, reel,
                                              timeline, first, last );
    if ( status == CMedia::kDecodeLoopEnd )
    {
        // We need two loops at end to force clips
        img->loop_at_end( last+1 );
        img->loop_at_end( last+2 );
    }
    else if ( status == CMedia::kDecodeLoopStart )
    {
        // We need two loops at start to force clips
        img->loop_at_start( first-1 );
        img->loop_at_start( first-2 );
    }

    return status;
}



EndStatus handle_loop( int64_t& frame,
                       int&     step,
                       CMedia* img,
                       bool    fg,
                       bool init_time,
                       ViewerUI* uiMain,
                       const mrv::Reel  reel,
                       const mrv::Timeline* timeline,
                       mrv::CMedia::DecodeStatus end,
                       ThreadType decode = kVideo )
{

    if ( !img || !timeline || !reel || !uiMain ) return kEndIgnore;

    bool ignore = true;

    switch( decode )
    {
    case kDecode:
        ignore = false; break;
    case kVideo:
        if ( img->has_picture() ) ignore = false;
        break;
    case kAudio:
        if ( img->valid_audio() && !img->has_picture() ) {
            ignore = false;
            decode = kVideo;
        }
        break;
    default:
        ;
    }

    if ( ignore ) return kEndIgnore;


    mrv::ImageView* view = uiMain->uiView;


    EndStatus status = kEndIgnore;
    mrv::media c;
    CMedia* next = NULL;

    int64_t first, last;
    end = check_loop( frame, img, decode, reel, timeline, first, last );


    CMedia::Looping loop = view->looping();


    switch( end )
    {
    case CMedia::kDecodeLoopEnd:
    {

        if ( reel->edl )
        {

            int64_t f = frame;
            f -= img->in_frame();
            f += reel->location(img);

            int64_t dts = f;

            next = reel->image_at( f );
            TRACE( img->name() << " DTS IS " << dts << " frame " << frame );

            if ( next == NULL && loop == CMedia::kLoop )
            {
                first = dts = f = int64_t(timeline->display_minimum());
                next = reel->image_at( f );
                TRACE( decode << ") " << next->name() << " NEXT FIRST FRAME IS " << first << " stopped? " << next->stopped() );
            }
            else if ( next == img )
            {
                TRACE( decode << ") " << img->name() << " == NEXT " << next->name() );
                if ( loop == CMedia::kLoop )
                {
                    first = int64_t(timeline->display_minimum());
                }
                else if ( loop == CMedia::kPingPong )
                {
                    last = frame;
                }
            }

            if ( next != img && next != NULL )
            {
                if ( decode != kVideo ) return kEndIgnore;

                int64_t local = reel->global_to_local( dts );
                TRACE( decode << ") " << next->name() << " NEXT GLOBAL FRAME IS " << dts << ", LOCAL " << local << " stopped? " << next->stopped() );

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

                    img->playback( CMedia::kStopped );
                    img->clear_packets();
                    img->notify_barriers();
                    //img->flush_all();
                    TRACE( next->name() << " NEXT SEEK FRAME IS " << dts );

                    if ( ! fg )   return kEndNextImage;


                    CMedia::Mutex& cmtx = view->commands_mutex;
                    SCOPED_LOCK( cmtx );
                    {
                        ImageView::Command c;
                        c.type = ImageView::kSeek;
                        c.frame = dts;
                        view->commands.push_back( c );
                    }
                    {
                        ImageView::Command c;
                        c.type = ImageView::kPlayForwards;
                        c.frame = dts;
                        view->commands.push_back( c );
                    }
                    return kEndNextImage;
                }

                img->playback( CMedia::kStopped );
                img->clear_packets();
                //img->flush_all();

                int idx = reel->index( next );
                TRACE( "******** " << next->name()
                        << " CHANGE IMAGE TO " << idx
                        << " stopped? " << next->stopped()
                        << " frame " << next->frame() );

                CMedia::Mutex& cmtx = view->commands_mutex;
                SCOPED_LOCK( cmtx );
                {
                    ImageView::Command c;
                    c.type = ImageView::kChangeImage;
                    c.frame = idx;
                    view->commands.push_back( c );
                }
                return kEndNextImage;
            }
        }


        if ( loop == CMedia::kLoop )
        {
            img->clear_packets();

            frame = reel->global_to_local( first );
            TRACE( decode << ") DECODE LOOP to " << first
                    << " frame " << frame );
            if ( next == img )
            {
                img->seek( frame );
            }
            else if ( fg && reel->edl )
            {
                img->playback( CMedia::kStopped );
                CMedia::Mutex& cmtx = view->commands_mutex;
                SCOPED_LOCK( cmtx );
                ImageView::Command c;
                c.type = ImageView::kSeek;
                c.frame = first;
                view->commands.push_back( c );
            }

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
            img->playback( CMedia::kStopped );
        }
        break;
    }
    case CMedia::kDecodeLoopStart:
    {
        if ( reel->edl )
        {
            int64_t f = frame;

            f -= img->in_frame();
            f += reel->location(img);

            int64_t dts = f;

            next = reel->image_at( f );

            if ( next == NULL && loop == CMedia::kLoop )
            {
                last = dts = f = int64_t(timeline->display_maximum());
                next = reel->image_at( f );
            }
            else if ( next == img )
            {
                if ( loop == CMedia::kLoop )
                {
                    last = int64_t(timeline->display_maximum());
                }
                else if ( loop == CMedia::kPingPong )
                {
                    first = frame;
                }
            }

            if ( next != img && next != NULL )
            {
                if ( decode != kVideo ) return kEndIgnore;

                if ( next->stopped()  )
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

                    img->playback( CMedia::kStopped );
                    img->clear_packets();
                    img->notify_barriers();
                    //img->flush_all();

                    if ( ! fg )   return kEndNextImage;

                    CMedia::Mutex& cmtx = view->commands_mutex;
                    SCOPED_LOCK( cmtx );
                    {
                        ImageView::Command c;
                        c.type = ImageView::kSeek;
                        c.frame = dts;
                        view->commands.push_back( c );
                    }
                    {
                        ImageView::Command c;
                        c.type = ImageView::kPlayBackwards;
                        c.frame = dts;
                        view->commands.push_back( c );
                    }

                    return kEndNextImage;
                }

                img->playback( CMedia::kStopped );
                img->clear_packets();
                //img->flush_all();

                int idx = reel->index( next );
                CMedia::Mutex& cmtx = view->commands_mutex;
                TRACE( "******** " << next->name()
                        << " CHANGE IMAGE TO " << idx
                        << " stopped? " << next->stopped()
                        << " frame " << next->frame() );
                SCOPED_LOCK( cmtx );
                {
                    ImageView::Command c;
                    c.type = ImageView::kChangeImage;
                    c.frame = idx;
                    view->commands.push_back( c );
                }
                return kEndNextImage;
            }
        }


        if ( loop == CMedia::kLoop )
        {
            img->clear_packets();

            frame = reel->global_to_local( last );

            if ( next == img )
            {
                img->seek( frame );
            }
            else if ( fg && reel->edl )
            {
                img->playback( CMedia::kStopped );
                CMedia::Mutex& cmtx = view->commands_mutex;
                SCOPED_LOCK( cmtx );
                {
                    ImageView::Command c;
                    c.type = ImageView::kSeek;
                    c.frame = last;
                    view->commands.push_back( c );
                }
            }

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
        }
        break;
    }
    default:
    {
        return kEndIgnore;
    }
    }

    if ( status == kEndStop || status == kEndNextImage )
    {
        img->playback( CMedia::kStopped );
    }


    return status;
}




//
// Main loop used to play audio (of any image)
//
void audio_thread( PlaybackData* data )
{
    assert( data != NULL );

    ViewerUI*     uiMain   = data->uiMain;
    assert( uiMain != NULL );
    CMedia* img = data->image;
    assert( img != NULL );

    bool fg = data->fg;

    // delete the data (we don't need it anymore)
    delete data;





    mrv::ImageView*      view = uiMain->uiView;
    mrv::Timeline*      timeline = uiMain->uiTimeline;
    mrv::ImageBrowser*   browser = uiMain->uiReelWindow->uiBrowser;

    int64_t frame = img->audio_frame();

    int idx = fg ? view->fg_reel() : view->bg_reel();

    mrv::Reel   reel = browser->reel_at( idx );
    if (!reel) return;

#ifdef DEBUG_THREADS
    TRACE( "ENTER " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? " << img->stopped() << " frame " << frame );
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

        img->wait_audio();



        int64_t f = frame;

        // if (!fg)
        //img->debug_audio_packets( frame, "play", false );

        // std::cerr << "decode audio " << frame << std::endl;

        CMedia::DecodeStatus status = img->decode_audio( f );
        // std::cerr << img->name() << " decoded audio " << f
        //           << " status " << status << std::endl;

        int64_t first, last;
        status = check_loop( frame, img, kAudio, reel, timeline, first, last );


        assert( img != NULL );
        assert( reel != NULL );
        assert( timeline != NULL );

        switch( status )
        {
        case CMedia::kDecodeError:
            LOGT_ERROR( img->name()
                       << _(" - decode Error audio frame ") << frame );
            frame += step;
            continue;
        case CMedia::kDecodeMissingSamples:
        case CMedia::kDecodeMissingFrame:
            timer.setDesiredFrameRate( img->play_fps() );
            timer.waitUntilNextFrameIsDue();
            frame += step;
            continue;
        case CMedia::kDecodeNoStream:
            timer.setDesiredFrameRate( img->play_fps() );
            timer.waitUntilNextFrameIsDue();
            if ( fg && !img->has_picture() && img->is_left_eye() )
            {
                int64_t f = frame + reel->location(img) - img->in_frame();
                f -= img->audio_offset();
                view->frame( f );
            }
            frame += step;
            continue;
        case  CMedia::kDecodeLoopEnd:
        case  CMedia::kDecodeLoopStart:
        {


            TRACE( img->name() << " BARRIER IN AUDIO " << frame );


            CMedia::Barrier* barrier = img->loop_barrier();
            if ( barrier )
            {
                DBGM1( img->name() << " BARRIER AUDIO WAIT      gen: "
                       << barrier->generation()
                       << " count: " << barrier->count()
                       << " threshold: " << barrier->threshold()
                       << " used: " << barrier->used() );
                // Wait until all threads loop and decode is restarted
                bool ok = barrier->wait();
                DBGM1( img->name() << " BARRIER PASSED IN AUDIO  gen: "
                       << barrier->generation()
                       << " count: " << barrier->count()
                       << " threshold: " << barrier->threshold()
                       << " used: " << barrier->used() );
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

            img->clear_audio_packets();

            frame -= img->audio_offset();

            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline,
                                         status, kAudio );

            frame += img->audio_offset();

            // This has to come here not in handle_loop, to avoid
            // setting playback dir on decode thread
            if ( end == kEndChangeDirection  )
            {
                CMedia::Playback p = (CMedia::Playback) step;
                if ( fg ) view->playback( p );
            }

            DBGM1( img->name() << " AUDIO LOOP END/START HAS FRAME " << frame );
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



        if ( fg && !img->has_picture() && img->is_left_eye() )
        {
            int64_t offset = img->audio_offset();
            int64_t f = frame + reel->location(img) - img->in_frame();
            f -= offset;
            view->frame( f );
        }


        if ( !img->stopped() )
        {
            img->find_audio(frame);
#ifdef _WIN32 // WIN32 stores application audio volume
            mrv::AudioEngine* engine = img->audio_engine();
            if ( engine )
            {
                uiMain->uiVolume->value( engine->volume() );
            }
#endif
        }

        if ( reel->edl ) frame = img->audio_frame();
        frame += step;
        TRACE( img->name() << " LOCAL AUDIO FRAME " << frame );
    }

    img->playback( CMedia::kStopped );

    CMedia::Barrier* barrier = img->loop_barrier();
    if ( barrier ) {
        TRACE( img->name() << " BARRIER NOTIFY ALL IN AUDIO" );
        barrier->notify_all();
    }

    barrier = img->fg_bg_barrier();
    if ( barrier ) {
        // barrier->notify_all();
        // if ( !fg ) {
        //     delete barrier;
        // }
        // img->fg_bg_barrier( NULL );
    }

    img->close_audio();

#ifdef DEBUG_THREADS
    TRACE( "EXIT " << (fg ? "FG" : "BG") << " AUDIO THREAD " << img->name() << " stopped? "  << img->stopped() << " frame " << img->audio_frame() );
#endif

} // audio_thread



//
// Main loop used to decode subtitles
//
void subtitle_thread( PlaybackData* data )
{
    assert( data != NULL );

    ViewerUI*     uiMain   = data->uiMain;

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
    TRACE( "ENTER SUBTITLE THREAD " << img->name() );
#endif


    while ( !img->stopped() && view->playback() != CMedia::kStopped )
    {
        int step = (int) img->playback();
        if ( step == 0 ) break;

        int64_t frame = img->frame() + step;

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
        default:
            continue;
        case CMedia::kDecodeLoopEnd:
        case CMedia::kDecodeLoopStart:


            CMedia::Barrier* barrier = img->loop_barrier();
            if ( barrier )
            {
                // Wait until all threads loop and decode is restarted
                barrier->wait();
            }

            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline, status,
                                         kSubtitle );
            continue;
        }

        double fps = img->play_fps();
        timer.setDesiredFrameRate( fps );
        timer.waitUntilNextFrameIsDue();
    }

    img->playback( CMedia::kStopped );

    CMedia::Barrier* barrier = img->loop_barrier();
    if ( barrier ) {
        TRACE( img->name() << " BARRIER NOTIFY ALL IN AUDIO" );
        barrier->notify_all();
    }

#ifdef DEBUG_THREADS
    TRACE( "EXIT  SUBTITLE THREAD " << img->name()
              << " stopped? " << img->stopped() );
    assert( img->stopped() );
#endif

}  // subtitle_thread


//#undef DEBUG
// #define DEBUG(x)

//#define DEBUG(x) std::cerr << x << std::endl

//
// Main loop used to play video (of any image)
//
void video_thread( PlaybackData* data )
{
    assert( data != NULL );

    ViewerUI*     uiMain   = data->uiMain;
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
        uint64_t d = reel->duration();
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
                uint64_t d = bimg->duration();
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

    int64_t frame;
    {
        // We lock the video mutex to make sure frame was properly updated
        boost::recursive_mutex::scoped_lock lk( img->video_mutex() );

        frame = img->frame();
    }


#ifdef DEBUG_THREADS
    TRACE( "ENTER " << (fg ? "FG" : "BG") << " VIDEO THREAD " << img->name() << " stopped? " << img->stopped() << " view playback "
            << view->playback() << " frame " << frame );
#endif


    mrv::Timer timer;
    double fps = img->play_fps();
    timer.setDesiredFrameRate( fps );

    while ( !img->stopped() && view->playback() != CMedia::kStopped )
    {
        TRACE( img->name() );
        img->wait_image();
        TRACE( img->name() );

        int step = (int) img->playback();
        if ( step == 0 ) break;

        TRACE( img->name() );

        CMedia::DecodeStatus status = img->decode_video( frame );

        TRACE( img->name() );

        int64_t first, last;
        status = check_loop( frame, img, kVideo, reel, timeline, first, last );

        TRACE( img->name() );
        // img->debug_video_packets( frame, img->name().c_str(), true );
        // img->debug_video_stores( frame, img->name().c_str(), true );


        switch( status )
        {
            // case CMedia::DecodeDone:
            //    continue;
        case CMedia::kDecodeError:
            LOGT_ERROR( img->name() << _(" - Decode of image frame ") << frame
                       << _(" returned ") << get_error_text( status ) );
            break;
        case CMedia::kDecodeLoopEnd:
        case CMedia::kDecodeLoopStart:
        {
            TRACE( img->name() );


            CMedia::Barrier* barrier = img->loop_barrier();

            if ( barrier )
            {
                DBGM1( img->name() << " BARRIER VIDEO WAIT      gen: "
                     << barrier->generation()
                     << " count: " << barrier->count()
                     << " threshold: " << barrier->threshold()
                     << " used: " << barrier->used() );
                // Wait until all threads loop and decode is restarted
                bool ok = barrier->wait();
                DBGM1( img->name() << " BARRIER PASSED IN VIDEO  gen: "
                       << barrier->generation()
                       << " count: " << barrier->count()
                       << " threshold: " << barrier->threshold()
                       << " used: " << barrier->used() );
            }

            // img->debug_video_packets( frame, "debug", true );
            img->clear_video_packets();

            barrier = img->fg_bg_barrier();
            if ( barrier )
            {
                // LOGT_WARNING( img->name() << " FG/BG BARRIER " << barrier << " VIDEO FG/BG LOCK gen: "
                //      << barrier->generation()
                //      << " count: " << barrier->count()
                //      << " threshold: " << barrier->threshold()
                //      << " used: " << barrier->used() );
                bool ok = barrier->wait();
                // LOGT_INFO( img->name() << " BARRIER " << barrier
                //            << " VIDEO FG/BG PASS gen: "
                //            << barrier->generation()
                //            << " count: " << barrier->count()
                //            << " threshold: " << barrier->threshold()
                //            << " used: " << barrier->used() );
            }


            barrier = img->stereo_barrier();
            // LOGT_INFO( img->name() << "@" << barrier << " wait" );
            if ( barrier )
            {
                bool ok = barrier->wait();
            }
            // LOGT_INFO( img->name() << "@" << barrier << " waited" );

            DBGM3( img->name() << " BARRIER PASSED IN VIDEO stopped? "
                 << img->stopped() << " frame: " << frame );


            DBGM3( img->name() << " VIDEO LOOP frame: " << frame );

            int64_t f = frame;
            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline,
                                         status, kVideo );

            DBGM3( img->name() << " end: " << end
                      << " Stopped? " << img->stopped() );

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
            // img->decode_eof( frame );
            break;
        case CMedia::kDecodeBufferFull:
        default:
            break;
        }

        fps = img->play_fps();

        double delay = 1.0 / fps;

        double diff = 0.0;
        double absdiff = 0.0;

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

        img->actual_frame_rate( timer.actualFrameRate() );

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
                    if ( ! uiMain->uiPrefs->uiPrefsPlayAllFrames->value() )
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



        // LOGT_INFO( "find image " << frame << " delay " << delay );
        //img->debug_video_packets( frame, "find_image", true );
        //img->debug_video_stores( frame, "find_image", false );

        if ( ! img->find_image( frame ) )
        {
            LOG_ERROR( _("Could not find image ") << frame );
        }


        if ( fg && img->is_left_eye() )
        {
            int64_t f = frame;
            mrv::media m = view->foreground();
            if ( !reel->edl || (m && m->image() == img) )
            {
                if ( reel->edl )
                {
                    f = reel->local_to_global( frame, img );
                }
                TRACE2( "SEND GLOBAL FRAME " << f << ", LOCAL " << frame
                        << " FROM IMAGE " << img->name() );
                view->frame( f );

                if ( reel->edl )
                {
                    CMedia* Aimg = view->A_image();
                    CMedia* Bimg = view->B_image();
                    CMedia::Playback play = view->playback();
                    if ( Aimg && play == CMedia::kForwards )
                    {
                        int64_t Aframe = Aimg->frame();
                        int64_t Aout   = Aimg->out_frame();
                        TRACE( Aimg->name() << " CHECK DISSOLVE END LOOP "
                                << Aframe << " out= " << Aout );
                        if ( Aframe >= Aout )
                        {
                            Aframe++;
                            TRACE( Aimg->name() << " SEND DISSOLVE END LOOP "
                                    << Aframe-1 << " GLOBAL " << f );
                            EndStatus end = handle_loop( Aframe, step, Aimg,
                                                         fg, true,
                                                         uiMain, reel, timeline,
                                                         status, kVideo );
                            if ( end == kEndChangeDirection )
                            {
                                CMedia::Playback p = (CMedia::Playback) step;
                                if ( fg && step != 0 ) view->playback( p );
                                continue;
                            }
                            TRACE( Aimg->name()
                                    << " SENT DISSOLVE END LOOP stopped? "
                                    << Aimg->stopped() );
                            if ( img == Aimg ) break;
                        }
                    }
                    else if ( Bimg && play == CMedia::kBackwards )
                    {
                        int64_t Bframe = Bimg->frame();
                        int64_t Bin    = Bimg->in_frame();
                        TRACE( Bimg->name() << " CHECK DISSOLVE START LOOP "
                                << Bframe << " in= " << Bin );
                        if ( Bframe <= Bin )
                        {
                            Bframe--;
                            TRACE( Bimg->name() << " SEND DISSOLVE START LOOP "
                                    << Bframe+1 << " GLOBAL " << f );
                            EndStatus end = handle_loop( Bframe, step, Bimg,
                                                         fg, true,
                                                         uiMain, reel, timeline,
                                                         status, kVideo );
                            if ( end == kEndChangeDirection )
                            {
                                CMedia::Playback p = (CMedia::Playback) step;
                                if ( fg && step != 0 ) view->playback( p );
                                continue;
                            }
                            TRACE( Bimg->name()
                                    << " SENT DISSOLVE START LOOP stopped? "
                                    << Bimg->stopped() );
                            if ( Bimg == img ) break;
                        }
                    }
                }
            }
            else
            {
                TRACE( "NO GLOBAL FRAME " << f << ", LOCAL " << frame
                       << " FROM IMAGE " << img->name() );
            }
        }

        timer.setDesiredSecondsPerFrame( delay );
        timer.waitUntilNextFrameIsDue();

        if ( reel->edl ) frame = img->frame();
        frame += step;
    }

    img->playback( CMedia::kStopped );

    Mutex& mtx = img->video_mutex();
    SCOPED_LOCK( mtx );

    CMedia::Barrier* barrier = img->loop_barrier();
    if ( barrier )
    {
        TRACE( img->name() << " BARRIER NOTIFY ALL IN VIDEO" );
        barrier->notify_all();
    }


    barrier = img->fg_bg_barrier();
    if ( barrier ) barrier->notify_all();
    barrier = img->stereo_barrier();
    if ( barrier ) barrier->notify_all();


#ifdef DEBUG_THREADS
    TRACE( "EXIT  " << (fg ? "FG" : "BG") << " VIDEO THREAD "
            << img->name() << " stopped? " << img->stopped()
            << " view playback " << view->playback() << " at " << frame << "  img->frame: " << img->frame() );
#endif


}  // video_thread




void decode_thread( PlaybackData* data )
{
    assert( data != NULL );

    ViewerUI*     uiMain   = data->uiMain;
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
    TRACE( "ENTER " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped()  << " view playback " << view->playback()
            << " at frame " << frame
            << " step " << step );
#endif


    while ( !img->stopped() && view->playback() != CMedia::kStopped )
    {

        if ( img->seek_request() )
        {
            TRACE( ">>>>>>   pre seek " << frame );
            img->do_seek();
            frame = img->dts();
            TRACE( ">>>>>> after seek " << frame );
        }


        step = (int) img->playback();
        if ( step == 0 ) break;


        frame += step;
        CMedia::DecodeStatus status = check_decode_loop( frame, img, reel,
                                                         timeline );


        if ( status != CMedia::kDecodeOK )
        {

            CMedia::Barrier* barrier = img->loop_barrier();

            if ( barrier )
            {
                DBGM1( img->name() << " BARRIER DECODE WAIT      gen: "
                     << barrier->generation()
                     << " count: " << barrier->count()
                     << " threshold: " << barrier->threshold()
                     << " used: " << barrier->used() );
                // Wait until all threads loop and decode is restarted
                bool ok = barrier->wait();
                DBGM1( img->name() << " BARRIER PASSED IN DECODE  gen: "
                       << barrier->generation()
                       << " count: " << barrier->count()
                       << " threshold: " << barrier->threshold()
                       << " used: " << barrier->used() );
            }

            // img->clear_packets();

            // Do the looping, taking into account ui state
            //  and return new frame and step.
            // This handle loop has to come after the barrier as decode thread
            // goes faster than video or audio threads
            EndStatus end = handle_loop( frame, step, img, fg, true,
                                         uiMain, reel, timeline, status,
                                         kDecode );

            if ( img->stopped() ) continue;
        }

        // If we could not get a frame (buffers full, usually),
        // wait a little.
        while ( !img->frame( frame ) )
        {
            if ( img->stopped() ||
                 view->playback() == CMedia::kStopped ) break;
            sleep_ms( 20 );
        }


        // After read, when playing audio files,
        // decode position may be several frames advanced as we buffer
        // multiple frames, so get back the dts frame from image.
        // When playing backwards, we get the previous frame to force a re-read
        // if needed.  See aviImage::seek_to_position().

        if ( img->has_video() || img->has_audio() )
        {
            frame = img->dts();
        }


    }


    img->playback( CMedia::kStopped );

    CMedia::Barrier* barrier = img->loop_barrier();
    if ( barrier )
    {
        TRACE( img->name() << " BARRIER NOTIFY ALL IN DECODE" );
        barrier->notify_all();
    }


#ifdef DEBUG_THREADS
    TRACE( "EXIT  " << (fg ? "FG" : "BG") << " DECODE THREAD " << img->name() << " stopped? " << img->stopped() << " view playback " << view->playback() << " frame " << img->frame() << "  dts: " << img->dts() );
#endif


}



} // namespace mrv
