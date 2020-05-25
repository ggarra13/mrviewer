#include "brawImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if !defined(WIN32) && !defined(WIN64)
#  include <arpa/inet.h>
#else
#  include <winsock2.h>    // for htonl
#  include <comutil.h>
#endif

#ifdef OSX
#include <CoreServices/CoreServices.h>
#define STDMETHODCALLTYPE
#endif


#ifdef DEBUG
#include <cassert>
#define VERIFY(condition) assert(SUCCEEDED(condition))
#else
#define VERIFY(condition) condition
#endif

extern "C" {
#include <libavutil/time.h>
#ifdef WIN32
#  pragma warning(push)
#  pragma warning (disable: 4244 )
#endif
#include <libavutil/intreadwrite.h>
#ifdef WIN32
#  pragma warning(pop)
#endif
}

#include "ImfFloatAttribute.h"
#include "ImfIntAttribute.h"
#include "ImfStringAttribute.h"
#include "ImfTimeCodeAttribute.h"

#include "core/mrvMath.h"
#include "core/mrvFrameFunctors.h"
#include "gui/mrvPreferences.h"
#include "mrvPreferencesUI.h"



namespace {
    const char* kModule = "braw";
}


namespace mrv {


#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#  define AVCODEC_MAX_AUDIO_FRAME_SIZE 198000
#endif

    static const BlackmagicRawResourceFormat s_resourceFormat = blackmagicRawResourceFormatRGBAU8;

    class CameraCodecCallback : public IBlackmagicRawCallback
    {
        brawImage* img;
        mrv::image_type_ptr& canvas;
        int64_t frame;
        BlackmagicRawResolutionScale scale;
        IBlackmagicRawFrame* m_frame = nullptr;
    public:
        explicit CameraCodecCallback( brawImage* b,
                                      mrv::image_type_ptr& c,
                                      const int64_t f,
                                      const BlackmagicRawResolutionScale s =
                                      blackmagicRawResolutionScaleEighth) :
            IBlackmagicRawCallback(),
            img( b ),
            canvas( c ),
            frame( f ),
            scale( s )
            {
            };
        virtual ~CameraCodecCallback()
            {
                if (m_frame != nullptr)
                    m_frame->Release();
            }

        IBlackmagicRawFrame*	GetFrame() {
            return m_frame;
        }

        virtual
        void STDMETHODCALLTYPE
        ReadComplete(IBlackmagicRawJob* readJob, HRESULT result,
                     IBlackmagicRawFrame* Frame)
            {
                if (result == S_OK)
                {
                    m_frame = Frame;
                    m_frame->AddRef();
                }

                IBlackmagicRawJob* decodeAndProcessJob = nullptr;

                if (result == S_OK)
                    result = Frame->SetResourceFormat(s_resourceFormat);

                if (result == S_OK)
                    result = Frame->SetResolutionScale( scale );

                if (result == S_OK)
                    result = Frame->CreateJobDecodeAndProcessFrame(nullptr,
                                                                   nullptr,
                                                                   &decodeAndProcessJob);

                if (result == S_OK)
                    result = decodeAndProcessJob->Submit();

                if (result != S_OK)
                {
                    if (decodeAndProcessJob)
                        decodeAndProcessJob->Release();
                }

                readJob->Release();
            }

        virtual
        void STDMETHODCALLTYPE
        ProcessComplete(IBlackmagicRawJob* job, HRESULT result,
                        IBlackmagicRawProcessedImage* processedImage)
            {
                unsigned int width = 0;
                unsigned int height = 0;
                void* imageData = nullptr;

                if (result == S_OK)
                    result = processedImage->GetWidth(&width);

                if (result == S_OK)
                    result = processedImage->GetHeight(&height);

                if (result == S_OK)
                    result = processedImage->GetResource(&imageData);

                if ( result == S_OK )
                    img->store_image( canvas, frame, width, height, imageData );

                job->Release();
            }

