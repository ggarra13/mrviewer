#include "R3dImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if !defined(WIN32) && !defined(WIN64)
#  include <arpa/inet.h>
#else
#  include <winsock2.h>    // for htonl
#endif

extern "C" {
#include <libavutil/time.h>
}

#include "ImfStringAttribute.h"

#include "core/mrvFrameFunctors.h"
#include "gui/mrvPreferences.h"
#include "mrvPreferencesUI.h"
#include "R3DSDK.h"


using namespace R3DSDK;

namespace {
    const char* kModule = "r3d";
}


namespace mrv {

    bool R3dImage::_init = false;

    R3dImage::R3dImage() :
        CMedia(),
        clip( NULL ),
        iproc( NULL ),
        _scale( 0 )
    {
    }

    R3dImage::~R3dImage()
    {
    }

    bool R3dImage::test( const char* file )
    {
        if ( file == NULL ) return false;

        if ( !_init )
        {
            //Initialize the R3DSDK prior to using any R3DSDK objects.
            std::string root = Preferences::root + "/lib";

            InitializeStatus status = InitializeSdk(root.c_str(),
                                                OPTION_RED_NONE);
            if ( status != ISInitializeOK)
            {
                LOG_ERROR( _("Failed to initialize R3D SDK: ") << status);
                return false;
            }

            _init = true;
        }

        if ( strcasecmp( file+strlen(file)-4, ".rmd" ) == 0 )
            return false;

        Clip* clip = new Clip( file );

        if ( !clip ) {
            return false;
        }
        delete clip;

        return true;
    }

    bool R3dImage::initialize()
    {
        // load the clip
        if ( !clip )
        {
            clip = new Clip( filename() );

            // let the user know if this failed
            if (clip->Status() != LSClipLoaded)
            {
                IMG_ERROR( _("Error loading file") );
                finalize();
                return false;
            }

            _fps = _play_fps = _orig_fps = clip->VideoAudioFramerate();
            _frameStart = _frame = _frame_start = 0;
            _frameEnd = _frame_end = clip->VideoFrameCount() - 1;

            _attrs.insert( std::make_pair( 0, Attributes() ) );
            for ( size_t i = 0; i < clip->MetadataCount(); ++i )
            {
                const std::string& name = clip->MetadataItemKey(i);
                Imf::StringAttribute attr( clip->MetadataItemAsString(i) );
                _attrs[0].insert( std::make_pair( name,
                                                  attr.copy() ) );
            }
            image_damage( image_damage() | kDamageData );
        }
        return true;
    }

    bool R3dImage::finalize()
    {
        delete clip; clip = NULL;
        delete iproc; iproc = NULL;
        // We call finalizeSdk in mainWindow as it is not reentrant it seems.
        return true;
    }


    void R3dImage::clear_cache()
    {
        SCOPED_LOCK( _mutex );
        _images.clear();
    }

    CMedia::Cache R3dImage::is_cache_filled( int64_t frame )
    {
        SCOPED_LOCK( _mutex );
        bool ok = false;

        // Check if video is already in video store
        video_cache_t::iterator end = _images.end();
        video_cache_t::iterator i = std::find_if( _images.begin(), end,
                                                  EqualFunctor(frame) );
        if ( i != end ) ok = true;

        if ( ok && _stereo_input != kSeparateLayersInput ) return kStereoCache;
        return (CMedia::Cache) ok;
    }

