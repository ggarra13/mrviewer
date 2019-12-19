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

#include <tinyxml2.h>

#include "ImfFloatAttribute.h"
#include "ImfIntAttribute.h"
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
    using namespace tinyxml2;

    bool R3dImage::init = false;

    R3dImage::R3dImage() :
        CMedia(),
        clip( NULL ),
        iproc( NULL ),
        _pipeline( R3DSDK::Full_Graded ),
        _color_space( ImageColorREDWideGamutRGB ),
        _gamma_curve( ImageGammaLinear ),
        _hdr( false ),
        _old_scale( -1 ),
        _scale( 3 ),
        _Bias( 0.0f ),
        _Brightness( 0 ),
        _Contrast( 0 ),
        _ExposureAdjust( 0.f ),
        _ExposureCompensation( 0.f ),
        _GainBlue( 1.0f ),
        _GainGreen( 1.0f ),
        _GainRed( 1.0f ),
        _ISO( 320 ),
        _Kelvin( 5600.0f ),
        _Saturation( 1.0f ),
        _Shadow( 0.0f ),
        _Tint( 0.f ),
        _trackNo( 0 ),
        _hdr_mode( HDR_DO_BLEND )
    {
    }

    R3dImage::~R3dImage()
    {
    }

    bool R3dImage::test( const char* file )
    {
        if ( file == NULL || !init ) return false;

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


            if ((clip->VideoTrackCount() == 2U) ||
                (clip->MetadataItemAsInt(RMD_HDR_MODE) == 2))
            {
                _hdr = true;
            }


            iproc = new ImageProcessingSettings;
            clip->GetClipImageProcessingSettings( *iproc );

            _color_space = iproc->ColorSpace;
            _gamma_curve = iproc->GammaCurve;
            _Brightness = iproc->Brightness;
            _Contrast = iproc->Contrast;
            _ExposureAdjust = iproc->ExposureAdjust;
            _ExposureCompensation = iproc->ExposureCompensation;
            _ISO = iproc->ISO;
            _GainRed = iproc->GainRed;
            _GainGreen = iproc->GainGreen;
            _GainBlue = iproc->GainBlue;
            _Kelvin = iproc->Kelvin;
            _Saturation = iproc->Saturation;
            _Shadow   = iproc->Shadow;
            _Tint = iproc->Tint;

            set_ics_based_on_color_space_and_gamma();


            _attrs.insert( std::make_pair( 0, Attributes() ) );

            _fps = _play_fps = _orig_fps = clip->VideoAudioFramerate();
            _frameStart = _frame = _frame_start = 0;
            _frameEnd = _frame_end = clip->VideoFrameCount() - 1;

            video_info_t info;
            info.has_codec = true;
            info.codec_name = "RED 3D";
            info.fourcc = "R3D ";
            info.fps = _fps;
            info.pixel_format = "RGB";
            info.has_b_frames = true;
            info.start = 0;
            info.duration = (double)(_frame_end - _frame_start) /
                            (double) _fps;
            _video_info.push_back( info );

            if ( _hdr )
                _video_info.push_back( info );

            if ( clip->MetadataExists( RMD_FLIP_HORIZONTAL ) )
                _flipX = clip->MetadataItemAsInt( RMD_FLIP_HORIZONTAL );
            if ( clip->MetadataExists( RMD_FLIP_VERTICAL ) )
                _flipY = clip->MetadataItemAsInt( RMD_FLIP_VERTICAL );

            _pixel_ratio = 1.0f;
            _depth = image_type::kShort;
            if ( clip->MetadataExists( RMD_PIXEL_ASPECT_RATIO ) )
                _pixel_ratio = clip->MetadataItemAsFloat( RMD_PIXEL_ASPECT_RATIO );

            //
            // We need to extract some XML info from the RMD sidecar since
            // the SDK does not parse all of it.
            //
            const char* rmdfile = clip->GetRmdPath();
            if ( rmdfile )
            {
                tinyxml2::XMLDocument doc;
                doc.LoadFile( rmdfile );
                XMLNode* root = doc.FirstChildElement("sidecar");
                if ( root ) root = root->FirstChildElement("edits");
                if ( root ) root = root->FirstChildElement("editlist");
                if ( root ) root = root->FirstChildElement("edit");
                XMLElement* element = root->FirstChildElement( "in" );
                if ( element )
                {
                    int f;
                    element->QueryIntAttribute( "value", &f );
                    _frameStart = f;
                }
                element = root->FirstChildElement( "out" );
                if ( element )
                {
                    int f;
                    element->QueryIntAttribute( "value", &f );
                    _frameEnd = f;
                }
                root = root->FirstChildElement( "framing" );
                if ( root )
                {

                    element = root->FirstChildElement("flipvertical");
                    if (element)
                        element->QueryBoolAttribute( "value", &_flipY );


                    element = root->FirstChildElement("fliphorizontal");
                    if (element)
                        element->QueryBoolAttribute( "value", &_flipX );


                    element = root->FirstChildElement("rotation");
                    float z = 0.f;
                    if (element)
                        element->QueryFloatAttribute( "value", &z );
                    _rot_z = z;
                }
            }

            static const char* kIgnoreMetadata[] = {
                RMD_BRIGHTNESS,
                RMD_CONTRAST,
                RMD_DIGITAL_GAIN_RED,
                RMD_DIGITAL_GAIN_GREEN,
                RMD_DIGITAL_GAIN_BLUE,
                RMD_EXPOSURE_ADJUST,
                RMD_EXPOSURE_COMPENSATION,
                RMD_FLIP_HORIZONTAL,
                RMD_FLIP_VERTICAL,
                RMD_FLUT_CONTROL,
                RMD_HDR_MODE,
                RMD_ISO,
                RMD_PIXEL_ASPECT_RATIO,
                RMD_SATURATION,
                RMD_SHADOW,
                RMD_WHITE_BALANCE_KELVIN,
                RMD_WHITE_BALANCE_TINT
            };

            for ( size_t i = 0; i < clip->MetadataCount(); ++i )
            {
                const std::string& name = clip->MetadataItemKey(i);

                bool ignore = false;
                for ( size_t j = 0; j < sizeof(kIgnoreMetadata)/sizeof(char*);
                      ++j )
                {
                    if ( strcasecmp( name.c_str(), kIgnoreMetadata[j] ) == 0 )
                    {
                        ignore = true;
                        break;
                    }
                }

                if ( ignore ) continue;

                if ( clip->MetadataItemType(i) == MetadataTypeInt )
                {
                    Imf::IntAttribute attr( clip->MetadataItemAsInt(i) );
                    _attrs[0].insert( std::make_pair( name,
                                                      attr.copy() ) );
                }
                else if ( clip->MetadataItemType(i) == MetadataTypeFloat )
                {
                    Imf::FloatAttribute attr( clip->MetadataItemAsFloat(i) );
                    _attrs[0].insert( std::make_pair( name,
                                                      attr.copy() ) );
                }
                else // MetadataTypeString
                {
                    Imf::StringAttribute attr( clip->MetadataItemAsString(i) );
                    _attrs[0].insert( std::make_pair( name,
                                                      attr.copy() ) );
                }
            }
            image_damage( image_damage() | kDamageData );
        }
        return true;
    }

    bool R3dImage::finalize()
    {
        delete clip; clip = NULL;
        delete iproc; iproc = NULL;
        // We call finalizeSdk in mainWindow as initializeSdk
        // is not reentrant it seems.
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

    void R3dImage::scale( int i )
    {
        if ( i < 0 || i > 3 )
        {
            IMG_ERROR( _("Wrong index ") << i << _(" for scale" ) );
            return;
        }
        _scale = i;
        if ( stopped() )
        {
            _dts = _frame.load();
            refetch();
            find_image( _dts );
        }
    }

    void R3dImage::iso_index( size_t i )
    {
        if ( i >= ImageProcessingLimits::ISOCount ) {
            LOG_ERROR( _("ISO Index out of bounds") );
            return;
        }
        size_t newiso = ImageProcessingLimits::ISOList[i];
        if ( iproc->ISO != newiso )
        {
            _ISO = newiso;
            if ( stopped() )
            {
                _dts = _frame.load();
                refetch();
                find_image( _dts );
            }
        }
    }

    size_t R3dImage::iso_index() const
    {
        size_t iso320 = 0;
        for ( size_t i = 0; i < ImageProcessingLimits::ISOCount; ++i )
        {
            if ( iproc->ISO == ImageProcessingLimits::ISOList[i] )
                return i;
            if ( 320 == ImageProcessingLimits::ISOList[i] )
                iso320 = i;
        }
        return iso320;  // Default: 320
    }

    bool R3dImage::fetch( mrv::image_type_ptr& canvas,
                          const boost::int64_t frame )
    {
        if ( !clip || frame < _frame_start || frame > _frame_end )
        {
            return false;
        }

        bool new_grade = false;

        if ( iproc->ISO != _ISO )
        {
            new_grade = true;
        }
        if ( iproc->Saturation != _Saturation )
        {
            new_grade = true;
        }
        if ( iproc->Shadow != _Shadow )
        {
            new_grade = true;
        }
        if ( iproc->ExposureAdjust != _ExposureAdjust )
        {
            new_grade = true;
        }
        if ( iproc->ExposureCompensation != _ExposureCompensation )
        {
            new_grade = true;
        }
        if ( iproc->Brightness != _Brightness )
        {
            new_grade = true;
        }
        if ( iproc->Contrast != _Contrast )
        {
            new_grade = true;
        }
        if ( iproc->GainRed != _GainRed   ||
             iproc->GainGreen != _GainGreen ||
             iproc->GainBlue != _GainBlue )
        {
            new_grade = true;
        }
        if ( iproc->Kelvin != _Kelvin )
        {
            new_grade = true;
        }
        if ( iproc->Tint != _Tint )
        {
            new_grade = true;
        }
        if ( iproc->ColorSpace != _color_space )
        {
            new_grade = true;
        }
        if ( iproc->GammaCurve != _gamma_curve )
        {
            new_grade = true;
        }

        if ( _old_scale != _scale || new_grade )
        {
            _old_scale = _scale;
            iproc->ISO = _ISO;
            iproc->Saturation = _Saturation;
            iproc->Shadow = _Shadow;
            iproc->ExposureAdjust = _ExposureAdjust;
            iproc->ExposureCompensation = _ExposureCompensation;
            iproc->Brightness = _Brightness;
            iproc->Contrast = _Contrast;
            iproc->GainRed = _GainRed;
            iproc->GainGreen = _GainGreen;
            iproc->GainBlue = _GainBlue;
            iproc->Kelvin = _Kelvin;
            iproc->Tint = _Tint;
            iproc->ColorSpace = _color_space;
            iproc->GammaCurve = _gamma_curve;
            clear_cache();
            SCOPED_LOCK( _mutex );
            return fetch( canvas, frame );
        }

        // calculate how much ouput memory we're going to need

        // going to do a full resolution decode
        size_t dw = _real_width = clip->Width();
        size_t dh = _real_height = clip->Height();

        if ( is_thumbnail() ) _scale = 3; // for thumbnails we do 1/8 decode

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

#if 0

#define pixel_type kHalf
#define channels kRGB
#define decode_type PixelType_HalfFloat_RGB_Interleaved

#else

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

        // letting the decoder know how big the buffer is
        job.OutputBufferSize = dw * dh * 3U * sizeof(half);

        iproc->ImagePipelineMode = _pipeline;
        job.ImageProcessing = iproc;
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

        _gamma = 1.0f;

        if ( _hdr )
        {
            if ( clip->VideoTrackDecodeFrame( 0U, frame, job ) != DSDecodeOK )
            {
                IMG_ERROR( _("Video decode failed for HDR track 0, frame ")
                           << frame );
                return false;
            }


            image_type_ptr hdr;
            allocate_pixels( hdr, frame, 3, image_type::channels,
                             image_type::pixel_type );

            job.OutputBuffer = hdr->data().get();
            if ( clip->VideoTrackDecodeFrame( 1U, frame, job ) != DSDecodeOK )
            {
                IMG_ERROR( _("Video decode failed for HDR track 1, frame ")
                           << frame );
                return false;
            }

            float bias = 0.0f;
            switch( _hdr_mode )
            {
            case HDR_USE_TRACKNO:
                if ( _trackNo == 0 ) bias = -1.0;
                else bias = 1.0;
                break;
            case HDR_DO_BLEND:
                bias = _Bias;
                break;
            }

            float t = (bias + 1.0f) / 2.0f;
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
                IMG_ERROR( _("Video decode failed for frame ") << frame );
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
                cache( canvas );
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
        if ( frame < _frame_start || frame > _frame_end )
            return false;

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

        uint64_t max_frames = max_image_frames();

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

    bool R3dImage::has_changed()
    {
        if ( is_cache_filled( _frame ) )
        {
            _dts = _frame.load();
            refetch();
            find_image( _dts );
        }
        return true;
    }

    void R3dImage::do_seek() {
        // No need to set seek frame for right eye here
        if ( _right_eye )  _right_eye->do_seek();

        if ( saving() ) _seek_req = false;

        bool got_video = !has_video();
        bool got_audio = !has_audio();

        if ( !got_audio || !got_video )
        {
            if ( !saving() && _seek_frame != _expected )
                clear_packets();

            if ( !saving() || _seek_frame == _expected )
            {
                if ( ! is_cache_filled( _seek_frame ) )
                {
                    image_type_ptr canvas;
                    fetch( canvas, _seek_frame );
                }
                find_image( _seek_frame );
            }

        }


        // Seeking done, turn flag off
        _seek_req = false;

        if ( stopped() || saving() )
        {

            DecodeStatus status;
            if ( has_audio() )
            {
                int64_t f = _seek_frame;
                f += _audio_offset;
                status = decode_audio( f );
                if ( status > kDecodeOK )
                    IMG_ERROR( _("Decode audio error: ")
                               << decode_error( status )
                               << _(" for frame ") << _seek_frame );

                if ( !_audio_start )
                    find_audio( _seek_frame + _audio_offset );
                _audio_start = false;
            }

            // if ( has_video() || has_audio() )
            // {
            //     if ( !find_image( _seek_frame ) )
            //         IMG_ERROR( _("Decode video error seek frame " )
            //                    << _seek_frame );
            // }

            // Queue thumbnail for update
            image_damage( image_damage() | kDamageThumbnail );
        }

    }


    int R3dImage::color_version() const
    {
        return (int) clip->DefaultColorVersion();
    }

    void R3dImage::load_camera_settings()
    {
        Mutex& mtx = _video_packets.mutex();
        SCOPED_LOCK( mtx );
        SCOPED_LOCK( _mutex );
        clip->GetClipImageProcessingSettings( *iproc );
        if ( clip->DefaultColorVersion() >= 3 )
        {
            _Brightness = iproc->Brightness;
            _Contrast = iproc->Contrast;
            _ExposureAdjust = iproc->ExposureAdjust;
            _ExposureCompensation = iproc->ExposureCompensation;
            _GainRed = iproc->GainRed;
            _GainGreen = iproc->GainGreen;
            _GainBlue = iproc->GainBlue;
            _ISO = iproc->ISO;
            _Kelvin = iproc->Kelvin;
            _Saturation = iproc->Saturation;
            _Shadow   = iproc->Shadow;
            _Tint = iproc->Tint;
        }
        _color_space = iproc->ColorSpace;
        _gamma_curve = iproc->GammaCurve;
        _Flut = iproc->FLUT;
        set_ics_based_on_color_space_and_gamma();
        clear_cache();
        if ( stopped() )
        {
            _dts = _frame.load();
            refetch();
            find_image( _dts );
        }
    }

    void R3dImage::load_rmd_sidecar()
    {
        Mutex& mtx = _video_packets.mutex();
        SCOPED_LOCK( mtx );
        SCOPED_LOCK( _mutex );
        clip->GetDefaultImageProcessingSettings( *iproc );
        _color_space = iproc->ColorSpace;
        _gamma_curve = iproc->GammaCurve;
        _Brightness = iproc->Brightness;
        _Contrast = iproc->Contrast;
        _ExposureAdjust = iproc->ExposureAdjust;
        _ExposureCompensation = iproc->ExposureCompensation;
        _Flut = iproc->FLUT;
        _GainRed = iproc->GainRed;
        _GainGreen = iproc->GainGreen;
        _GainBlue = iproc->GainBlue;
        _ISO = iproc->ISO;
        _Kelvin = iproc->Kelvin;
        _Saturation = iproc->Saturation;
        _Shadow   = iproc->Shadow;
        _Tint = iproc->Tint;

        if ( is_hdr() )
        {
            HdrProcessingSettings settings;
            _hdr_mode = clip->GetRmdHdrProcessingSettings( settings, _trackNo );
            _Bias = settings.Bias;
            if ( _Bias != 0.0f ) _Bias = -_Bias;
        }

        set_ics_based_on_color_space_and_gamma();
        clear_cache();
        if ( stopped() )
        {
            _dts = _frame.load();
            refetch();
            find_image( _dts );
        }
    }

    struct ColorSpace
    {
        ImageColorSpace num;
        const char* const name;
    };

    ColorSpace kColorSpace[] = {
        { ImageColorREDWideGamutRGB, "REDWideGamutRGB" },
        { ImageColorRec2020, "Rec2020" },
        { ImageColorRec709, "Rec709" },
        { ImageColorSRGB, "sRGB" },
        { ImageColorAdobe1998, "Adobe 1998" },
        { ImageColorDCIP3, "DCIP3" },
        { ImageColorProPhotoRGB, "ProPhotoRGB" },
        { ImageColorDCIP3D65, "DCIP3 D65" },
        { ImageColorDRAGONcolor2, "DRAGON color2" },
        { ImageColorREDcolor4, "RED color4" },
        { ImageColorCameraRGB, "Camera RGB" },
        { ImageColorREDspace, "RED space" },
        { ImageColorREDcolor, "RED color" },
        { ImageColorREDcolor2, "RED color2" },
        { ImageColorREDcolor3, "RED color3" },
        { ImageColorDRAGONcolor, "DRAGON color" },
    };

    void R3dImage::color_space( unsigned idx )
    {
        Mutex& mtx = _video_packets.mutex();
        SCOPED_LOCK( mtx );
        SCOPED_LOCK( _mutex );
        if ( idx >= sizeof(kColorSpace)/sizeof(ColorSpace) )
        {
            IMG_ERROR( _("Invalid index ") << idx << _(" for color_space" ) );
            return;
        }
        _color_space = kColorSpace[idx].num;
        set_ics_based_on_color_space_and_gamma();
        clear_cache();
        if ( stopped() )
        {
            _dts = _frame.load();
            refetch();
            find_image( _dts );
        }
    }

    // This should be ImageProcessingLimits::ColorSpaceLabels, but it does
    // not work.
    std::string R3dImage::color_space() const
    {
        for ( size_t i = 0; i < sizeof(kColorSpace)/sizeof(ColorSpace); ++i )
        {
            if ( kColorSpace[i].num == _color_space )
                return kColorSpace[i].name;
        }
        return "unknown";
    }

    void R3dImage::color_spaces( stringArray& options ) const
    {
        options.clear();
        for ( size_t i = 0; i < sizeof(kColorSpace)/sizeof(ColorSpace); ++i )
        {
            options.push_back( kColorSpace[i].name );
        }
    }


    struct GammaCurve
    {
        ImageGammaCurve num;
        const char* const name;
    };

    GammaCurve kGammaCurve[] = {
        { ImageGammaLinear, "Linear" },
        { ImageGammaSRGB, "sRGB" },
        { ImageGammaHDR2084, "HDR2084" },
        { ImageGammaBT1886, "BT1886" },
        { ImageGammaLog3G12, "Log3G12" },
        { ImageGammaLog3G10, "Log3G10" },
        { ImageGammaREDlogFilm, "REDlogFilm" },
        { ImageGammaHybridLogGamma, "HybridLogGamma" },
        { ImageGamma2_2, "Gamma2.2" },
        { ImageGamma2_6, "Gamma2.6" },
        { ImageGammaRec709, "Rec709" },
        { ImageGammaREDgamma4, "REDgamma4" },
        { ImageGammaPDlog685, "PDlog685" },
        { ImageGammaPDlog985, "PDlog985" },
        { ImageGammaCustomPDlog, "Custom PD log" },
        { ImageGammaREDspace, "RED space"},
        { ImageGammaREDlog, "RED log"},
        { ImageGammaREDgamma, "RED gamma" },
        { ImageGammaREDgamma2, "RED gamma2" },
        { ImageGammaREDgamma3, "RED gamma3" },
    };

    void R3dImage::gamma_curve( unsigned idx )
    {
        Mutex& mtx = _video_packets.mutex();
        SCOPED_LOCK( mtx );
        SCOPED_LOCK( _mutex );
        if ( idx >= sizeof(kGammaCurve)/sizeof(GammaCurve) )
        {
            IMG_ERROR( _("Invalid index ") << idx << _(" for gamma_curve" ) );
            return;
        }
        _gamma_curve = kGammaCurve[idx].num;
        set_ics_based_on_color_space_and_gamma();
        clear_cache();
        if ( stopped() )
        {
            _dts = _frame.load();
            refetch();
            find_image( _dts );
        }
    }

    // This should be ImageProcessingLimits::GammaCurveLabels, but it does
    // not work.
    std::string R3dImage::gamma_curve() const
    {
        for ( size_t i = 0; i < sizeof(kGammaCurve)/sizeof(GammaCurve); ++i )
        {
            if ( kGammaCurve[i].num == _gamma_curve )
                return kGammaCurve[i].name;
        }
        return "unknown";
    }

    void R3dImage::gamma_curves( stringArray& options ) const
    {
        options.clear();
        for ( size_t i = 0; i < sizeof(kGammaCurve)/sizeof(GammaCurve); ++i )
        {
            options.push_back( kGammaCurve[i].name );
        }
    }

    void R3dImage::set_ics_based_on_color_space_and_gamma()
    {
        PreferencesUI* uiPrefs = ViewerUI::uiPrefs;
        const char* var = uiPrefs->uiPrefsOCIOConfig->value();
        if ( !var ) return;

        std::string ocio = var;
        if ( ocio.rfind( "nuke-default" ) != std::string::npos )
        {
            if ( _gamma_curve == ImageGammaLog3G12 )
                ocio_input_color_space( "Log3G12" );
            else if ( _gamma_curve == ImageGammaBT1886 )
                ocio_input_color_space( "BT1886" );
            else if ( _gamma_curve == ImageGammaHDR2084 )
                ocio_input_color_space( "st2084" );
            else if ( _gamma_curve == ImageGammaREDlogFilm )
                ocio_input_color_space( "compositing_log" );
            else if ( _gamma_curve == ImageGammaLinear )
                ocio_input_color_space( "scene_linear" );
            else if ( _gamma_curve == ImageGammaHybridLogGamma )
                ocio_input_color_space( "HybridLogGamma" );
            else if ( _gamma_curve == ImageGamma2_2 )
                ocio_input_color_space( "Gamma2.2" );
            else if ( _gamma_curve == ImageGamma2_6 )
                ocio_input_color_space( "Gamma2.6" );
            else if ( _gamma_curve == ImageGammaREDlog )
                ocio_input_color_space( "REDlog" );
            else if ( _gamma_curve == ImageGammaREDgamma  ||
                      _gamma_curve == ImageGammaREDgamma2 ||
                      _gamma_curve == ImageGammaREDgamma3 ||
                      _gamma_curve == ImageGammaREDgamma4 )
                ocio_input_color_space( "rec709" );
            else
            {
                if ( _color_space == ImageColorREDWideGamutRGB )
                    ocio_input_color_space( "Log3G10" );
                else
                    ocio_input_color_space( "rec709" );
            }
        }
        else
        {
            if ( _color_space == ImageColorREDWideGamutRGB )
            {
                ocio_input_color_space( "Input - RED - REDLog3G10 - REDWideGamutRGB" );
                _gamma_curve = ImageGammaLog3G10;
            }
            else
            {
                ocio_input_color_space( "Output - Rec.709" );
            }
        }
        mrv::Preferences::uiMain->uiICS->copy_label( ocio_input_color_space().c_str() );

    }
}