        virtual void STDMETHODCALLTYPE DecodeComplete(IBlackmagicRawJob*, HRESULT) {}
        virtual void STDMETHODCALLTYPE TrimProgress(IBlackmagicRawJob*, float) {}
        virtual void STDMETHODCALLTYPE TrimComplete(IBlackmagicRawJob*, HRESULT) {}

#ifdef WIN32
        virtual void STDMETHODCALLTYPE SidecarMetadataParseWarning(IBlackmagicRawClip*, BSTR, uint32_t, BSTR) {}
        virtual void STDMETHODCALLTYPE SidecarMetadataParseError(IBlackmagicRawClip*, BSTR, uint32_t, BSTR) {}
#elif LINUX
        virtual void SidecarMetadataParseWarning(IBlackmagicRawClip*, const char*, uint32_t, const char*) {}
        virtual void SidecarMetadataParseError(IBlackmagicRawClip*, const char*, uint32_t, const char*) {}
#elif OSX
        virtual void SidecarMetadataParseWarning(IBlackmagicRawClip*, CFStringRef, uint32_t, CFStringRef) {}
        virtual void SidecarMetadataParseError(IBlackmagicRawClip*, CFStringRef, uint32_t, CFStringRef) {}
#endif
        virtual void STDMETHODCALLTYPE PreparePipelineComplete(void*, HRESULT) {}

        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID*)
            {
                return E_NOTIMPL;
            }

        virtual ULONG STDMETHODCALLTYPE AddRef(void)
            {
                return 0;
            }