    bool R3dImage::fetch( mrv::image_type_ptr& canvas,
                          const boost::int64_t frame )
    {
        if ( !clip )
        {
            IMG_ERROR( _("Clip is undefined") );
            return false;
        }


        if ( cache_scale() != _scale )
        {
            clear_cache();
            _video_packets.clear();
        }

        _scale = cache_scale();

        // calculate how much ouput memory we're going to need
        size_t dw = clip->Width();
        size_t dh = clip->Height(); // going to do a full resolution decode

        if ( is_thumbnail() ) _scale = 3;

        if ( _scale == 1 )
        {
            dw = clip->Width() / 2;
            dh = clip->Height() / 2; // going to do a half resolution decode
        }
        else if ( _scale == 2 )
        {
            dw = clip->Width() / 4;
            dh = clip->Height() / 4; // going to do a quarter resolution decode
        }
        else if ( _scale == 3 )
        {
            dw = clip->Width() / 8;
            dh = clip->Height() / 8; // going to do a 1/8th resolution decode
        }

        if ( _layers.empty() )
        {
            _num_channels = 0;
            rgb_layers();
            lumma_layers();
        }

#if 1
#define pixel_type kHalf
#define channels kRGB
#define decode_type PixelType_HalfFloat_RGB_Interleaved
#endif

#if 0
#define pixel_type kShort
#define channels kRGB
#define decode_type PixelType_16Bit_RGB_Interleaved
#endif

        image_size( dw, dh );
        allocate_pixels( canvas, frame, 3, image_type::channels,
                         image_type::pixel_type );


        // alloc this memory 16-byte aligned
        unsigned char * imgbuffer = (unsigned char*)canvas->data().get();



        // create and fill out a decode job structure so the
        // decoder knows what you want it to do
        VideoDecodeJob job;

        // calculate the bytes per row, for an interleaved
        // 16-bit image this is the width times 6
        job.BytesPerRow = dw * sizeof(half) * 3U;

        // letting the decoder know how big the buffer is (we do that here
        // since AlignedMalloc below will overwrite the value in this
        job.OutputBufferSize = dw * dh * 3U * sizeof(half);

        job.ImageProcessing = NULL;  // iproc (does not work!)
        job.HdrProcessing = NULL;

        switch( _scale )
        {
        default:
        case 0:
            // decode at full resolution at premium quality
            job.Mode = DECODE_FULL_RES_PREMIUM;
            break;
        case 1:
            // decode at half resolution at premium quality
            job.Mode = DECODE_HALF_RES_PREMIUM;
            break;
        case 2:
            // decode at quarter resolution at good quality
            job.Mode = DECODE_QUARTER_RES_GOOD;
            break;
        case 3:
            // decode at eight resolution at good quality
            job.Mode = DECODE_EIGHT_RES_GOOD;
            break;
        }

        // store the image here
        job.OutputBuffer = imgbuffer;

        // store the image in a 16-bit interleaved RGB or
        // in half interleaved RGB format
        job.PixelType = decode_type;

        _pixel_ratio = 1.0;
        _gamma = 1.0f;

        if ((clip->VideoTrackCount() == 2U) ||
            (clip->MetadataItemAsInt(RMD_HDR_MODE) == 2))
        {
            if ( clip->VideoTrackDecodeFrame( 0U, frame, job ) != DSDecodeOK )
            {
                IMG_ERROR( _("Decode failed for frame ") << frame );
                return false;
            }


            image_type_ptr hdr;
            allocate_pixels( hdr, frame, 3, image_type::channels,
                             image_type::pixel_type );

            job.OutputBuffer = hdr->data().get();
            if ( clip->VideoTrackDecodeFrame( 1U, frame, job ) != DSDecodeOK )
            {
                IMG_ERROR( _("Decode failed for HDR frame ") << frame );
                return false;
            }

            size_t track = 0;
            HdrProcessingSettings settings;
            HdrMode mode = clip->GetRmdHdrProcessingSettings( settings, track );

            float t = (settings.Bias + 1.0f) / 2.0f;
            float t2 = 1.0f - t;

            for ( unsigned y = 0; y < dh; ++y )
            {
                for ( unsigned x = 0; x < dw; ++x )
                {
                    CMedia::Pixel p = canvas->pixel( x, y );
                    CMedia::Pixel p2 = hdr->pixel( x, y );
                    p.r *= t;
                    p.g *= t;
                    p.b *= t;
                    p.r += p2.r * t2;
                    p.g += p2.g * t2;
                    p.b += p2.b * t2;
                    canvas->pixel( x, y, p );
                }
            }
        }
        else
        {
            // decode the first frame (0) of the clip
            if (clip->DecodeVideoFrame(frame, job) != DSDecodeOK)
            {
                IMG_ERROR( _("Decode failed for frame ") << frame );
                return false;
            }
        }

        SCOPED_LOCK( _mutex );

        if ( _images.empty() || _images.back()->frame() < frame )
        {
            _images.push_back( canvas );
        }
        else
        {
            video_cache_t::iterator at = std::lower_bound( _images.begin(),
                                                           _images.end(),
                                                           frame,
                                                           LessThanFunctor() );


            // Avoid storing duplicate frames, replace old frame with this one
            if ( at != _images.end() )
            {
                if ( (*at)->frame() == frame )
                {
                    at = _images.erase(at);
                }
            }

            _images.insert( at, canvas );
        }


        return true;
    }

