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
#include <libavutil/channel_layout.h>
#include <libavutil/intreadwrite.h>
#include <libavcodec/avcodec.h>
}

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <tinyxml2.h>

#include "ImfFloatAttribute.h"
#include "ImfIntAttribute.h"
#include "ImfStringAttribute.h"

#include "core/mrvMath.h"
#include "core/mrvFrameFunctors.h"
#include "gui/mrvPreferences.h"
#include "mrvPreferencesUI.h"
#include "R3DSDK.h"


using namespace R3DSDK;

namespace {
    const char* kModule = "r3d";
}


namespace mrv {

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#  define AVCODEC_MAX_AUDIO_FRAME_SIZE 198000
#endif

#define kALIGNMENT 512U

    //! Byte swap int
    inline int32_t swap_int32( int32_t val )
    {
       val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );
       return (val << 16) | ((val >> 16) & 0xFFFF);
    }

// The R3D SDK requires that the audio output buffer is 512-byte aligned
    unsigned char * AlignedMalloc(size_t & sizeNeeded)
    {
  // alloc 511 bytes more to make sure we can align the buffer in case it isn't
  unsigned char * buffer = (unsigned char *)av_malloc(sizeNeeded +
                  kALIGNMENT - 1);

  if (!buffer)
      return NULL;

  sizeNeeded = 0U;

  // cast to a 32-bit or 64-bit (depending on platform) integer so we can do the math
  uintptr_t ptr = (uintptr_t)buffer;

  // check if it's already aligned, if it is we're done
  if ((ptr % kALIGNMENT) == 0U)
      return buffer;

  // calculate how many bytes we need
  sizeNeeded = kALIGNMENT - (ptr % kALIGNMENT);

  return buffer + sizeNeeded;
    }

    using namespace tinyxml2;

    bool R3dImage::init = false;

    const size_t bufferSize = 1 * 1024 * 1024;

    R3dImage::R3dImage() :
  CMedia(),
  clip( NULL ),
  iproc( NULL ),
  _pipeline( R3DSDK::Full_Graded ),
  _color_space( ImageColorREDWideGamutRGB ),
  _gamma_curve( ImageGammaLinear ),
  _hdr( false ),
  _old_scale( -1 ),
  _scale( Preferences::R3dScale ),
  _color_version( 2 ),
  _Bias( 0.0f ),
  _Brightness( 0 ),
  _Contrast( 0 ),
  _Denoise( R3DSDK::ImageDenoiseOff ),
  _Detail( R3DSDK::ImageDetailHigh ),
  _ExposureAdjust( 0.f ),
  _ExposureCompensation( 0.f ),
  _Flut( 0.f ),
  _GainBlue( 1.0f ),
  _GainGreen( 1.0f ),
  _GainRed( 1.0f ),
  _ISO( 320 ),
  _Kelvin( 5600.0f ),
  _old_Bias( -2.0f ),
  _old_trackNo( -1 ),
  _old_hdr_mode( HDR_USE_TRACKNO ),
  _Saturation( 1.0f ),
  _Shadow( 0.0f ),
  _Sharpness( R3DSDK::ImageOLPFCompOff ),
  _Tint( 0.f ),
  _trackNo( 0 ),
  _hdr_mode( HDR_DO_BLEND ),
  adjusted( 0 ),
  audiobuffer( NULL ),
  audioinit( false )
    {
  _gamma = 1.0f;
  _audio_last_frame = 0;
    }

    R3dImage::~R3dImage()
    {
  finalize();
    }

    bool R3dImage::test( const char* file )
    {
        if ( file == NULL || !init ) return false;

        // Ignore .RMD sidedata files
        if ( strcasecmp( file+strlen(file)-4, ".rmd" ) == 0 )
            return false;

        // Ignore .RMD sidedata files
        if ( strcasecmp( file+strlen(file)-5, ".otio" ) == 0 )
            return false;

        if ( strcasecmp( file+strlen(file)-8, ".session" ) == 0 )
            return false;

        Clip* c = new Clip( file );

        if ( !c ) {
            return false;
        }
        c->Close();
        delete c;
        return true;
    }

    bool R3dImage::initialize()
    {
        delete clip; clip = NULL;
        delete iproc; iproc = NULL;

        // load the clip
        clip = new Clip( filename() );

        // let the user know if this failed
        if (clip->Status() != LSClipLoaded)
        {
            IMG_ERROR( _("Error loading file ") << filename() );
            finalize();
            return false;
        }

        _hdr = false;
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
        _Denoise = iproc->Denoise;
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

        _color_version = clip->DefaultColorVersion();

        set_ics_based_on_color_space_and_gamma();

        _attrs.clear();
        _attrs.insert( std::make_pair( 0, Attributes() ) );

        _fps = _play_fps = _orig_fps = clip->VideoAudioFramerate();
        _frameStart = _frame = _frame_start = 0;
        _frameEnd = _frame_end = clip->VideoFrameCount() - 1;

        _video_info.clear();

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

        _depth = image_type::kShort;
        if ( clip->MetadataExists( RMD_PIXEL_ASPECT_RATIO ) )
            pixel_ratio( 0, clip->MetadataItemAsFloat( RMD_PIXEL_ASPECT_RATIO ) );

        //
        // We need to extract some XML info from the RMD sidecar since
        // the SDK does not parse all of it.
        //
        const char* rmdfile = clip->GetRmdPath();
        if ( rmdfile && fs::exists(rmdfile) )
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


        // Parse audio
        maxAudioBlockSize = 0U;
        size_t blocks = clip->AudioBlockCountAndSize(&maxAudioBlockSize);

        if (blocks == 0U) return true;

        frequency = clip->MetadataItemAsInt(RMD_SAMPLERATE);
        unsigned int samplesize = clip->MetadataItemAsInt(RMD_SAMPLE_SIZE);


        _audio_channels = (unsigned short)clip->AudioChannelCount();
        _total_samples = clip->AudioSampleCount();

        audio_info_t s;
        s.has_codec = true;
        s.codec_name = "R3D Audio";
        s.fourcc = "R3DA";
        s.channels = _audio_channels;
        s.frequency = frequency;
        s.bitrate = frequency * _audio_channels * samplesize;
        s.format = "s32be";
        s.language = _("und");
        s.start = 0;
        s.duration = _total_samples / (double) frequency;

        _audio_info.push_back( s );
        _audio_index = _audio_info.size() - 1;

        if ( !_audio_buf )
        {
            _audio_max = AVCODEC_MAX_AUDIO_FRAME_SIZE * 6;
            _audio_buf = new aligned16_uint8_t[ _audio_max ];
            memset( _audio_buf, 0, _audio_max );

            adjusted = bufferSize;

            // alloc this memory 512-byte aligned
            audiobuffer = AlignedMalloc(adjusted);
            if (audiobuffer == NULL)
            {
                IMG_ERROR( _("Out of memory for audio buffer") );
                return false;
            }
        }

        if ( ! _is_thumbnail )
        {
            char buf[256];
            uint64_t in_ch_layout = get_valid_channel_layout( 0,
                                                              _audio_channels );
            if ( in_ch_layout == 0 )
                in_ch_layout = get_valid_channel_layout( AV_CH_LAYOUT_STEREO,
                                                         _audio_channels );

            if ( in_ch_layout == 0 )
                in_ch_layout = get_valid_channel_layout( AV_CH_LAYOUT_MONO,
                                                         _audio_channels );

            av_get_channel_layout_string( buf, 256, _audio_channels,
                                          in_ch_layout );
            IMG_INFO( _("Audio ") << buf << _(", channels ")
                      << _audio_channels );
            IMG_INFO( _("format s32be, frequency ") << frequency );

            audio_initialize();
            _audio_format = AudioEngine::kS32LSB;

        }

        return true;
    }

    bool R3dImage::finalize()
    {
        if (init)
        {
            delete iproc; iproc = NULL;
            if ( clip ) clip->Close();
            delete clip; clip = NULL;
        }

        av_free(((uint8_t*)audiobuffer - adjusted));
        audiobuffer = NULL;
        adjusted = 0;

        // We call initializeSdk/finalizeSdk in mainWindow as initializeSdk
        // is not reentrant it seems.
        return true;
    }


    void R3dImage::clear_cache()
    {
        SCOPED_LOCK( _mutex );
        _images.clear();
        image_damage( image_damage() | kDamageCache | kDamageContents );
    }

    CMedia::Cache R3dImage::is_cache_filled( int64_t frame )
    {
        SCOPED_LOCK( _mutex );
        bool ok = false;

        // Check if video is already in video store
        video_cache_t::iterator i = std::find_if( _images.begin(),
                                                  _images.end(),
                                                  EqualFunctor(frame) );
        if ( i != _images.end() ) ok = true;

        if ( ok && _stereo_input != kSeparateLayersInput ) return kStereoCache;
        return (CMedia::Cache) ok;
    }

    void R3dImage::scale( int i )
    {
        SCOPED_LOCK( _mutex );
        _scale = short(i);
        clear_cache();
        if ( stopped() )
        {
            if ( refetch( _frame ) )
            {
                int64_t f = _frame;
                find_image( f );
            }
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
            clear_cache();
            if ( stopped() )
            {
                copy_values();
                if ( refetch( _frame ) )
                {
                    int64_t f = _frame;
                    find_image( f );
                }
            }
        }
    }

    size_t R3dImage::iso_index() const
    {
        size_t iso320 = 0;
        for ( size_t i = 0; i < ImageProcessingLimits::ISOCount; ++i )
        {
            if ( _ISO == ImageProcessingLimits::ISOList[i] )
                return i;
            if ( 320 == ImageProcessingLimits::ISOList[i] )
                iso320 = i;
        }
        return iso320;  // Default: 320
    }

    void R3dImage::copy_values()
    {
        SCOPED_LOCK( _mutex );
        _old_scale = _scale;
        iproc->ISO = _ISO;
        _old_Bias = _Bias;
        iproc->Brightness = _Brightness;
        iproc->Contrast = _Contrast;
        iproc->ColorSpace = _color_space;
        iproc->Denoise = _Denoise;
        iproc->Detail = _Detail;
        iproc->ExposureAdjust = _ExposureAdjust;
        iproc->ExposureCompensation = _ExposureCompensation;
        iproc->FLUT = _Flut;
        iproc->GainRed = _GainRed;
        iproc->GainGreen = _GainGreen;
        iproc->GainBlue = _GainBlue;
        iproc->GammaCurve = _gamma_curve;
        _old_hdr_mode = _hdr_mode;
        iproc->ImagePipelineMode = _pipeline;
        iproc->Kelvin = _Kelvin;
        iproc->Saturation = _Saturation;
        iproc->Shadow = _Shadow;
        iproc->OLPFCompensation = _Sharpness;
        iproc->Tint = _Tint;
        _old_trackNo = _trackNo;
    }


    bool R3dImage::fetch( mrv::image_type_ptr& canvas,
                          const boost::int64_t frame )
    {
        // create and fill out a decode job structure so the
        // decoder knows what you want it to do
        VideoDecodeJob job;
        // going to do a full resolution decode
        size_t dw, dh;

        SCOPED_LOCK( _load_mutex );

        if ( !clip || !iproc || frame < _frame_start || frame > _frame_end )
        {
            return false;
        }

        if ( _old_scale != _scale ||
             iproc->ISO                  != _ISO ||
             iproc->ColorSpace           != _color_space ||
             iproc->GammaCurve           != _gamma_curve ||
             (!is_equal( _old_Bias, _Bias             ) ) ||
             (!is_equal( iproc->Kelvin, _Kelvin )) ||
             (iproc->Denoise != _Denoise ) ||
             (iproc->Detail  != _Detail  ) ||
             (iproc->ImagePipelineMode  != _pipeline  ) ||
             (!is_equal( iproc->ExposureAdjust, _ExposureAdjust ) ) ||
             (!is_equal( iproc->ExposureCompensation,
                         _ExposureCompensation ) ) ||
             (!is_equal( iproc->Brightness, _Brightness ) ) ||
             (!is_equal( iproc->Contrast, _Contrast   ) ) ||
             (!is_equal( iproc->FLUT, _Flut           ) ) ||
             (!is_equal( iproc->GainRed, _GainRed     ) ) ||
             (!is_equal( iproc->GainGreen, _GainGreen ) ) ||
             (!is_equal( iproc->GainBlue, _GainBlue   ) ) ||
             (!is_equal( iproc->Saturation, _Saturation ) ) ||
             (!is_equal( iproc->Shadow, _Shadow ) ) ||
             (iproc->OLPFCompensation != _Sharpness ) ||
             (!is_equal( iproc->Tint, _Tint )) ||
             _old_trackNo                != _trackNo ||
             _old_hdr_mode               != _hdr_mode )
        {
            clear_cache();
            copy_values();
        }

        // calculate how much ouput memory we're going to need

        // going to do a full resolution decode
        dw = _real_width = clip->Width();
        dh = _real_height = clip->Height();

        if ( is_thumbnail() ) _scale = 4; // for thumbnails we do 1/8 decode

        if ( _scale > 0 )
        {
            dw /= unsigned( 1 << _scale );
            dh /= unsigned( 1 << _scale );
        }

        if ( _layers.empty() )
        {
            _num_channels = 0;
            rgb_layers();
            lumma_layers();
        }


        image_size( dw, dh );
        allocate_pixels( canvas, frame, 3, image_type::kRGB,
                         image_type::kShort, dw, dh );

        // alloc this memory 16-byte aligned
        unsigned char * imgbuffer = (unsigned char*)canvas->data().get();




        // calculate the bytes per row, for an interleaved
        // 16-bit image this is the width times 6 ( 2 * 3 )
        job.BytesPerRow = dw * sizeof(short) * 3U;

        // letting the decoder know how big the buffer is
        job.OutputBufferSize = dw * dh * 3U * sizeof(short);

        job.ImageProcessing = iproc;
        job.HdrProcessing = NULL;

        //  std::cerr << frame << " scale 1:" << ( 1 << _scale ) << std::endl;

        switch( _scale )
        {
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
        case 4:
        default:
            // decode at eight resolution at good quality
            job.Mode = DECODE_SIXTEENTH_RES_GOOD;
            break;
        }

        // store the image here
        job.OutputBuffer = imgbuffer;

        // store the image in a 16-bit interleaved RGB
        job.PixelType = PixelType_16Bit_RGB_Interleaved;




        if ( is_hdr() )
        {
            if ( clip->VideoTrackDecodeFrame( 0U, (size_t)frame, job ) !=
                 DSDecodeOK )
            {
                IMG_ERROR( _("Video decode failed for HDR track 0, frame ")
                           << frame );
                return initialize();
            }


            image_type_ptr hdr;
            allocate_pixels( hdr, frame, 3, image_type::kRGB,
                             image_type::kShort, dw, dh );

            job.OutputBuffer = hdr->data().get();
            if ( clip->VideoTrackDecodeFrame( 1U, (size_t)frame, job ) !=
                 DSDecodeOK )
            {
                IMG_ERROR( _("Video decode failed for HDR track 1, frame ")
                           << frame );
                return initialize();
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
            // decode the frame 'frame' of the clip
            if (clip->DecodeVideoFrame((size_t)frame, job) != DSDecodeOK)
            {
                IMG_ERROR( _("Video decode failed for frame ") << frame );
                return initialize();
            }
        }

        // Store in queue
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

    unsigned int R3dImage::audio_bytes_per_frame()
    {
        unsigned int ret = 0;
        if ( !has_audio() ) return ret;

        int channels = _audio_channels;
        if (_audio_engine->channels() > 0 && _audio_channels > 0 ) {
            channels = FFMIN(_audio_engine->channels(),
                             (unsigned)_audio_channels);
        }
        if ( channels <= 0 || _audio_format == AudioEngine::kNoAudioFormat)
            return ret;

        SCOPED_LOCK( _audio_mutex );

        AVSampleFormat ft = AudioEngine::ffmpeg_format( _audio_format );
        unsigned bps = av_get_bytes_per_sample( ft );

        if ( _orig_fps <= 0.0f ) _orig_fps = _fps.load();
        ret = (unsigned int)( (double) frequency / _orig_fps ) * channels * bps;
        return ret;
    }

    bool R3dImage::frame( const int64_t f )
    {
        if ( memory_used >= Preferences::max_memory  )
        {
            int64_t max_frames = (int64_t) max_image_frames();
            if ( std::abs( f - _frame ) >= max_frames )
                return false;
            limit_video_store( _frame );
            if ( has_audio() ) limit_audio_store( _frame );
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

        AVPacket* pkt = av_packet_alloc();
        pkt->dts = pkt->pts = _dts;
        pkt->size = 0;
        pkt->data = NULL;


        if ( ! is_cache_filled( _dts ) )
        {
            image_type_ptr canvas;
            if ( fetch( canvas, _dts ) )
            {
                default_color_corrections();
            }
        }

        _video_packets.push_back( *pkt );

        if ( has_audio() )
        {
            _audio_packets.push_back( *pkt );
        }


        _expected = _dts + 1;
        _expected_audio = _expected + _audio_offset;
        return true;
    }

    bool R3dImage::find_image( const int64_t frame )
    {

        if ( _right_eye && (stopped() || saving() ) )
            _right_eye->find_image( frame );

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
                        _frame = _hires->frame();
                        // ++counter;
                        // IMG_WARNING( _("find_image: frame ") << frame
                        //              << _(" not found, choosing ")
                        //              << _hires->frame()
                        //              << _(" instead") );
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
                    return timercmp( a, b, < );
                }
        };

        typedef std::map< timeval, int64_t, customMore > TimedSeqMap;
        TimedSeqMap tmp;
        {
            SCOPED_LOCK( _mutex );

            {
                video_cache_t::iterator  it = _images.begin();
                video_cache_t::iterator end = _images.end();
                for ( ; it != end; ++it )
                {
                    tmp.insert( std::make_pair( (*it)->ptime(),
                                                (*it)->frame() ) );
                }
            }

            //
            // Limit the video store to fewer than max frames images.
            //
            TimedSeqMap::iterator it = tmp.begin();
            for ( ; it != tmp.end() &&
                      memory_used >= Preferences::max_memory; ++it )
            {
                if ( _images.size() < max_frames ) break;
                auto start = std::remove_if( _images.begin(), _images.end(),
                                             EqualFunctor( it->second ) );
                _images.erase( start, _images.end() );
            }

        } // End of SCOPED_LOCK( _mutex )
    }

    void R3dImage::limit_video_store(const int64_t frame)
    {

        return timed_limit_store( frame );

    }

    bool R3dImage::has_changed()
    {
        if ( is_cache_filled( _frame ) )
        {
            copy_values();
            if ( refetch( _frame ) )
            {
                int64_t f = _frame;
                find_image( f );
            }
        }
        image_damage( image_damage() | kDamageCache | kDamageContents );
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

            if ( ! is_cache_filled( _seek_frame ) )
            {
                image_type_ptr canvas;
                if ( fetch( canvas, _seek_frame ) )
                {
                    default_color_corrections();
                }
            }

            find_image( _seek_frame );

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
                               << get_error_text( status )
                               << _(" for frame ") << _seek_frame );

                if ( !_audio_start )
                    find_audio( _seek_frame + _audio_offset );
                _audio_start = false;
            }


            // Queue thumbnail for update
            image_damage( image_damage() | kDamageThumbnail );
        }

    }

    CMedia::DecodeStatus R3dImage::decode_audio( int64_t& f )
    {

        if ( ! _audio_packets.empty() )
        {
            if ( _audio_packets.is_loop_end() )
            {
                _audio_packets.pop_front();
                return kDecodeLoopEnd;
            }
            else if ( _audio_packets.is_loop_start() )
            {
                _audio_packets.pop_front();
                return kDecodeLoopStart;
            }

            _audio_packets.pop_front();
        }

        int64_t frame = f;


        bool ok = in_audio_store( frame );
        if ( ok ) return kDecodeOK;

        unsigned int bytes_per_frame = audio_bytes_per_frame();

        size_t samplesInBuffer = bufferSize / ( _audio_channels * 4U );

        unsigned long long startSample = frame *
                                         ( (double) frequency / _orig_fps );

        clip->DecodeAudio( startSample, &samplesInBuffer, audiobuffer,
                           bufferSize );


        unsigned j = 0;

        if ( samplesInBuffer > 0 )
        {
            int32_t* tmp = (int32_t*) ((int8_t*)_audio_buf + _audio_buf_used);

            size_t bytesInBuffer = samplesInBuffer * _audio_channels *
                                   sizeof(int32_t);

            for ( size_t i = 0; i < bytesInBuffer; i += 4, ++tmp )
            {
                int32_t base = *((int32_t*) ((int8_t*)audiobuffer + i) );
                *tmp = swap_int32( base );
            }

            _audio_buf_used += bytesInBuffer;
        }


        CMedia::DecodeStatus got_audio = kDecodeMissingFrame;
        int64_t last = frame;
        uint32_t index = 0;

        if ( last == first_frame() || (stopped() /* || saving() */ ) )
        {
            if ( bytes_per_frame > _audio_buf_used && _audio_buf_used > 0 )
            {
                bytes_per_frame = _audio_buf_used;
            }
        }


        // Split audio read into frame chunks
        for (;;)
        {

            if ( bytes_per_frame > _audio_buf_used ) break;

#ifdef DEBUG
            if ( index + bytes_per_frame >= _audio_max )
            {
                std::cerr << "frame: " << frame << std::endl
                          << "index: " << index << std::endl
                          << "  bpf: " << bytes_per_frame << std::endl
                          << " used: " << _audio_buf_used << std::endl
                          << "  max: " << _audio_max << std::endl;
            }
#endif

            uint32_t skip = store_audio( last,
                                         (uint8_t*)_audio_buf + index,
                                         bytes_per_frame );
            if ( skip == 0 ) break;

            index += skip;


            if ( last >= frame ) got_audio = kDecodeOK;

            assert( bytes_per_frame <= _audio_buf_used );
            _audio_buf_used -= bytes_per_frame;
            ++last;

        }


        if ( _audio_buf_used > 0 && index > 0 )
        {
            //
            // NOTE: audio buffer must remain 16 bits aligned for ffmpeg.
            memmove( _audio_buf, _audio_buf + index, _audio_buf_used );
        }

        return got_audio;
    }

    int R3dImage::color_version() const
    {
        if ( !clip ) return 0;
        return _color_version;
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
        _Denoise = iproc->Denoise;
        _Detail = iproc->Detail;
        _Flut = iproc->FLUT;
        _Sharpness = iproc->OLPFCompensation;
        set_ics_based_on_color_space_and_gamma();
        clear_cache();
        if ( stopped() )
        {
            copy_values();
            if ( refetch( _frame ) )
            {
                int64_t f = _frame;
                find_image( f );
            }
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
        _Denoise = iproc->Denoise;
        _Detail = iproc->Detail;
        _ExposureAdjust = iproc->ExposureAdjust;
        _ExposureCompensation = iproc->ExposureCompensation;
        _Flut = iproc->FLUT;
        _GainRed = iproc->GainRed;
        _GainGreen = iproc->GainGreen;
        _GainBlue = iproc->GainBlue;
        _ISO = iproc->ISO;
        _Kelvin = iproc->Kelvin;
        _Saturation = iproc->Saturation;
        _Sharpness = iproc->OLPFCompensation;
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
            copy_values();
            if ( refetch( _frame ) )
            {
                int64_t f = _frame;
                find_image( f );
            }
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
            copy_values();
            if ( refetch( _frame ) )
            {
                int64_t f = _frame;
                find_image( f );
            }
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
            copy_values();
            if ( refetch( _frame ) )
            {
                int64_t f = _frame;
                find_image( f );
            }
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
            if ( _color_space == ImageColorREDWideGamutRGB )
            {
                ocio_input_color_space( "Log3G10" );
                _gamma_curve = ImageGammaLog3G10;
            }
            else
            {
                if ( _gamma_curve == ImageGammaLog3G12 )
                    ocio_input_color_space( "Log3G12" );
                else if ( _gamma_curve == ImageGammaLog3G10 )
                    ocio_input_color_space( "Log3G10" );
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
                    ocio_input_color_space( "rec709" );
                }
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

    void R3dImage::audio_stream( int idx )
    {
        CMedia::audio_stream( idx );
        if ( idx < 0 ) return;

        _audio_channels = (unsigned short) clip->AudioChannelCount();
        _audio_format = AudioEngine::kS32LSB;
    }
}