        virtual ULONG STDMETHODCALLTYPE Release(void)
            {
                return 0;
            }
    };

    bool brawImage::init = true;
    IBlackmagicRawFactory* brawImage::factory = nullptr;

    brawImage::brawImage() :
        CMedia(),
        _scale( Preferences::BRAWScale ),
        _old_scale( Preferences::BRAWScale ),
        bitDepth( 0 ),
        audiobuffer( NULL ),
        audioinit( false ),
        codec( NULL ),
        clip( NULL ),
        readJob( NULL ),
        audio( NULL )
    {
        initialize();
        _gamma = 1.0f;
        _audio_last_frame = 0;
    }

    brawImage::~brawImage()
    {
        finalize();
    }

    bool brawImage::store_image( mrv::image_type_ptr& canvas,
                                 int64_t frame, unsigned dw, unsigned dh,
                                 void* imageData )
    {
        image_size( dw, dh );
        const image_type::PixelType pixel_type = image_type::kByte;
        allocate_pixels( canvas, frame, 4, image_type::kRGBA, pixel_type,
                         dw, dh );

        memcpy( canvas->data().get(), imageData, dw*dh*4 );

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

    bool brawImage::test( const char* file )
    {
        if ( file == NULL ) return false;

        HRESULT result;

        std::string libpath = mrv::Preferences::root;
        libpath += "/lib";

#ifdef WIN32
        _bstr_t clib( libpath.c_str() );
#elif LINUX
        const char* clib = libpath.c_str();
#elif OSX
        CFStringRef clib = CFStringCreateWithCString( NULL,
                                                      libpath.c_str(),
                                                      kCFStringEncodingUTF8 );
#endif

        if ( factory == nullptr )
        {
            factory = CreateBlackmagicRawFactoryInstanceFromPath( clib );
            if (factory == nullptr)
            {
#if defined(LINUX) || defined(_WIN64) || defined(OSX)
                LOG_ERROR( _("Failed to create IBlackmagicRawFactory!") );
#endif
                return false;
            }
        }

#ifdef OSX
        CFRelease( clib );
#endif


        IBlackmagicRaw* codec;
        result = factory->CreateCodec(&codec);
        if (result != S_OK)
        {
            LOG_ERROR( _("Failed to create IBlackmagicRaw!") );
            return false;
        }

#ifdef WIN32
        _bstr_t filename( file );
#elif LINUX
        const char* filename = file;
#elif OSX
        CFStringRef filename = CFStringCreateWithCString( NULL,
                                                          file,
                                                          kCFStringEncodingUTF8 );
#endif

        IBlackmagicRawClip* localclip = nullptr;
        result = codec->OpenClip(filename, &localclip);
        if (result != S_OK)
        {
            return false;
        }

#ifdef OSX
        CFRelease( filename );
#endif

        localclip->Release();
        localclip = nullptr;

        codec->Release();
        codec = nullptr;

        return true;
    }

    void brawImage::parse_metadata(int64_t frame,
                                   IBlackmagicRawMetadataIterator*
                                   metadataIterator)
    {
#ifdef WIN32
        BSTR keyStr;
        VARIANT value;
#elif LINUX
        const char* keyStr = nullptr;
        Variant value;
#elif OSX
        CFStringRef keyStr;
        Variant value;
#endif

        if ( _attrs.find( frame ) == _attrs.end() )
        {
            _attrs.insert( std::make_pair( frame, Attributes() ) );
        }

        HRESULT result;
        while (SUCCEEDED(metadataIterator->GetKey(&keyStr)))
        {
            VariantInit(&value);
            result = metadataIterator->GetData(&value);
            if (result != S_OK)
            {
                LOG_ERROR( _("Failed to get data from "
                             "IBlackmagicRawMetadataIterator!") );
                break;
            }


#ifdef LINUX
            const char* key = (const char*) keyStr;
            BlackmagicRawVariantType variantType = value.vt;
#elif WIN32
            // or use true to get original BSTR released through wrapper
            _bstr_t interim(keyStr, false);
            const char* key((const char*) interim);
            VARTYPE variantType = value.vt;
#elif OSX
            const char* key = CFStringGetCStringPtr( keyStr,
                                                     kCFStringEncodingUTF8 );
            BlackmagicRawVariantType variantType = value.vt;
#endif

            if ( key == NULL || strlen(key) == 0 ) {
                VariantClear(&value);
                metadataIterator->Next();
                continue;
            }

            switch (variantType)
            {
            case blackmagicRawVariantTypeS16:
            {
                short s16 = value.iVal;
                Imf::IntAttribute attr( s16 );
                if ( frame < 0 )
                {
                    _clip_attrs.insert( std::make_pair( key, attr.copy() ) );
                }
                else
                {
                    _attrs[frame].insert( std::make_pair( key, attr.copy() ) );
                }
                break;
            }
            case blackmagicRawVariantTypeU16:
            {
                unsigned short u16 = value.uiVal;
                Imf::IntAttribute attr( u16 );
                if ( frame < 0 )
                {
                    _clip_attrs.insert( std::make_pair( key, attr.copy() ) );
                }
                else
                {
                    _attrs[frame].insert( std::make_pair( key, attr.copy() ) );
                }
                break;
            }
            case blackmagicRawVariantTypeS32:
            {
                int i32 = value.intVal;
                Imf::IntAttribute attr( i32 );
                if ( frame < 0 )
                {
                    _clip_attrs.insert( std::make_pair( key, attr.copy() ) );
                }
                else
                {
                    _attrs[frame].insert( std::make_pair( key, attr.copy() ) );
                }
                break;
            }
            case blackmagicRawVariantTypeU32:
            {
                unsigned int u32 = value.uintVal;
                Imf::IntAttribute attr( u32 );
                if ( frame < 0 )
                {
                    _clip_attrs.insert( std::make_pair( key, attr.copy() ) );
                }
                else
                {
                    _attrs[frame].insert( std::make_pair( key, attr.copy() ) );
                }
                break;
            }
            case blackmagicRawVariantTypeFloat32:
            {
                float f32 = value.fltVal;
                Imf::FloatAttribute attr( f32 );
                if ( frame < 0 )
                {
                    _clip_attrs.insert( std::make_pair( key, attr.copy() ) );
                }
                else
                {
                    _attrs[frame].insert( std::make_pair( key, attr.copy() ) );
                }
                break;
            }
            case blackmagicRawVariantTypeString:
            {
#ifdef LINUX
                const char* str = value.bstrVal;
#elif WIN32
                BSTR tmp = value.bstrVal;
                // or use true to get original BSTR released through wrapper
                _bstr_t interim(tmp, false);
                const char* str((const char*) interim);
#elif OSX
                const char* str = CFStringGetCStringPtr( value.bstrVal,
                                                         kCFStringEncodingUTF8 );
#endif
                if ( str == NULL ) str = "";

                Imf::StringAttribute attr( str );
                if ( frame < 0 )
                {
                    _clip_attrs.insert( std::make_pair( key, attr.copy() ) );
                }
                else
                {
                    _attrs[frame].insert( std::make_pair( key, attr.copy() ) );
                }
                break;
            }
            }

            VariantClear(&value);

            metadataIterator->Next();
        }

        image_damage( image_damage() | kDamageData );
    }

    bool brawImage::initialize()
    {
        init = true;
        HRESULT result;

        result = factory->CreateCodec(&codec);
        if (result != S_OK)
        {
            LOG_ERROR( _("Failed to create IBlackmagicRaw Codec!") );
            return false;
        }

#ifdef WIN32
        _bstr_t file = filename();
#elif LINUX
        const char* file = filename();
#elif OSX
        const char* fname = filename();
        if ( fname == NULL || strlen(fname) == 0 ) return false;
        CFStringRef file = CFStringCreateWithCString( NULL,
                                                      fname,
                                                      kCFStringEncodingUTF8 );
#endif

        result = codec->OpenClip( file, &clip );
        if (result != S_OK)
        {
            IMG_ERROR( _("Could not open clip" ) );
            return false;
        }

#ifdef OSX
        CFRelease( file );
#endif




        rgb_layers();
        lumma_layers();
        alpha_layers();

        _frameStart = _frame_start = 0;
        uint64_t duration;
        result = clip->GetFrameCount( &duration );
        if (result != S_OK)
        {
            IMG_ERROR( _("Could not obtain frame count" ) );
            return false;
        }
        _frameEnd = _frame_end = (int64_t)duration - 1;
        float f;
        result = clip->GetFrameRate( &f );
        if ( result != S_OK )
        {
            IMG_ERROR( _("Could not obtain frame rate" ) );
            return false;
        }
        _fps = _orig_fps = _play_fps = f;

        video_info_t info;
        info.has_codec = true;
        info.codec_name = "BRAW";
        info.fourcc = "BRAW";
        info.fps = _fps;
        info.pixel_format = "RGBA";
        info.has_b_frames = true;
        info.start = 0;
        info.duration = (double)(_frame_end - _frame_start) /
                        (double) _fps;
        _video_info.push_back( info );


        IBlackmagicRawMetadataIterator* clipMetadataIterator = nullptr;

        result = clip->GetMetadataIterator(&clipMetadataIterator);
        if (result != S_OK)
        {
            LOG_ERROR(_("Failed to get clip IBlackmagicRawMetadataIterator!"));
        }

        parse_metadata( -1, clipMetadataIterator );

        image_damage( image_damage() | kDamageData );

        result = clip->QueryInterface(IID_IBlackmagicRawClipAudio,
                                      (void**)&audio);
        if ( result != S_OK ) return true;

        uint32_t channelCount;
        uint32_t sampleRate;

        BlackmagicRawAudioFormat format;
        result = audio->GetAudioFormat(&format);
        if (result != S_OK)
        {
            IMG_ERROR( _("Failed to get Audio Format!") );
            return false;
        }

        result = audio->GetAudioSampleCount(&audioSamples);
        if (result != S_OK)
        {
            IMG_ERROR( _("Failed to get total audio sample count!") );
            return false;
        }

        result = audio->GetAudioBitDepth(&bitDepth);
        if (result != S_OK)
        {
            IMG_ERROR( _("Failed to get Audio Bit Depth!") );
            return false;
        }

        result = audio->GetAudioChannelCount(&channelCount);
        if (result != S_OK)
        {
            IMG_ERROR( _("Failed to get Audio Channel Count!") );
            return false;
        }

        result = audio->GetAudioSampleRate(&sampleRate);
        if (result != S_OK)
        {
            IMG_ERROR( _("Failed to get Audio Sample Rate!") );
            return false;
        }

        audio_info_t a;
        a.has_codec = bitDepth == 0 ? false : true;
        a.codec_name = "BRAW Audio";
        a.fourcc = "BRAW";
        if ( format == blackmagicRawAudioFormatPCMLittleEndian )
        {
            switch( bitDepth )
            {
            case 8:
                _audio_format = AudioEngine::kU8;
                a.format = "u8";
                break;
            case 16:
                _audio_format = AudioEngine::kS16LSB;
                a.format = "s16le";
                break;
            case 24:
                _audio_format = AudioEngine::kS32LSB;
                a.format = "s24le";
                break;
            case 32:
                _audio_format = AudioEngine::kS32LSB;
                a.format = "s32le";
                break;
            default:
                LOG_ERROR( _("Unknown audio bit depth ") << bitDepth );
                _audio_format = AudioEngine::kS16MSB;
                a.format = "s16be";
                break;
            }
        }
        else
        {
            switch( bitDepth )
            {
            case 8:
                _audio_format = AudioEngine::kU8;
                a.format = "u8";
                break;
            case 16:
                _audio_format = AudioEngine::kS16MSB;
                a.format = "s16be";
                break;
            case 24:
                _audio_format = AudioEngine::kS32MSB;
                a.format = "s24be";
                break;
            case 32:
                _audio_format = AudioEngine::kS32MSB;
                a.format = "s32be";
                break;
            default:
                LOG_ERROR( _("Unknown audio bit depth ") << bitDepth );
                _audio_format = AudioEngine::kS16MSB;
                a.format = "s16be";
                break;
            }
        }


        a.channels = _audio_channels = channelCount;
        a.frequency = frequency = sampleRate;
        a.bitrate = sampleRate * channelCount * bitDepth;
        a.language = _("und");
        a.start = 0;
        a.duration = audioSamples / (double) frequency;

        _audio_info.push_back( a );

        if ( a.has_codec )
        {
            _audio_index = (int) _audio_info.size() - 1;
            audio_initialize();

            if ( !_audio_buf )
            {
                _audio_max = AVCODEC_MAX_AUDIO_FRAME_SIZE * 2;
                // Final buffer for 32 bit audio
                _audio_buf = new aligned16_uint8_t[ _audio_max ];
                memset( _audio_buf, 0, _audio_max );

                static constexpr uint32_t maxSampleCount = 48000;
                uint32_t bufferSize = (maxSampleCount*_audio_channels*
                                       bitDepth)/8;

                // temporary buffer for 24 bit audio
                audiobuffer = new int8_t[ bufferSize ];

                char buf[256];
                uint64_t  in_ch_layout =
                    get_valid_channel_layout( 0, _audio_channels);
                if ( in_ch_layout == 0 )
                    in_ch_layout = get_valid_channel_layout( AV_CH_LAYOUT_STEREO,
                                                             _audio_channels);

                if ( in_ch_layout == 0 )
                    in_ch_layout = get_valid_channel_layout( AV_CH_LAYOUT_MONO,
                                                             _audio_channels);

                if ( ! _is_thumbnail )
                {
                    av_get_channel_layout_string( buf, 256, _audio_channels,
                                                  in_ch_layout );
                    IMG_INFO( _("Audio ") << buf << _(", channels ")
                              << _audio_channels );
                    IMG_INFO( _("format ") << a.format
                              << _(", frequency ") << frequency );
                }
            }
        }

        return true;
    }

    bool brawImage::finalize()
    {
        if ( audio )
            audio->Release();
        audio = nullptr;
        if ( clip )
            clip->Release();
        clip = nullptr;
        if ( codec )
            codec->Release();
        codec = nullptr;
        if ( factory )
            factory->Release();
        factory = nullptr;

        delete [] audiobuffer;
        audiobuffer = NULL;

        init = false;

        return true;
    }


    void brawImage::clear_cache()
    {
        SCOPED_LOCK( _mutex );
        _images.clear();
        image_damage( image_damage() | kDamageCache | kDamageContents );
    }

    CMedia::Cache brawImage::is_cache_filled( int64_t frame )
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


    bool brawImage::frame( const int64_t f )
    {

        if ( Preferences::max_memory <= CMedia::memory_used )
        {
            int64_t max_frames = (int64_t) max_image_frames();
            if ( std::abs( f - _frame ) >= max_frames )
                return false;
            limit_video_store( f );
            if ( has_audio() ) limit_audio_store( f );
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
            _audio_packets.push_back( pkt );
        }


        _expected = _dts + 1;
        _expected_audio = _expected + _audio_offset;
        return true;
    }

    void brawImage::copy_values()
    {
        _old_scale = _scale;
    }

    bool brawImage::fetch( mrv::image_type_ptr& canvas,
                           const int64_t frame )
    {
        if ( frame < _frame_start || frame > _frame_end )
            return false;

        if ( _scale != _old_scale )
        {
            clear_cache();
            copy_values();
        }

        BlackmagicRawResolutionScale s = blackmagicRawResolutionScaleEighth;
        switch ( _scale )
        {
        case 0:
            s = blackmagicRawResolutionScaleFull;
            break;
        case 1:
            s = blackmagicRawResolutionScaleHalf;
            break;
        case 2:
            s = blackmagicRawResolutionScaleQuarter;
            break;
        case 3:
        default:
            s = blackmagicRawResolutionScaleEighth;
            break;
        }

        HRESULT result = S_OK;

        CameraCodecCallback callback( this, canvas, frame, s );
        result = codec->SetCallback(&callback);
        if (result != S_OK)
        {
            LOG_ERROR( "Failed to set IBlackmagicRawCallback!" );
            return false;
        }

        result = clip->CreateJobReadFrame(frame, &readJob);
        if (result != S_OK)
        {
            LOG_ERROR( "Failed to create IBlackmagicRawJob!" );
            return false;
        }

        result = readJob->Submit();
        if (result != S_OK)
        {
            // submit has failed, the ReadComplete callback won't be called,
            // release the job here instead
            readJob->Release();
            LOG_ERROR( "Failed to submit IBlackmagicRawJob!" );
            return false;
        }

        codec->FlushJobs();


        IBlackmagicRawFrame* f = callback.GetFrame();

        if (f == nullptr)
        {
            LOG_ERROR( _("Failed to get IBlackmagicRawFrame!") );
            return true;
        }

        IBlackmagicRawMetadataIterator* frameMetadataIterator = nullptr;
        result = f->GetMetadataIterator(&frameMetadataIterator);
        if (result != S_OK)
        {
            LOG_ERROR( _("Failed to get frame "
                         "IBlackmagicRawMetadataIterator!") );
            return true;
        }

        parse_metadata(frame, frameMetadataIterator);


        return true;
    }

    bool brawImage::find_image( int64_t& frame )
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
                        frame = _frame = _hires->frame();
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


#ifdef WIN32
            BSTR timeCode;
#elif LINUX
            const char* timeCode = nullptr;
#elif OSX // APPLE
            CFStringRef timeCode;
#endif
            HRESULT result = clip->GetTimecodeForFrame( frame, &timeCode );
            if ( result != S_OK )
            {
                LOG_ERROR( _("Could not get timecode for frame ") << frame );
            }

#ifdef WIN32
            _bstr_t tmp( timeCode, false );
            const char* timecode( (const char*)tmp );
#elif LINUX
            const char* timecode = timeCode;
#elif OSX  // APPLE
            const char* timecode = CFStringGetCStringPtr( timeCode,
                                                          kCFStringEncodingUTF8 );
            if ( timecode == NULL ) return false;
#endif

            Imf::TimeCode t = str2timecode( timecode );
            if ( frame == start_frame() )
            {
                process_timecode( t );
                _tc_frame++;
            }
            Imf::TimeCodeAttribute attr( t );
            _attrs[frame].insert( std::make_pair( "timecode", attr.copy() ) );

#ifdef OSX
            CFRelease( timeCode );
#endif

            refresh();
            image_damage( image_damage() | kDamageTimecode );

        }  // release lock


        return true;
    }

    void brawImage::timed_limit_store( const int64_t frame )
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
        {
            SCOPED_LOCK( _mutex );
            TimedSeqMap tmp;
            {
                video_cache_t::iterator  it = _images.begin();
                video_cache_t::iterator end = _images.end();
                for ( ; it != end; ++it )
                {
                    tmp.insert( std::make_pair( (*it)->ptime(), (*it)->frame() ) );
                }
            }

            TimedSeqMap::iterator it = tmp.begin();
            for ( ; it != tmp.end() &&
                      memory_used >= Preferences::max_memory; ++it )
            {
                if ( _images.size() < max_frames ) break;
                auto start = std::remove_if( _images.begin(), _images.end(),
                                             EqualFunctor( it->second ) );
                _images.erase( start, _images.end() );
            }

        }

    }