    bool R3dImage::frame( const int64_t f )
    {
        if ( Preferences::max_memory <= CMedia::memory_used )
        {
            if ( stopped() ) return false;
            limit_video_store( f );
            if ( has_audio() ) limit_audio_store( f );
        }


//  in ffmpeg, sizes are in bytes...
#define MAX_VIDEOQ_SIZE (5 * 2048 * 1024)
#define MAX_AUDIOQ_SIZE (5 * 60 * 1024)
#define MAX_SUBTITLEQ_SIZE (5 * 30 * 1024)
        if (
        _video_packets.bytes() > MAX_VIDEOQ_SIZE ||
        _audio_packets.bytes() > MAX_AUDIOQ_SIZE ||
        _subtitle_packets.bytes() > MAX_SUBTITLEQ_SIZE  )

        {
            return false;
        }

        if ( f < _frameStart )    _dts = _adts = _frameStart;
        else if ( f > _frameEnd ) _dts = _adts = _frameEnd;
        else                      _dts = _adts = f;

        AVPacket pkt;
        av_init_packet( &pkt );
        pkt.dts = pkt.pts = _dts;
        pkt.size = 0;
        pkt.data = NULL;

        if ( ! is_cache_filled( _dts ) )
        {
            image_type_ptr canvas;
            if ( fetch( canvas, _dts ) )
            {
                default_color_corrections();
            }
        }

        _video_packets.push_back( pkt );

        if ( has_audio() )
        {
            fetch_audio( f + _audio_offset );
        }

        _expected = _dts + 1;
        _expected_audio = _expected + _audio_offset;
        return true;
    }

    bool R3dImage::find_image( const int64_t frame )
    {

        if ( _right_eye && (stopped() || saving() ) )
            _right_eye->find_image( frame );

        assert0( frame != AV_NOPTS_VALUE );
        assert0( frame >= _frameStart );
        assert0( frame <= _frameEnd );

#ifdef DEBUG_VIDEO_PACKETS
        debug_video_packets(frame, "find_image");
#endif

#ifdef DEBUG_VIDEO_STORES
        debug_video_stores(frame, "find_image", true);
#endif


        _frame = frame;


        {
            SCOPED_LOCK( _mutex );

            int64_t f = frame - _start_number;

            video_cache_t::iterator end = _images.end();
            video_cache_t::iterator i;

            if ( playback() == kBackwards )
            {
                i = std::upper_bound( _images.begin(), end,
                                      f, LessThanFunctor() );
            }
            else
            {
                i = std::lower_bound( _images.begin(), end,
                                      f, LessThanFunctor() );
            }

            if ( i != end && *i )
            {
                _hires = *i;

                int64_t distance = f - _hires->frame();


                if ( distance > _hires->repeat() )
                {
                    int64_t first = (*_images.begin())->frame();
                    video_cache_t::iterator end = std::max_element( _images.begin(),
                                                                    _images.end() );
                    int64_t last  = (*end)->frame();
                    boost::uint64_t diff = last - first + 1;
                    IMG_ERROR( _("Video Sync master frame ") << f
                               << " != " << _hires->frame()
                               << _(" video frame, cache ") << first << "-" << last
                               << " (" << diff << _(") cache size: ") << _images.size()
                               << " dts: " << _dts );
                    //  debug_video_stores(frame);
                    //  debug_video_packets(frame);
                }
            }
            else
            {
                // Hmm... no close image was found.  If we have some images in
                // cache, we choose the last one in it.  This avoids problems if
                // the last frame is the one with problem.
                // If not, we fail.

                if ( ! _images.empty() )
                {
                    _hires = _images.back();

                    uint64_t diff = abs(f - _hires->frame() );

                    static short counter = 0;

                    if ( _hires->frame() != f &&
                         diff > 1 && diff < 10 && counter < 10 &&
                         f <= _frameEnd )
                    {
                        ++counter;
                        IMG_WARNING( _("find_image: frame ") << frame
                                     << _(" not found, choosing ")
                                     << _hires->frame()
                                     << _(" instead") );
                    }
                    else
                    {
                        if ( diff == 0 ) counter = 0;
                    }
                }
                else
                {
                    IMG_ERROR( _("find_image: frame ") << frame << _(" not found") );
                    return false;
                }
            }



            // Limit (clean) the video store as we play it
            limit_video_store( f );

            _video_pts   = f  / _fps; //av_q2d( get_video_stream()->avg_frame_rate );
            _video_clock = double(av_gettime_relative()) / 1000000.0;

            update_video_pts(this, _video_pts, 0, 0);

        }  // release lock


        refresh();

        return true;
    }