//
// Limit the video store to approx. max frames images on each side.
// We have to check both where frame is as well as where _dts is.
//
    void brawImage::limit_video_store(const int64_t frame)
    {

        return timed_limit_store( frame );


    }

    CMedia::DecodeStatus brawImage::decode_audio( int64_t& f )
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

        if ( !_audio_packets.empty() )
            _audio_packets.pop_front();

        int64_t frame = f;

        bool ok = in_audio_store( frame );
        if ( ok ) return kDecodeOK;

        HRESULT result;


        unsigned int bytes_per_frame = audio_bytes_per_frame();

        static constexpr uint32_t maxSampleCount = 48000;
        uint32_t bufferSize = (maxSampleCount*_audio_channels*
                               bitDepth)/8;

        uint32_t startSample = (uint32_t) ( frame *
                                            ( (double) frequency / _orig_fps ));


        uint32_t samplesRead;
        uint32_t bytesRead;

        result = audio->GetAudioSamples(startSample,
                                        audiobuffer,
                                        bufferSize,
                                        maxSampleCount,
                                        &samplesRead,
                                        &bytesRead);

        if (result != S_OK)
        {
            IMG_ERROR( _("Could not get audio samples") );
            return kDecodeMissingSamples;
        }

        assert0( _audio_buf_used % 16 == 0 );
        int32_t* tmp = (int32_t*) ((int8_t*)_audio_buf + _audio_buf_used);

        size_t j = 0, i = 0;
        if ( bitDepth == 24 )
        {
            for ( size_t i = 0; i < bytesRead; i += 3, ++tmp )
            {
                uint32_t base = *((uint32_t*) ((uint8_t*)audiobuffer + i) );
                AV_WN32A( tmp, (uint32_t)(base << 8) );
            }
        }
        else
        {
            IMG_ERROR( _("Bitdepth to process is unknown") );
            return kDecodeError;
        }

        _audio_buf_used += (bytesRead / 3) * 4;

        CMedia::DecodeStatus got_audio = kDecodeMissingFrame;

        if (samplesRead == 0 && _audio_buf_used == 0 ) return got_audio;

        int64_t last = frame;
        unsigned int index = 0;

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
                abort();
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

        if (_audio_buf_used > 0 && index > 0 )
        {
            //
            // NOTE: audio buffer must remain 16 bits aligned for ffmpeg.
            memmove( _audio_buf, _audio_buf + index, _audio_buf_used );
        }

        return got_audio;
    }

    bool brawImage::has_changed()
    {
        if ( is_cache_filled( _frame ) )
        {
            if ( refetch( _frame ) )
            {
                int64_t f = _frame;
                find_image( f );
            }
        }
        image_damage( image_damage() | kDamageCache | kDamageContents );
        return true;
    }

    void brawImage::do_seek() {
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


    void brawImage::audio_stream( int idx )
    {
        CMedia::audio_stream( idx );
        if ( idx < 0 ) return;

        uint32_t channelCount;
        HRESULT result = audio->GetAudioChannelCount(&channelCount);
        if (result != S_OK)
        {
            IMG_ERROR( _("Failed to get Audio Channel Count!") );
            return;
        }
        _audio_channels = channelCount;
        _audio_format = AudioEngine::kS32LSB;
    }

    void brawImage::debug_video_stores(const int64_t frame,
                                       const char* routine,
                                       const bool detail )
    {

        SCOPED_LOCK( _mutex );

        video_cache_t::const_iterator iter = _images.begin();
        video_cache_t::const_iterator last = _images.end();

        std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:"
                  << frame
                  << " " << routine << " video stores  #"
                  << _images.size() << ": ";


        bool dtail = detail;

        if ( iter != last )
        {
            video_cache_t::const_iterator end = last - 1;

            std::cerr << std::dec;
            std::cerr << (*iter)->frame() << "-"
                      << (*end)->frame()
                      << std::endl;

            if ( (*iter)->frame() > (*end)->frame() )
                dtail = true;
        }
        else
            std::cerr << std::endl;

        if ( dtail )
        {
            std::cerr << std::dec;
            for ( ; iter != last; ++iter )
            {
                int64_t f = (*iter)->frame();
                if ( f == frame )  std::cerr << "S";
                if ( f == _dts )   std::cerr << "D";
                if ( f == _frame ) std::cerr << "F";
                std::cerr << f << " (" << (*iter)->ptime().tv_sec
                          << "." << (*iter)->ptime().tv_usec << ") ";
            }
            std::cerr << endl;
        }
    }

}