    void R3dImage::timed_limit_store( const int64_t frame )
    {
        uint64_t max_frames = max_image_frames();

#undef timercmp
# define timercmp(a, b, CMP)                    \
        (((a).tv_sec == (b).tv_sec) ?           \
         ((a).tv_usec CMP (b).tv_usec) :        \
         ((a).tv_sec CMP (b).tv_sec))

        struct customMore {
            inline bool operator()( const timeval& a,
                                    const timeval& b ) const
                {
                    return timercmp( a, b, > );
                }
        };

        typedef std::multimap< timeval, video_cache_t::iterator,
                               customMore > TimedSeqMap;
        TimedSeqMap tmp;
        {
            video_cache_t::iterator  it = _images.begin();
            video_cache_t::iterator end = _images.end();
            for ( ; it != end; ++it )
            {
                tmp.insert( std::make_pair( (*it)->ptime(), it ) );
            }
        }

        // For backwards playback, we consider _dts to not remove so
        // many frames.
        if ( playback() == kBackwards )
        {
            max_frames = frame + max_frames;
            if ( _dts > frame ) max_frames = _dts + max_frames;
        }


        unsigned count = 0;
        TimedSeqMap::iterator it = tmp.begin();
        typedef std::vector< video_cache_t::iterator > IteratorList;
        IteratorList iters;
        for ( ; it != tmp.end(); ++it )
        {
            ++count;
            if ( count > max_frames )
            {
                // Store this iterator to remove it later
                iters.push_back( it->second );
            }
        }

        if ( iters.empty() ) return;


        _images.erase( std::remove_if( _images.begin(), _images.end(),
                                       IteratorMatch<IteratorList>( iters ) ),
                       _images.end() );

    }

//
// Limit the video store to approx. max frames images on each side.
// We have to check both where frame is as well as where _dts is.
//
    void R3dImage::limit_video_store(const int64_t frame)
    {

        if ( playback() == kForwards )
            return timed_limit_store( frame );

        SCOPED_LOCK( _mutex );

        int max_frames = max_image_frames();

        int64_t first, last;

        switch( playback() )
        {
        case kBackwards:
            first = frame - max_frames;
            last  = frame + max_frames;
            if ( _dts > last )   last  = _dts;
            if ( _dts < first )  first = _dts;
            break;
        case kForwards:
            first = frame - max_frames;
            last  = frame + max_frames;
            if ( _dts > last )   last  = _dts;
            if ( _dts < first )  first = _dts;
            break;
        default:
            first = frame - max_frames;
            last  = frame + max_frames;
            if ( _dts > last )   last = _dts;
            if ( _dts < first ) first = _dts;
            break;
        }

        if ( _images.empty() ) return;

        _images.erase( std::remove_if( _images.begin(), _images.end(),
                                       NotInRangeFunctor( first, last ) ),
                       _images.end() );


    }

}
