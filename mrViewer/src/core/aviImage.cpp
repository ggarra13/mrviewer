/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2016  Gonzalo GarramuÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂÃÂ±o

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
 * @file   aviImage.cpp
 * @author gga
 * @date   Tue Sep 26 17:54:48 2006
 *
 * @brief  Read and play an avi/mov/wmv file with audio.
 *         We rely on using the ffmpeg library.
 *
 */

#include <cstdio>

#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <iostream>
#include <algorithm>
#include <limits>
#include <cstring>
using namespace std;


#if !defined(WIN32) && !defined(WIN64)
#  include <arpa/inet.h>
#else
#  include <winsock2.h>    // for htonl
#endif



extern "C" {
#include <libavutil/mathematics.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavutil/avstring.h>
#include <libswresample/swresample.h>
#include <libavutil/mastering_display_metadata.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}


#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <ImfStringAttribute.h>

#include "core/mrvPlayback.h"
#include "core/mrvHome.h"
#include "core/Sequence.h"
#include "core/aviImage.h"
#include "core/mrvFrameFunctors.h"
#include "core/mrvThread.h"
#include "core/mrvCPU.h"
#include "core/mrvColorSpaces.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
#include "video/mrvGLEngine.h"
#include "mrViewer.h"


namespace
{
const char* kModule = "avi";
}



//#define DEBUG_STREAM_INDICES
//#define DEBUG_STREAM_KEYFRAMES
//#define DEBUG_DECODE
//#define DEBUG_DECODE_POP_AUDIO
//#define DEBUG_DECODE_AUDIO
//#define DEBUG_SEEK
//#define DEBUG_SEEK_VIDEO_PACKETS
//#define DEBUG_SEEK_AUDIO_PACKETS
//#define DEBUG_SEEK_SUBTITLE_PACKETS
//#define DEBUG_HSEEK_VIDEO_PACKETS
//#define DEBUG_VIDEO_PACKETS
//#define DEBUG_VIDEO_STORES
//#define DEBUG_AUDIO_PACKETS
//#define DEBUG_PACKETS
//#define DEBUG_PACKETS_DETAIL
//#define DEBUG_AUDIO_STORES
// #define DEBUG_STORES_DETAIL
//#define DEBUG_SUBTITLE_STORES
//#define DEBUG_SUBTITLE_RECT



//  in ffmpeg, sizes are in bytes...
#define kMAX_QUEUE_SIZE (30 * 1024 * 1024)
#define kMAX_PACKET_SIZE 50
#define kMAX_AUDIOQ_SIZE (20 * 16 * 1024)
#define kMAX_SUBTITLEQ_SIZE (5 * 30 * 1024)
#define kMIN_FRAMES 25

namespace {
const unsigned int  kMaxCacheImages = 70;
}

namespace mrv {


int CMedia::colorspace_override = 0;

const char* const kColorRange[] = {
    _("Unspecified"),
    "MPEG", ///< the normal 219*2^(n-8) "MPEG" YUV ranges
    "JPEG", ///< the normal     2^n-1   "JPEG" YUV ranges
};

const char* const kColorSpaces[] = {
    "RGB",
    "BT709",
    _("Unspecified"),
    _("Reserved"),
    "FCC",
    "BT470BG", ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
    "SMPTE170M", ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC / functionally identical to above
    "SMPTE240M",
    "YCOCG", ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
    "BT2020_NCL", ///< ITU-R BT2020 non-constant luminance system
    "BT2020_CL", ///< ITU-R BT2020 constant luminance system
};

const size_t aviImage::colorspace_index() const
{
    if ( !_av_frame ) return 2; // Unspecified
    aviImage* img = const_cast< aviImage* >( this );
    if ( _colorspace_index < 0 ||
            _colorspace_index >= sizeof( kColorSpaces )/sizeof(char*) )
    {
        if ( colorspace_override ) img->_colorspace_index = colorspace_override;
        else img->_colorspace_index = _av_frame->colorspace;
    }
    return _colorspace_index;
}

const char* const aviImage::colorspace() const
{
    aviImage* img = const_cast< aviImage* >( this );
    return _( kColorSpaces[img->colorspace_index()] );
}

const char* const aviImage::color_range() const
{
    if ( !_av_frame ) return kColorRange[0];
    return kColorRange[_av_frame->color_range];
}


aviImage::aviImage() :
    CMedia(),
    _initialize( false ),
    _force_playback( false ),
    _has_image_seq( false ),
    _video_index(-1),
    _stereo_index(-1),
    _av_dst_pix_fmt( AV_PIX_FMT_RGB24 ),
    _pix_fmt( VideoFrame::kRGB ),
    _ptype( VideoFrame::kByte ),
    _av_frame( NULL ),
    _filt_frame( NULL ),
    _subtitle_ctx( NULL ),
    buffersink_ctx( NULL ),
    buffersrc_ctx( NULL ),
    filter_graph( NULL ),
    _convert_ctx( NULL ),
    _last_cached( false ),
    _max_images( kMaxCacheImages ),
    _inv_table( NULL )
{
    _gamma = 1.0f;
    _compression = "";

    memset(&_sub, 0, sizeof(AVSubtitle));
    _depth = mrv::image_type::kByte;
}


aviImage::~aviImage()
{

    if ( !stopped() )
        stop();

    image_damage(kNoDamage);

    _video_packets.clear();
    _subtitle_packets.clear();

    flush_video();
    flush_subtitle();

    if ( _convert_ctx )
    {
        sws_freeContext( _convert_ctx );
        _convert_ctx = NULL;
    }

    if ( filter_graph )
        avfilter_graph_free(&filter_graph);

    if ( _av_frame )
        av_frame_unref( _av_frame );
    if ( _filt_frame )
        av_frame_unref( _filt_frame );

    close_video_codec();
    close_subtitle_codec();

    if ( _av_frame )
        av_frame_free( &_av_frame );

    if ( _filt_frame )
        av_frame_free( &_filt_frame );

    avsubtitle_free( &_sub );

}


bool aviImage::test_filename( const char* buf )
{
    AVFormatContext* ctx = NULL;
    int error = avformat_open_input( &ctx, buf, NULL, NULL );
    if ( ctx )
        avformat_close_input( &ctx );

    if ( error < 0 ) return false;

    return true;
}


/*! Test a block of data read from the start of the file to see if it
  looks like the start of an .avi file. This returns true if the
  data contains RIFF as magic number and a chunk of 'AVI '
  following.
  I tried opening the file as the file test but it was too sensitive and
  was opening mray's map files as sound files.
*/
bool aviImage::test(const boost::uint8_t *data, unsigned len)
{
    if ( len < 12 ) return false;


    unsigned int magic = ntohl( *((unsigned int*)data) );

    // std::cerr << std::hex << tag2 << std::dec << std::endl;

    if ( magic == 0x000001ba || magic == 0x00000001 )
    {
        // MPEG movie
        return true;
    }
    else if ( magic == 0x1a45dfa3 )
    {
        // Matroska
        return true;
    }
    else if ( magic == 0x3026B275 )
    {
        // WMV movie
        magic = ntohl( *(unsigned int*)(data+4) );
        if ( magic != 0x8E66CF11 ) return false;

        magic = ntohl( *(unsigned int*)(data+8) );
        if ( magic != 0xA6D900AA ) return false;

        magic = ntohl( *(unsigned int*)(data+12) );
        if ( magic != 0x0062CE6C ) return false;
        return true;
    }
    else if ( strncmp( (char*)data, "FLV", 3 ) == 0 )
    {
        // FLV
        return true;
    }
    else if ( ( strncmp( (char*)data, "GIF89a", 6 ) == 0 ) ||
              ( strncmp( (char*)data, "GIF87a", 6 ) == 0 ) )
    {
        // GIF89a/87a
        return true;
    }
    else if ( strncmp( (char*)data, ".RMF", 4 ) == 0 )
    {
        // Real Movie
        return true;
    }
    else if ( strncmp( (char*)data, "OggS", 4 ) == 0 )
    {
        // Ogg / Vorbis
        return true;
    }
    else if ( strncmp( (char*)data, "RIFF", 4 ) == 0 )
    {
        // AVI or WAV
        const char* tag = (char*)data + 8;
        if ( strncmp( tag, "AVI ", 4 ) != 0 &&
                strncmp( tag, "WAVE", 4 ) != 0 &&
                strncmp( tag, "CDXA", 4 ) != 0 )
            return false;

        return true;
    }
    else if ( strncmp( (char*)data, "ID3", 3 ) == 0 ||
              (magic & 0xFFE00000) == 0xFFE00000 ||
              (magic == 0x00000000) )
    {
        // MP3
        if ( (magic != 0x00000000) &&
                ((magic & 0xF000) == 0xF000 ||
                 (magic & 0xF000) == 0 ) ) return false;
        return true;
    }
    else if ( magic == 0x00000144 )
    {
        // RED ONE camera images
        if ( strncmp( (char*)data+4, "RED1", 4 ) != 0 )
            return false;
        return true;
    }
    else if ( CMedia::load_library == CMedia::kFFMPEGLibrary &&
              magic == 0x89504E47 )
    {
        // PNG
        unsigned int tag = ntohl( *((unsigned int*)data+1) );
        if ( tag != 0x0D0A1A0A ) return false;

        return true;
    }
    else if ( magic == 0x060E2B34 )
    {
        // MXF
        unsigned int tag = ntohl( *((unsigned int*)data+1) );
        if ( tag != 0x02050101 ) return false;

        tag = ntohl( *((unsigned int*)data+2) );
        if ( tag != 0x0D010201 ) return false;

        return true;
    }
    else if ( strncmp( (char*)data, "YUV4MPEG2", 9 ) == 0 )
    {
        return true;
    }
    else if ( strncmp( (char*)data, "DHAV", 4 ) == 0 )
    {
        return true;
    }
    else if ( CMedia::load_library == CMedia::kFFMPEGLibrary &&
              magic == 0xFFD8FFE0 )
    {
        // JPEG
        if ( strncmp( (char*)data + 6, "JFIF", 4 ) == 0 )
            return true;
    }
    else if ( CMedia::load_library == CMedia::kFFMPEGLibrary &&
              ( ( memcmp( (char*)&magic, "XPDS", 4) == 0 ) ||
                ( memcmp( (char*)&magic, "SDPX", 4) == 0 ) ) )
    {
        // DPX
        return true;
    }
    else
    {
        // Check for Quicktime
        if ( strncmp( (char*)data+4, "ftyp", 4 ) == 0 ||
                strncmp( (char*)data+4, "moov", 4 ) == 0 ||
                strncmp( (char*)data+4, "free", 4 ) == 0 ||
                strncmp( (char*)data+4, "mdat", 4 ) == 0 ||
                strncmp( (char*)data+4, "wide", 4 ) == 0 ||
                strncmp( (char*)data+4, "pnot", 4 ) == 0 )
            return true;
    }

    // For M2TS (AVCHD), we search for 0x47 and if so, we do the full check
    // in ffmpeg.
    for ( int i = 0; i < 128; i += 4 )
    {
        unsigned int magic = ntohl( *(unsigned int*)(data+i) );
        if ( ( magic & 0x47000000 ) == 0x47000000 )
        {
            uint8_t* d = new uint8_t[ len + AVPROBE_PADDING_SIZE ];
            memset( d+len, 0, AVPROBE_PADDING_SIZE );
            memcpy( d, data, len );

            AVProbeData pd = { NULL, d, static_cast<int>(len), "video/MP2T" };
            AVInputFormat* ctx = av_probe_input_format(&pd, 1);

            delete [] d;

            if ( ctx && ( strcmp( ctx->name, "mpegts" ) == 0 ) )
                return true;

            return false;
        }
    }

    return false;

}

// Returns the current subtitle stream or NULL if none available
AVStream* aviImage::get_subtitle_stream() const
{
    return _subtitle_index >= 0 ? _context->streams[ subtitle_stream_index() ] : NULL;
}

// Returns the current video stream or NULL if none available
AVStream* aviImage::get_video_stream() const
{
    CMedia::Mutex& mtx = const_cast< Mutex& >( _mutex );
    SCOPED_LOCK( mtx );
    return _video_index >= 0 ? _context->streams[ video_stream_index() ] : NULL;
}

int aviImage::init_filters(const char *filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs  = avfilter_inout_alloc();
    AVRational time_base = get_video_stream()->time_base;
    AVRational fr = av_guess_frame_rate(_context, get_video_stream(), NULL);
    enum AVPixelFormat pix_fmts[] = { _video_ctx->pix_fmt, AV_PIX_FMT_NONE };

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph || !buffersrc || !buffersink) {
        LOG_ERROR( _("No memory to allocate filter graph") );
        ret = AVERROR(ENOMEM);
        goto end;
    }


    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             _video_ctx->width, _video_ctx->height, _video_ctx->pix_fmt,
             time_base.num, time_base.den,
             _video_ctx->sample_aspect_ratio.num,
             _video_ctx->sample_aspect_ratio.den);
    if (fr.num && fr.den)
        av_strlcatf(args, sizeof(args), ":frame_rate=%d/%d", fr.num, fr.den);


    LOG_INFO( "args " << args );

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        LOG_ERROR( _( "Cannot create buffer source" ) );
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        LOG_ERROR( _("Cannot create buffer sink" ) );
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOG_ERROR( _("Cannot set output pixel format" ) );
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
    {
        LOG_ERROR( _("Error parsing filter description") );
        goto end;
    }

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
    {
        LOG_ERROR( _("Error configuring filter graph") );
        goto end;
    }


end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}


void aviImage::subtitle_file( const char* f )
{
    if ( _right_eye )
    {
        aviImage* aviR = dynamic_cast< aviImage* >( _right_eye );
        if ( aviR )
        {
            aviR->subtitle_file( f );
        }
    }


    flush_subtitle();

    close_subtitle_codec();

    SCOPED_LOCK( _subtitle_mutex );

    avfilter_graph_free( &filter_graph );
    filter_graph = NULL;

    if ( _filt_frame )
    {
        av_frame_unref( _filt_frame );
        av_frame_free( &_filt_frame );
    }

    if ( !has_video() )
    {
        return;
    }

    _subtitle_info.clear();
    _subtitle_index = -1;

    if ( f == NULL || strlen(f) == 0 )
        _subtitle_file.clear();
    else
    {

        _subtitle_file = f;

        AVFormatContext* scontext = NULL; //!< current read file context

        AVDictionary *opts = NULL;
        AVInputFormat*     format = NULL;
        int error = avformat_open_input( &scontext, _subtitle_file.c_str(),
                                         format, &opts );
        if ( error < 0 )
        {
            char errbuf[256];
            av_make_error_string( errbuf, 255, error );
            LOG_ERROR( name() << " - " << _subtitle_file << ": " <<  errbuf );
            return;
        }

        // Iterate through all the streams available
        for( unsigned i = 0; i < scontext->nb_streams; ++i )
        {
            // Get the codec context
            const AVStream* stream = scontext->streams[ i ];

            if ( stream == NULL ) continue;

            const AVCodecParameters* par = stream->codecpar;
            if ( par == NULL ) continue;


            AVCodec* codec = avcodec_find_decoder( par->codec_id );
            AVCodecContext* ctx = avcodec_alloc_context3( codec );
            int err = avcodec_parameters_to_context( ctx, par );
            if ( err < 0 )
            {
                LOG_ERROR( _("Could not copy parameters to context") );
            }

            // Determine the type and obtain the first index of each type
            switch( ctx->codec_type )
            {
            case AVMEDIA_TYPE_SUBTITLE:
            {
                subtitle_info_t s;
                std::ostringstream msg;
                populate_stream_info( s, msg, scontext, par, i );
                s.bitrate    = calculate_bitrate( stream, par );
                s.play = false;
                _subtitle_info.push_back( s );
                break;
            }
            case AVMEDIA_TYPE_ATTACHMENT:
            case AVMEDIA_TYPE_DATA:
            case AVMEDIA_TYPE_VIDEO:
            default:
                break;
            }

            avcodec_free_context( &ctx );
        }

        if ( _subtitle_info.empty() )
        {
            IMG_ERROR( _("Could not find subtitle in '")
                       << _subtitle_file << "'" );
            return;
        }


        // Comment complicated characters in subtitle file.
        // Be wary of ' (single quote) \ and : which are special.
        std::string sub;
        const char* s = _subtitle_file.c_str();

        for ( ; *s != 0; ++s )
        {
            if ( *s == '\'' || *s == ':' || *s == '\\' )
            {
                // Double quote this so a \ gets passed in first escape path
                sub += "\\\\";
            }
            if ( *s == ' ' || *s == '('  || *s == ')'  || *s == ',' ||
                    *s == ':' || *s == '\\' || *s == '\'' || *s == '[' ||
                    *s == ']' )
            {
                sub += '\\';
            }
            sub += *s;
        }


        LOG_INFO( _("Subtitle file ") << sub );
        LOG_INFO( _("Subtitle font ") << _subtitle_font );
        LOG_INFO( _("Subtitle encoding ") << _subtitle_encoding );
        _filter_description = "subtitles=";
        _filter_description += sub;
        _filter_description += ":charenc=";
        _filter_description += _subtitle_encoding;
        _filter_description += ":force_style='FontName=";
        _filter_description += _subtitle_font;
        _filter_description += "'";

        int ret;
        if ( ret = init_filters( _filter_description.c_str() ) < 0 )
        {
            LOG_ERROR( "Could not init filters: ret " << ret
                       << " " << get_error_text(ret) );
            _subtitle_index = -1;
            avfilter_graph_free( &filter_graph );
            filter_graph = NULL;
            return;
        }
        else
        {
            _subtitle_index = 0;
        }

        _filt_frame = av_frame_alloc();
        if ( ! _filt_frame )
        {
            LOG_ERROR( _("Could not allocate filter frame.  "
                         "Not enough memory.") );
        }

        avformat_free_context( scontext );

        image_damage( image_damage() | kDamageSubtitle );
    }

}

bool aviImage::has_video() const
{
    return ( _video_index >= 0 && _video_info[ _video_index ].has_codec );
}


bool aviImage::valid_video() const
{
    // If there's at least one valid video stream, return true
    size_t num_streams = number_of_video_streams();

    bool valid = false;
    for ( size_t i = 0; i < num_streams; ++i )
    {
        if ( _video_info[i].has_codec ) {
            valid = true;
            break;
        }
    }

    return valid;
}

// Opens the video codec associated to the current stream
void aviImage::open_video_codec()
{
    AVStream *stream = get_video_stream();
    if ( stream == NULL ) return;

    AVCodecParameters* codecpar = stream->codecpar;

    AVCodec* video_codec = avcodec_find_decoder( codecpar->codec_id );

    _video_ctx = avcodec_alloc_context3(video_codec);
    int r = avcodec_parameters_to_context(_video_ctx, codecpar);
    if ( r < 0 )
    {
        LOG_ERROR( _("avcodec_context_from_parameters failed for video") );
        return;
    }


    static int workaround_bugs = 1;
    static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
    static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
    static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;
    static int idct = FF_IDCT_AUTO;
    static int error_concealment = 3;

    _video_ctx->codec_id        = video_codec->id;
    _video_ctx->workaround_bugs = workaround_bugs;
    // _video_ctx->skip_frame= skip_frame;
    // _video_ctx->skip_idct = skip_idct;
    // _video_ctx->skip_loop_filter= skip_loop_filter;
    // _video_ctx->idct_algo = idct;



    double aspect_ratio;
    if ( _video_ctx->sample_aspect_ratio.num == 0 )
        aspect_ratio = 0;
    else
        aspect_ratio = av_q2d( _video_ctx->sample_aspect_ratio ) *
                       _video_ctx->width / _video_ctx->height;




    if ( width() > 0 && height() > 0 )
    {
        double image_ratio = (double) width() / (double)height();
        if ( aspect_ratio <= 0.0 ) aspect_ratio = image_ratio;

        if ( image_ratio == aspect_ratio ) _pixel_ratio = 1.0;
        else _pixel_ratio = aspect_ratio / image_ratio;
    }

    avcodec_parameters_from_context( stream->codecpar, _video_ctx );

    AVDictionary* info = NULL;
    if (!av_dict_get(info, "threads", NULL, 0))
        av_dict_set(&info, "threads", Preferences::video_threads.c_str(), 0 );
    //av_dict_set(&info, "threads", "auto", 0);  // not "auto" nor "4"

    // recounted frames needed for subtitles
    av_dict_set(&info, "refcounted_frames", "1", 0);

    if ( video_codec == NULL ||
            avcodec_open2( _video_ctx, video_codec, &info ) < 0 )
        _video_index = -1;

}

void aviImage::close_video_codec()
{
    if ( _video_ctx && _video_index >= 0 )
    {
        avcodec_free_context( &_video_ctx );
    }
}


// Flush video buffers
void aviImage::flush_video()
{
    SCOPED_LOCK( _mutex );
    if ( _video_ctx && _video_index >= 0 )
    {
        avcodec_flush_buffers( _video_ctx );
    }
}


void aviImage::clear_cache()
{
    {
        SCOPED_LOCK( _mutex );
        _images.clear();
    }

    clear_stores();
}

/// VCR play (and cache frames if needed) sequence
void aviImage::play( const Playback dir, ViewerUI* const uiMain,
                     const bool fg )
{
    CMedia::play( dir, uiMain, fg );
}

CMedia::Cache aviImage::is_cache_filled( int64_t frame )
{
    bool ok = in_video_store( frame - _start_number );
    if ( ok && _stereo_input != kSeparateLayersInput ) return kStereoCache;
    return (CMedia::Cache) ok;
}

// Seek to the requested frame
bool aviImage::seek_to_position( const int64_t frame )
{


#ifdef DEBUG_SEEK
    LOG_INFO( "BEFORE SEEK:  D: " << _dts << " E: " << _expected );
#endif


    // double frac = ( (double) (frame - _frameStart) /
    //             (double) (_frameEnd - _frameStart) );
    // int64_t offset = int64_t( _context->duration * frac );
    // if ( _context->start_time != AV_NOPTS_VALUE )
    //    offset += _context->start_time;

    // static const AVRational base = { 1, AV_TIME_BASE };
    // int64_t min_ts = std::numeric_limits< int64_t >::max();

    if ( _context == NULL ) return false;


    bool skip = false;
    bool got_audio = !has_audio();
    bool got_video = !has_video();
    bool got_subtitle = !has_subtitle();

    if ( playback() == kStopped &&
            (got_video || in_video_store( frame )) &&
            (got_audio || in_audio_store( frame + _audio_offset )) &&
            (got_subtitle || in_subtitle_store( frame )) )
    {
        skip = true;
    }

    // With frame and reverse playback, we often do not get the current
    // frame.  So we search for frame - 1.
    int64_t start = frame;
    int64_t offset = 0;

    //if ( _start_number != start_frame() )
    {
        if ( _start_number != 0 )
        {
            start -= _start_number;
        }
        else
        {
            if ( playback() == kBackwards ) --start;
        }
    }
    if ( !skip ) --start;

    if ( start < 0 ) start = 0;

    // std::cerr << name() << std::endl << "-------------" << std::endl;
    // std::cerr << "_start_number " << _start_number << std::endl;
    // std::cerr << "start " << start << " AV_TIME_BASE " << AV_TIME_BASE
    //        << " fps " << fps() << std::endl;
    // std::cerr << "mult " << double(start * AV_TIME_BASE / fps() ) << std::endl;
    // std::cerr << "int64 "
    //        << int64_t( double(start * AV_TIME_BASE / fps() ) ) << std::endl;
    offset = int64_t( double(start * AV_TIME_BASE  / fps() ) );

    if ( offset < 0 ) offset = 0;


    int flag = AVSEEK_FLAG_BACKWARD;
    int ret = av_seek_frame( _context, -1, offset, flag );
    //int ret = avformat_seek_file( _context, -1,
    //                            std::numeric_limits<int64_t>::min(), offset,
    //                            std::numeric_limits<int64_t>::max(), flag );
    if (ret < 0)
    {
        IMG_ERROR( _("Could not seek to frame ") << start
                   << N_(" offset ") << offset
                   << N_(": ") << get_error_text(ret) );
        return false;
    }

    if ( _acontext )
    {
        offset = int64_t( double(start + _audio_offset )
                          * AV_TIME_BASE
                          / fps() );
        if ( offset < 0 ) offset = 0;

        int ret = av_seek_frame( _acontext, -1, offset, flag );

        if (ret < 0)
        {
            IMG_ERROR( _("Could not seek to frame ") << frame
                       << N_("(offset: ") << offset << N_(") : ")
                       << get_error_text(ret) );
            return false;
        }
    }


    // Skip the seek packets when playback is stopped (scrubbing)
    if ( skip )
    {
        int64_t f = frame-1;
        if ( f > _frame_end ) f = _frame_end;
        int64_t dts = queue_packets( f, false, got_video,
                                     got_audio, got_subtitle );
        _dts = _adts = dts;
        // Set the expected to an impossible frame
        _expected = _expected_audio = _frame_start-1;
        _seek_req = false;
        return true;
    }


    int64_t vpts = 0, apts = 0, spts = 0;

    mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
    SCOPED_LOCK( vpm );

    mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
    SCOPED_LOCK( apm );

    mrv::PacketQueue::Mutex& spm = _subtitle_packets.mutex();
    SCOPED_LOCK( spm );

    if ( !got_video ) {
        vpts = frame2pts( get_video_stream(), start );
    }

    if ( !got_audio ) {
        if ( _acontext )
        {
            apts = frame2pts( get_audio_stream(), start + 1 + _audio_offset );
        }
        else
        {
            apts = frame2pts( get_audio_stream(), start + 1);
        }
    }

    if ( !got_subtitle ) {
        spts = frame2pts( get_subtitle_stream(), start );
    }


#ifdef DEBUG_SEEK_VIDEO_PACKETS
    debug_video_packets(start, _right_eye ? "RBEFORE SEEK" : "BEFORE SEEK", true);
#endif

#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(start, _right_eye ? "RBEFORE SEEK" :  "BEFORE SEEK", true);
#endif

    if ( !_seek_req && playback() == kBackwards )
    {
        if ( !got_video )    _video_packets.preroll(vpts);
        if ( !got_audio )    _audio_packets.preroll(apts);
        if ( !got_subtitle ) _subtitle_packets.preroll(spts);
    }
    else
    {
        if ( !got_video )    _video_packets.seek_begin(vpts);
        if ( !got_audio && apts >= 0 )    _audio_packets.seek_begin(apts);
        if ( !got_subtitle ) _subtitle_packets.seek_begin(spts);
    }


    int64_t dts = queue_packets( frame, true, got_video,
                                 got_audio, got_subtitle );

    _dts = _adts = dts;
    assert( _dts >= first_frame() && _dts <= last_frame() );

    _expected = _expected_audio = dts + 1;
    _seek_req = false;


#ifdef DEBUG_SEEK
    LOG_INFO( "AFTER SEEK:  D: " << _dts << " E: " << _expected
              << " _frame_offset " << _frame_offset );
#endif

#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "AFTER SEEK");
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RAFTER SEEK" : "AFTER SEEK");
#endif

#ifdef DEBUG_SEEK_VIDEO_PACKETS
    debug_video_packets(frame,  "AFTER SEEK", true);
#endif

#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "RAFTER SEEK" : "AFTER SEEK", true);
#endif


    return true;
}


mrv::image_type_ptr aviImage::allocate_image( const int64_t& frame,
                                              const int64_t& pts
                                            )
{
    double aspect_ratio = (double)_w / (double) _h;
    if ( _w > mrv::GLEngine::maxTexWidth() )
        _w = mrv::GLEngine::maxTexWidth();
    if ( _h > mrv::GLEngine::maxTexHeight() )
        _h = (unsigned int) ( mrv::GLEngine::maxTexHeight() / aspect_ratio );
    return mrv::image_type_ptr( new image_type( frame,
                                width(),
                                height(),
                                (unsigned short) _num_channels,
                                _pix_fmt,
                                _ptype,
                                _av_frame->repeat_pict,
                                pts ) );
}


void aviImage::store_image( const int64_t frame,
                            const int64_t pts )
{


    AVStream* stream = get_video_stream();
    assert( stream != NULL );

    mrv::image_type_ptr image;
    try {
        image = allocate_image( frame, pts );
    }
    catch ( const std::bad_alloc& e )
    {
        LOG_ERROR( _("Not enough memory for image") );
        return;
    }
    catch ( const std::exception& e )
    {
        LOG_ERROR( _("Problem allocating image ") << e.what() );
        return;
    }


    if ( ! image )
    {
        IMG_ERROR( "No memory for video frame" );
        IMG_ERROR( "Audios #" << _audio.size() );
        IMG_ERROR( "Videos #" << _images.size() );
        return;
    }

    AVFrame output = { 0 };
    boost::uint8_t* ptr = (boost::uint8_t*)image->data().get();

    unsigned int w = width();
    unsigned int h = height();

    // Fill the fields of AVPicture output based on _av_dst_pix_fmt
    // avpicture_fill( &output, ptr, _av_dst_pix_fmt, w, h );
    av_image_fill_arrays( output.data, output.linesize, ptr, _av_dst_pix_fmt,
                          w, h, 1);

    AVPixelFormat fmt = _video_ctx->pix_fmt;
    switch (fmt)
    {
    case AV_PIX_FMT_YUVJ420P:
        fmt = AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        fmt = AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        fmt = AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        fmt = AV_PIX_FMT_YUV440P;
        break;
    default:
        break;
    }
    int sws_flags = 0;
    if ( (int)w < _video_ctx->width || (int)h < _video_ctx->height )
        sws_flags = SWS_BICUBIC;


    // We handle all cases directly except YUV410 and PAL8
    _convert_ctx = sws_getCachedContext(_convert_ctx,
                                        _video_ctx->width,
                                        _video_ctx->height,
                                        fmt, w, h,
                                        _av_dst_pix_fmt, sws_flags,
                                        NULL, NULL, NULL);

    if ( _convert_ctx == NULL )
    {
        IMG_ERROR( _("Could not get image conversion context.") );
        return;
    }

    int in_full, out_full, brightness, contrast, saturation;
    const int *inv_table, *table;

    int ret = sws_getColorspaceDetails( _convert_ctx,
                                        (int**)&inv_table,
                                        &in_full,
                                        (int**)&table,
                                        &out_full,
                                        &brightness,
                                        &contrast,
                                        &saturation );

    if ( ret < 0 )
    {
        char buf[128];
        av_strerror(ret, buf, 128);
        IMG_ERROR( _("Colorspace details not gotten ") << buf );
    }


    if ( _inv_table ) {
        inv_table = _inv_table;
        in_full = 0;
        out_full = 1;
    }

    // inv_table = sws_getCoefficients( SWS_CS_FCC );
    // inv_table = sws_getCoefficients( SWS_CS_SMPTE240M );
    // inv_table = sws_getCoefficients( SWS_CS_BT2020 );
    // inv_table = sws_getCoefficients( SWS_CS_ITU709 );
    //table     = sws_getCoefficients( SWS_CS_ITU601 );


    // std::cerr << "infull " << in_full << " out_full " << out_full
    //           << "brightness " << brightness << " contrast " << contrast
    //           << "saturation " << saturation << std::endl;

    ret = sws_setColorspaceDetails(_convert_ctx, inv_table, in_full,
                                   table, out_full,
                                   brightness, contrast, saturation);

    if ( ret < 0 )
    {
        char buf[128];
        av_strerror(ret, buf, 128);
        IMG_ERROR( _("Colorspace details not set ") << buf );
    }

    _av_frame->color_range = out_full ? AVCOL_RANGE_JPEG : AVCOL_RANGE_MPEG;

    sws_scale(_convert_ctx, _av_frame->data, _av_frame->linesize,
              0, _video_ctx->height, output.data, output.linesize);

    if ( _av_frame->interlaced_frame )
        _interlaced = ( _av_frame->top_field_first ?
                        kTopFieldFirst : kBottomFieldFirst );

    SCOPED_LOCK( _mutex );

    if ( _images.empty() || _images.back()->frame() < frame )
    {
        _images.push_back( image );
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

        _images.insert( at, image );
    }

}


int CMedia::decode(AVCodecContext *avctx, AVFrame *frame, int *got_frame,
                   AVPacket *pkt, bool& eof)
{
    int ret = 0;

    *got_frame = 0;

    if (pkt) {
        ret = avcodec_send_packet(avctx, pkt);

        // In particular, we don't expect AVERROR(EAGAIN), because we read all
        // decoded frames with avcodec_receive_frame() until done.-
        if ( ret < 0 && ret != AVERROR_EOF )
        {
            char buf[128];
            av_strerror(ret, buf, 128);
            IMG_ERROR( "send_packet error: " << buf );
            return ret;
        }

        if ( ret == AVERROR_EOF ) eof = true;

    }

    ret = avcodec_receive_frame(avctx, frame);

    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF )
    {
        char buf[128];
        av_strerror(ret, buf, 128);
        IMG_ERROR( "receive_frame error: " << buf );
        return ret;
    }

    if ( ret == AVERROR_EOF ) eof = true;

    if (ret >= 0)
    {
        *got_frame = 1;
    }

    return 0;
}


CMedia::DecodeStatus
aviImage::decode_video_packet( int64_t& ptsframe,
                               const int64_t frame,
                               const AVPacket* p
                             )
{
    AVPacket* pkt = (AVPacket*)p;

    if ( pkt && _video_packets.is_jump( *pkt ) )
    {
        return kDecodeDone;
    }


    AVStream* stream = get_video_stream();

    assert0( stream != NULL );

    int got_pict = 0;

    bool eof_found = false;
    bool eof = false;

    if ( pkt && pkt->data == NULL ) {

        eof = true;
        pkt->size = 0;
    }



    while( !pkt || pkt->size > 0 || pkt->data == NULL )
    {
        int err = decode( _video_ctx, _av_frame, &got_pict, pkt, eof_found );


        if ( err < 0 ) {
            IMG_ERROR( "Decode video error: " << get_error_text(err) );
            return kDecodeError;
        }



        if ( got_pict ) {
            ptsframe = _av_frame->best_effort_timestamp;

            if ( pkt && ptsframe == AV_NOPTS_VALUE )
            {
                ptsframe = pkt->pts;
                if ( ptsframe == AV_NOPTS_VALUE )
                {
                    ptsframe = pkt->dts;
                }
            }


            // The following is a work around for bug in decoding
            // bgc.sub.dub.ogm
            if ( playback() != kStopped && pkt && pkt->dts != AV_NOPTS_VALUE &&
                    pkt->dts < ptsframe  )
            {
                ptsframe = pkt->dts;
            }

            if ( ptsframe == AV_NOPTS_VALUE )
            {
                ptsframe = _av_frame->pkt_pts;
            }


            // needed for some corrupt movies
            _av_frame->pts = ptsframe;


            // Turn PTS into a frame

            if ( pkt && ptsframe == AV_NOPTS_VALUE )
            {

                ptsframe = get_frame( stream, *p );
                if ( ptsframe == AV_NOPTS_VALUE ) ptsframe = frame;
            }
            else
            {

                ptsframe = pts2frame( stream, ptsframe ); // - _frame_offset;
            }



            if ( filter_graph && _subtitle_index >= 0 )
            {


                SCOPED_LOCK( _subtitle_mutex );
                /* push the decoded frame into the filtergraph */
                if (av_buffersrc_add_frame_flags(buffersrc_ctx, _av_frame,
                                                 AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                    LOG_ERROR( _("Error while feeding the filtergraph") );
                    close_subtitle_codec();
                    break;
                }

                int ret = av_buffersink_get_frame(buffersink_ctx, _filt_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                if (ret < 0)
                {
                    LOG_ERROR( "av_buffersink_get frame failed" );
                    close_subtitle_codec();
                    return kDecodeError;
                }

                av_frame_unref( _av_frame );
                _av_frame = av_frame_clone( _filt_frame );
                av_frame_unref( _filt_frame );
                if (!_av_frame )
                {
                    LOG_ERROR( _("Could not clone subtitle frame") );
                    close_subtitle_codec();
                    return kDecodeError;
                }
            }


            if ( eof )
            {

                eof_found = true;
                pkt->size = 0;
                pkt->data = NULL;
                store_image( ptsframe, _av_frame->pts + 1 );
                av_frame_unref( _av_frame );
                av_frame_unref( _filt_frame );
                continue;
            }


            return kDecodeOK;
        }

        if ( err == 0 ) {

            // If flushing caches, return done.
            if ( pkt && pkt->data == NULL ) return kDecodeDone;
            break;
        }

    }


    return kDecodeMissingFrame;
}



// Decode the image
CMedia::DecodeStatus
aviImage::decode_image( const int64_t frame, AVPacket& pkt )
{
    int64_t ptsframe = frame;
    DecodeStatus status = decode_video_packet( ptsframe, frame, &pkt );
    if ( status == kDecodeOK )
    {
        store_image( ptsframe, _av_frame->pts );

        do {
            status = decode_video_packet( ptsframe, frame, NULL );

            if ( status == kDecodeOK )
            {
                store_image( ptsframe, _av_frame->pts );
            }
        } while ( status == kDecodeOK );

        av_frame_unref(_av_frame);
        av_frame_unref(_filt_frame);
        if ( ( stopped() || saving() ) && ptsframe != frame &&
                frame != first_frame() )
            return kDecodeMissingFrame;
        return kDecodeOK;
    }
    else if ( status == kDecodeError )
    {
        char ftype = av_get_picture_type_char( _av_frame->pict_type );
        if ( ptsframe >= first_frame() && ptsframe <= last_frame() )
            IMG_WARNING( _("Could not decode video frame ") << ptsframe
                         << _(" type ") << ftype << " pts: "
                         << (pkt.pts == AV_NOPTS_VALUE ?
                             -1 : pkt.pts ) << " dts: " << pkt.dts
                         << " data: " << (void*)pkt.data);
        av_frame_unref(_av_frame);
        av_frame_unref(_filt_frame);
    }
    else
    {
        av_frame_unref(_av_frame);
        av_frame_unref(_filt_frame);
    }

    if ( status == kDecodeDone ) status = kDecodeOK;
    return status;
}

void aviImage::clear_packets()
{

#ifdef DEBUG_AUDIO_PACKETS
    cerr << "+++++++++++++ CLEAR VIDEO/AUDIO/SUBTITLE PACKETS " << _frame
         << " expected: " << _expected << endl;
#endif

    _video_packets.clear();
    _audio_packets.clear();
    _subtitle_packets.clear();
}

void aviImage::timed_limit_store( const int64_t& frame )
{
    uint64_t max_frames = max_video_frames();
    if ( _has_image_seq )
    {
        max_frames = max_image_frames();
    }

#undef timercmp
# define timercmp(a, b, CMP)					\
    (((a).tv_sec == (b).tv_sec) ?				\
     ((a).tv_usec CMP (b).tv_usec) :				\
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

    IteratorList::iterator i = iters.begin();
    IteratorList::iterator e = iters.end();

    // We erase from greater to lower to avoid dangling iterators
    std::sort( i, e, std::greater<video_cache_t::iterator>() );

    // Finally, remove the images with the iterators
    i = iters.begin();
    e = iters.end();
    for ( ; i != e; ++i )
    {
        _images.erase( *i );
    }

}


//
// Limit the video store to approx. max frames images on each side.
// We have to check both where frame is as well as where _dts is.
//
void aviImage::limit_video_store(const int64_t frame)
{
    SCOPED_LOCK( _mutex );

    int max_frames = max_video_frames();
    if ( _has_image_seq )
    {
        max_frames = max_image_frames();
    }

    return timed_limit_store( frame );

    // int64_t first, last;

    // switch( playback() )
    // {
    // case kBackwards:
    //     first = frame - max_frames;
    //     last  = frame + max_frames;
    //     if ( _dts > last )   last  = _dts;
    //     if ( _dts < first )  first = _dts;
    //     break;
    // case kForwards:
    //     first = frame - max_frames;
    //     last  = frame + max_frames;
    //     if ( _dts > last )   last  = _dts;
    //     if ( _dts < first )  first = _dts;
    //     break;
    // default:
    //     first = frame - max_frames;
    //     last  = frame + max_frames;
    //     if ( _dts > last )   last = _dts;
    //     if ( _dts < first ) first = _dts;
    //     break;
    // }

    // if ( _images.empty() ) return;

    // video_cache_t::iterator end = _images.end();
    // _images.erase( std::remove_if( _images.begin(), end,
    //                                NotInRangeFunctor( first, last ) ), end );


}

//
// Limit the subtitle store to approx. max frames images on each side.
// We have to check both where frame is as well as where _dts is.
//
void aviImage::limit_subtitle_store(const int64_t frame)
{

    int64_t first, last;

    switch( playback() )
    {
    case kBackwards:
        first = frame - (int64_t)fps() * 2;
        last  = frame;
        if ( _dts < first ) first = _dts;
        break;
    case kForwards:
        first = frame;
        last  = frame + (int64_t)fps() * 2;
        if ( _dts > last )   last = _dts;
        break;
    default:
        first = frame - (int64_t)fps() * 2;
        last  = frame + (int64_t)fps() * 2;
        break;
    }

    subtitle_cache_t::iterator end = _subtitles.end();
    _subtitles.erase( std::remove_if( _subtitles.begin(), end,
                                      NotInRangeFunctor( first, last ) ), end );



}

// Opens the subtitle codec associated to the current stream
void aviImage::open_subtitle_codec()
{

    AVStream* stream = get_subtitle_stream();
    if (!stream) return;

    AVCodecParameters* codecpar = stream->codecpar;
    AVCodec* subtitle_codec = avcodec_find_decoder( codecpar->codec_id );

    _subtitle_ctx = avcodec_alloc_context3(subtitle_codec);
    int r = avcodec_parameters_to_context(_subtitle_ctx, codecpar);
    if ( r < 0 )
    {
        LOG_ERROR( _("avcodec_copy_context failed for subtitle") );
        return;
    }

    static int workaround_bugs = 1;
    static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
    static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
    static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;
    static int error_concealment = 3;

    _subtitle_ctx->idct_algo         = FF_IDCT_AUTO;
    _subtitle_ctx->workaround_bugs = workaround_bugs;
    _subtitle_ctx->skip_frame= skip_frame;
    _subtitle_ctx->skip_idct= skip_idct;
    _subtitle_ctx->skip_loop_filter= skip_loop_filter;
    _subtitle_ctx->error_concealment= error_concealment;

    AVDictionary* info = NULL;
    if ( subtitle_codec == NULL ||
            avcodec_open2( _subtitle_ctx, subtitle_codec, &info ) < 0 )
        _subtitle_index = -1;
}

void aviImage::close_subtitle_codec()
{
    if ( _subtitle_ctx && _subtitle_index >= 0 )
    {
        avcodec_free_context( &_subtitle_ctx );
    }
}


bool aviImage::find_subtitle( const int64_t frame )
{

    SCOPED_LOCK( _subtitle_mutex );

    subtitle_cache_t::iterator end = _subtitles.end();
    subtitle_cache_t::iterator i = _subtitles.begin();

    _subtitle.reset();
    for ( ; i != end; ++i )
    {
        if ( frame >= (*i)->frame() && frame <= (*i)->frame() + (*i)->repeat() )
        {
            _subtitle = *i;
        }
    }


    image_damage( image_damage() | kDamageContents | kDamageSubtitle );

    limit_subtitle_store( frame );

    return false;
}

bool aviImage::find_image( const int64_t frame )
{

    if ( _right_eye && (playback() == kStopped || playback() == kSaving) )
        _right_eye->find_image( frame );


#ifdef DEBUG_VIDEO_PACKETS
    debug_video_packets(frame, "find_image");
#endif

#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "find_image", true);
#endif



    _frame = frame;

    if ( !has_video() )
    {
        _video_pts   = _frame  / _fps;
        _video_clock = double(av_gettime_relative()) / 1000000.0;

        update_video_pts(this, _video_pts, 0, 0);
        return true;
    }

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

                if ( !filter_graph &&
                        _hires->frame() != f &&
                        diff > 1 && diff < 10 && counter < 10 )
                {
                    ++counter;
                    IMG_WARNING( _("find_image: frame ") << frame
                                 << _(" not found, choosing ") << _hires->frame()
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


int aviImage::subtitle_stream_index() const
{
    assert( _subtitle_index >= 0 &&
            _subtitle_index < int(_subtitle_info.size()) );
    return _subtitle_info[ _subtitle_index ].stream_index;
}

/**
 * Change video stream
 *
 * @param x video stream number or -1 for no stream.
 */
void aviImage::video_stream( int x )
{
    if ( x < -1 || unsigned(x) >= number_of_video_streams() ) {
        IMG_ERROR( _("Invalid video stream ") << x );
        return;
    }

    if ( x == _video_index ) return;  // same stream, no change

    // mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
    // SCOPED_LOCK( vpm );

    if ( has_video() )
    {
        flush_video();
        close_video_codec();
        _video_packets.clear();
    }

    int old = _video_index;

    _video_index  = x;
    _num_channels = 0;
    if ( x < 0 ) return;

    std::vector<AVPixelFormat> fmt;
    fmt.reserve( 32 );
    if ( uses_16bits() )
    {
        fmt.push_back( AV_PIX_FMT_RGBA64LE );
        fmt.push_back( AV_PIX_FMT_BGRA64LE );
        fmt.push_back( AV_PIX_FMT_RGB48 );
        fmt.push_back( AV_PIX_FMT_BGR48 );
        fmt.push_back( AV_PIX_FMT_GRAY16LE );
        fmt.push_back( AV_PIX_FMT_YUV420P16LE );
        fmt.push_back( AV_PIX_FMT_YUV422P16LE );
        fmt.push_back( AV_PIX_FMT_YUV444P16LE );
        // fmt.push_back( AV_PIX_FMT_YUV420P10LE );
        // fmt.push_back( AV_PIX_FMT_YUV422P10LE );
        // fmt.push_back( AV_PIX_FMT_YUV444P10LE );
    }

    fmt.push_back( AV_PIX_FMT_BGR32 );
    fmt.push_back( AV_PIX_FMT_BGR24 );
    fmt.push_back( AV_PIX_FMT_RGB32 );
    fmt.push_back( AV_PIX_FMT_RGB24 );
    fmt.push_back( AV_PIX_FMT_NONE );

    AVPixelFormat* fmts = &fmt[0];

    if ( supports_yuva() )
    {
        fmt.clear();
        if ( uses_16bits() )
        {
            fmt.push_back( AV_PIX_FMT_RGBA64LE );
            fmt.push_back( AV_PIX_FMT_BGRA64LE );
            fmt.push_back( AV_PIX_FMT_RGB48 );
            fmt.push_back( AV_PIX_FMT_BGR48 );
            fmt.push_back( AV_PIX_FMT_GRAY16LE );
            fmt.push_back( AV_PIX_FMT_YUV420P16LE );
            fmt.push_back( AV_PIX_FMT_YUV422P16LE );
            fmt.push_back( AV_PIX_FMT_YUV444P16LE );
            // fmt.push_back( AV_PIX_FMT_YUV420P10LE );
            // fmt.push_back( AV_PIX_FMT_YUV422P10LE );
            // fmt.push_back( AV_PIX_FMT_YUV444P10LE );
        }
        fmt.push_back( AV_PIX_FMT_RGB32 );
        fmt.push_back( AV_PIX_FMT_BGR32 );
        fmt.push_back( AV_PIX_FMT_BGR24 );
        fmt.push_back( AV_PIX_FMT_RGB24 );
        fmt.push_back( AV_PIX_FMT_YUVA444P );
        fmt.push_back( AV_PIX_FMT_YUVA422P );
        fmt.push_back( AV_PIX_FMT_YUVA420P );
        fmt.push_back( AV_PIX_FMT_YUV444P );
        fmt.push_back( AV_PIX_FMT_YUV422P );
        fmt.push_back( AV_PIX_FMT_YUV420P );
        fmt.push_back( AV_PIX_FMT_YUVJ444P );
        fmt.push_back( AV_PIX_FMT_YUVJ422P );
        fmt.push_back( AV_PIX_FMT_YUVJ420P );
        fmt.push_back( AV_PIX_FMT_NONE );
        fmts = &fmt[0];
    }
    else if ( supports_yuv() )
    {

        fmt.clear();
        if ( uses_16bits() )
        {
            fmt.push_back( AV_PIX_FMT_RGB48 );
            fmt.push_back( AV_PIX_FMT_BGR48 );
            fmt.push_back( AV_PIX_FMT_GRAY16LE );
            fmt.push_back( AV_PIX_FMT_YUV420P16LE );
            fmt.push_back( AV_PIX_FMT_YUV422P16LE );
            fmt.push_back( AV_PIX_FMT_YUV444P16LE );
            // fmt.push_back( AV_PIX_FMT_YUV420P10LE );
            // fmt.push_back( AV_PIX_FMT_YUV422P10LE );
            // fmt.push_back( AV_PIX_FMT_YUV444P10LE );
        }
        fmt.push_back( AV_PIX_FMT_RGB32 );
        fmt.push_back( AV_PIX_FMT_BGR32 );
        fmt.push_back( AV_PIX_FMT_BGR24 );
        fmt.push_back( AV_PIX_FMT_RGB24 );
        fmt.push_back( AV_PIX_FMT_YUV444P );
        fmt.push_back( AV_PIX_FMT_YUV422P );
        fmt.push_back( AV_PIX_FMT_YUV420P );
        fmt.push_back( AV_PIX_FMT_YUVJ444P );
        fmt.push_back( AV_PIX_FMT_YUVJ422P );
        fmt.push_back( AV_PIX_FMT_YUVJ420P );
        fmt.push_back( AV_PIX_FMT_NONE );
        fmts = &fmt[0];
    }

    AVStream* stream = get_video_stream();
    AVCodecParameters* ctx = stream->codecpar;

    int has_alpha = ( ( ctx->format == AV_PIX_FMT_RGBA    ) |
                      ( ctx->format == AV_PIX_FMT_ABGR    ) |
                      ( ctx->format == AV_PIX_FMT_GBRAP   ) |
                      ( ctx->format == AV_PIX_FMT_GBRAP16BE ) |
                      ( ctx->format == AV_PIX_FMT_GBRAP16LE ) |
                      ( ctx->format == AV_PIX_FMT_RGBA64BE ) |
                      ( ctx->format == AV_PIX_FMT_BGRA64BE ) |
                      ( ctx->format == AV_PIX_FMT_RGBA64LE ) |
                      ( ctx->format == AV_PIX_FMT_BGRA64LE ) |
                      ( ctx->format == AV_PIX_FMT_ARGB    ) |
                      ( ctx->format == AV_PIX_FMT_RGB32   ) |
                      ( ctx->format == AV_PIX_FMT_RGB32_1 ) |
                      ( ctx->format == AV_PIX_FMT_PAL8    ) |
                      ( ctx->format == AV_PIX_FMT_BGR32   ) |
                      ( ctx->format == AV_PIX_FMT_BGR32_1 ) |
                      ( ctx->format == AV_PIX_FMT_YUVA420P ) |
                      ( ctx->format == AV_PIX_FMT_YUVA422P ) |
                      ( ctx->format == AV_PIX_FMT_YUVA444P ) |
                      ( ctx->format == AV_PIX_FMT_YUVA420P9BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA422P9BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA444P9BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA420P9LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA422P9LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA444P9LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA420P10LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA422P10LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA444P10LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA420P10BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA422P10BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA444P10BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA420P16LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA422P16LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA444P16LE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA420P16BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA422P16BE ) |
                      ( ctx->format == AV_PIX_FMT_YUVA444P16BE )  );


    _av_dst_pix_fmt = avcodec_find_best_pix_fmt_of_list( fmts,
                      (AVPixelFormat)
                      ctx->format,
                      has_alpha, NULL );


    _num_channels = 0;
    _layers.clear();

    rgb_layers();
    lumma_layers();

    if ( _av_dst_pix_fmt == AV_PIX_FMT_RGBA ||
            _av_dst_pix_fmt == AV_PIX_FMT_BGRA ||
            _av_dst_pix_fmt == AV_PIX_FMT_YUVA420P16LE ||
            _av_dst_pix_fmt == AV_PIX_FMT_YUVA422P16LE ||
            _av_dst_pix_fmt == AV_PIX_FMT_YUVA444P16LE ||
            _av_dst_pix_fmt == AV_PIX_FMT_YUVA420P ||
            _av_dst_pix_fmt == AV_PIX_FMT_YUVA422P ||
            _av_dst_pix_fmt == AV_PIX_FMT_YUVA444P ) alpha_layers();

    // if (ctx->lowres) {
    //     ctx->flags |= CODEC_FLAG_EMU_EDGE;
    // }

    _ptype = VideoFrame::kByte;

    if ( colorspace_override ) _colorspace_index = colorspace_override;
    else _colorspace_index = ctx->color_space;

    switch( _av_dst_pix_fmt )
    {
    case AV_PIX_FMT_RGBA64BE:
    case AV_PIX_FMT_RGBA64LE:
        _ptype = VideoFrame::kShort;
        _pix_fmt = VideoFrame::kRGBA;
        break;
    case AV_PIX_FMT_BGRA64BE:
    case AV_PIX_FMT_BGRA64LE:
        _ptype = VideoFrame::kShort;
        _pix_fmt = VideoFrame::kBGRA;
        break;
    case AV_PIX_FMT_RGB48:
        _ptype = VideoFrame::kShort;
        _pix_fmt = VideoFrame::kRGB;
        break;
    case AV_PIX_FMT_BGR48:
        _ptype = VideoFrame::kShort;
        _pix_fmt = VideoFrame::kBGRA;
        break;
    case AV_PIX_FMT_BGR24:
        _pix_fmt = VideoFrame::kBGR;
        break;
    case AV_PIX_FMT_BGRA:
        _pix_fmt = VideoFrame::kBGRA;
        break;
    case AV_PIX_FMT_RGB24:
        _pix_fmt = VideoFrame::kRGB;
        break;
    case AV_PIX_FMT_RGBA:
        _pix_fmt = VideoFrame::kRGBA;
        break;
    case AV_PIX_FMT_YUV444P16LE:
        _ptype = VideoFrame::kShort;
    case AV_PIX_FMT_YUV444P:
    case AV_PIX_FMT_YUVJ444P:
        if ( _colorspace_index == AVCOL_SPC_BT709 ||
                ( ctx->height >= 630 && ctx->width >= 1120 ) )
            _pix_fmt = VideoFrame::kITU_709_YCbCr444;
        else
            _pix_fmt = VideoFrame::kITU_601_YCbCr444;
        break;
    case AV_PIX_FMT_YUV422P16LE:
        _ptype = VideoFrame::kShort;
    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
        if ( _colorspace_index == AVCOL_SPC_BT709 ||
                ( ctx->height >= 630 && ctx->width >= 1120 )  )
            _pix_fmt = VideoFrame::kITU_709_YCbCr422;
        else
            _pix_fmt = VideoFrame::kITU_601_YCbCr422;
        break;
    case AV_PIX_FMT_YUV420P16LE:
        _ptype = VideoFrame::kShort;
    case AV_PIX_FMT_YUV420P:
    case AV_PIX_FMT_YUVJ420P:
        if ( _colorspace_index == AVCOL_SPC_BT709 ||
                ( ctx->height >= 630 && ctx->width >= 1120 ) )
            _pix_fmt = VideoFrame::kITU_709_YCbCr420;
        else
            _pix_fmt = VideoFrame::kITU_601_YCbCr420;
        break;
    case AV_PIX_FMT_YUVA420P:
        if ( _colorspace_index == AVCOL_SPC_BT709 || ctx->height > 525  )
            _pix_fmt = VideoFrame::kITU_709_YCbCr420A;
        else
            _pix_fmt = VideoFrame::kITU_601_YCbCr420A;
        break;
    default:
        IMG_ERROR( _("Unknown destination video frame format: ")
                   << _av_dst_pix_fmt << " "
                   << av_get_pix_fmt_name( _av_dst_pix_fmt ) );
        _pix_fmt = VideoFrame::kBGRA;
        break;
    }


    if ( old >= 0 )
    {
        open_video_codec();

        {
            SCOPED_LOCK( _mutex );

            if ( stopped() ) _images.clear();
            else
            {
                // Keep the current frame which won't get replaced
                video_cache_t::iterator i = _images.begin();
                for ( ; i != _images.end(); )
                {
                    if ( (*i)->frame() == _frame ) {
                        ++i;
                        continue;
                    }
                    i = _images.erase( i );
                }
                assert( _images.size() == 1 );
            }

            _dts = _frame - 1;
            _expected = _frame-2;
        }
        int64_t f = _frame;
        seek( f );
    }
}

bool aviImage::readFrame(int64_t & pts)
{
    AVPacket packet;

    int got_video = 0;

    while (! got_video)
    {
        int r = av_read_frame(_context, &packet);
        AVPacket* pkt = &packet;
        if (r < 0)
        {
            pkt->size = 0;
            pkt->data = NULL;
        }
        bool eof = false;
        if ( video_stream_index() == packet.stream_index)
        {
            if (decode( _video_ctx, _av_frame, &got_video, pkt, eof ) <= 0)
            {
                break;
            }
        }
    }

    pts = _av_frame->best_effort_timestamp;

    if ( pts == AV_NOPTS_VALUE )
    {
        pts = _av_frame->pkt_dts;
    }

    AVRational q = { 1, AV_TIME_BASE };

    pts = av_rescale_q( pts,
                        get_video_stream()->time_base,
                        q );
    return got_video;
}

int aviImage::video_stream_index() const
{
    assert( _video_index >= 0 &&
            _video_index < int(_video_info.size()) );
    return _video_info[ _video_index ].stream_index;
}


// Analyse streams and set input values
void aviImage::populate()
{
    std::ostringstream msg;

    if ( _context == NULL ) return;

    // Iterate through all the streams available
    for( unsigned i = 0; i < _context->nb_streams; ++i )
    {
        // Get the codec context
        const AVStream* stream = _context->streams[ i ];

        if ( stream == NULL ) continue;

        const AVCodecParameters* par = stream->codecpar;
        if ( par == NULL ) continue;

        AVCodecContext* ctx;

        AVCodec* codec = avcodec_find_decoder( par->codec_id );
        ctx = avcodec_alloc_context3( codec );
        int err = avcodec_parameters_to_context( ctx, par );
        if ( err < 0 )
        {
            LOG_ERROR( _("Could not copy parameters to context") );
        }

        // Determine the type and obtain the first index of each type
        switch( ctx->codec_type )
        {
        // We ignore attachments for now.
        case AVMEDIA_TYPE_ATTACHMENT:
        {
            continue;
        }
        // We ignore data tracks for now.  Data tracks are, for example,
        // the timecode track in quicktimes.
        case AVMEDIA_TYPE_DATA:
        {
            continue;
        }
        case AVMEDIA_TYPE_VIDEO:
        {
            video_info_t s;
            populate_stream_info( s, msg, _context, ctx, i );
            s.has_b_frames = ( ctx->has_b_frames != 0 );
            s.fps          = calculate_fps( stream );
            if ( av_get_pix_fmt_name( ctx->pix_fmt ) )
                s.pixel_format = av_get_pix_fmt_name( ctx->pix_fmt );
            _video_info.push_back( s );
            if ( _video_index < 0 && s.has_codec )
            {
                video_stream( _video_info.size()-1 );
                int w = ctx->width;
                int h = ctx->height;
                image_size( w, h );
            }
            else if ( _video_info.size() == 2 )
            {
                static std::string file;
                double diff1 = fabs( _video_info[0].fps - s.fps );
                double diff2 = fabs( _video_info[0].duration -
                                     s.duration );
                if ( !_is_thumbnail && file != filename() &&
                        _w == ctx->width && _h == ctx->height &&
                        diff1 <= 0.0001 && diff2 <= 0.0001 )
                {
                    _is_stereo = true;
                    _is_left_eye = true;
                    file = filename();
                    _right_eye = CMedia::guess_image( filename(),
                                                      NULL, 0, true );
                    _right_eye->is_stereo( true );
                    _right_eye->is_left_eye( false );
                    _right_eye->video_stream( 1 );
                    _right_eye->audio_stream( -1 );
                }
            }
            break;
        }
        case AVMEDIA_TYPE_AUDIO:
        {

            audio_info_t s;
            populate_stream_info( s, msg, _context, ctx, i );

            s.channels   = ctx->channels;
            s.frequency  = ctx->sample_rate;
            s.bitrate    = calculate_bitrate( stream, par );


            const char* fmt = av_get_sample_fmt_name( ctx->sample_fmt );
            if ( fmt ) s.format = fmt;

            _audio_info.push_back( s );
            if ( _audio_index < 0 && s.has_codec )
                _audio_index = _audio_info.size()-1;
            break;
        }
        case AVMEDIA_TYPE_SUBTITLE:
        {
            subtitle_info_t s;
            populate_stream_info( s, msg, _context, ctx, i );
            s.bitrate    = calculate_bitrate( stream, par );
            _subtitle_info.push_back( s );
            if ( _subtitle_index < 0 )
                _subtitle_index = _subtitle_info.size()-1;
            break;
        }
        default:
        {
            const char* stream = stream_type( ctx );
            msg << _("\n\nNot a known stream type for stream #")
                << i << (", type ") << stream;
            break;
        }
        }

        avcodec_free_context( &ctx );
    }


    if ( msg.str().size() > 0 )
    {
        LOG_ERROR( filename() << msg.str() );
    }

    if ( _video_index < 0 && _audio_index < 0 )
    {
        LOG_ERROR( filename() << _(" No audio or video stream in file") );
        return;
    }

    // Open these video and audio codecs
    if ( has_video() )
        open_video_codec();
    if ( has_audio() )
        open_audio_codec();
    if ( has_subtitle() )
        open_subtitle_codec();


    // Configure video input properties
    AVStream* stream = NULL;

    if ( has_video() )
    {
        stream = get_video_stream();
    }
    else if ( has_audio() )
    {
        stream = get_audio_stream();
    }
    else
    {
        return;  // no stream detected
    }

    _orig_fps = _fps = _play_fps = calculate_fps( stream );



#ifdef DEBUG_STREAM_INDICES
    debug_stream_index( stream );
#elif defined(DEBUG_STREAM_KEYFRAMES)
    debug_stream_keyframes( stream );
#endif


    //
    // Calculate frame start and frame end if possible
    //

    _frameStart = 1;

    if ( _context->start_time != AV_NOPTS_VALUE )
    {
        _frameStart = int64_t( ( _fps *
                                 ( double )_context->start_time /
                                 ( double )AV_TIME_BASE ) + 1);
    }
    else
    {
        double start = std::numeric_limits< double >::max();
        if ( has_video() )
        {
            start = _video_info[ _video_index ].start;
        }

        if ( has_audio() )
        {
            double d = _audio_info[ _audio_index ].start;
            if ( d < start ) start = d;
        }

        if ( start != std::numeric_limits< double >::max() )
        {
            _frameStart = (int64_t)start;
            if ( _frameStart == 0 ) _frameStart = 1;
        }
    }




    _frame_start = _frame = _frameEnd = _frameStart + _start_number;


    //
    // BUG FIX for ffmpeg bugs with some codecs/containers.
    //
    // using context->start_time and context->duration should be enough,
    // but with that ffmpeg often reports a frame that cannot be decoded
    // with some codecs like h264.
    int64_t duration = 0;

    if ( 0 ) //_context->duration != AV_NOPTS_VALUE && _context->duration > 0 )
    {
        duration = _context->duration;

        // int hours, mins, secs, us;
        // secs  = duration / AV_TIME_BASE;
        // us    = duration % AV_TIME_BASE;
        // mins  = secs / 60;
        // secs %= 60;
        // hours = mins / 60;
        // mins %= 60;

        // duration = int64_t( _fps * ( hours*3600 + mins * 60 + secs ) +
        //                     _fps * (double) us / AV_TIME_BASE );

        duration = int64_t( ( double )_fps * ( double )duration /
                            ( double ) AV_TIME_BASE + 0.5 );
    }
    else
    {
        if ( stream->nb_frames > 0 )
        {
            duration = stream->nb_frames;
        }
        else
        {
            double length = 0;

            if ( has_video() )
            {
                length = _video_info[ _video_index ].duration;
            }

            if ( has_audio() )
            {
                double d = _audio_info[ _audio_index ].duration;
                if ( d > length ) length = d;
            }

            if ( length > 0 )
            {
                duration = int64_t( length * _fps + 0.5 );
            }
            else
            {
                // As a last resort, count the frames manually.
                int64_t pts = 0;

                if ( fileroot() == filename() )
                {
                    duration = 0; // GIF89
                    while ( readFrame(pts) )
                        ++duration;

                    flush_video();
                    av_seek_frame( _context,
                                   video_stream_index(),
                                   0,
                                   AVSEEK_FLAG_BACKWARD);
                }
                else
                {
                    duration = _frameEnd - _frameStart + 1;
                }
            }
        }
    }

    if ( duration <= 0 ) duration = 1;

    _frameEnd = _frameStart + duration - 1;
    _frame_end = _frame_start + duration - 1;

    _frame_offset = 0;

    // if ( _pix_fmt == VideoFrame::kITU_709_YCbCr420  ||
    //   _pix_fmt == VideoFrame::kITU_709_YCbCr420A ||
    //   _pix_fmt == VideoFrame::kITU_709_YCbCr422  ||
    //   _pix_fmt == VideoFrame::kITU_709_YCbCr422A ||
    //   _pix_fmt == VideoFrame::kITU_709_YCbCr444  ||
    //   _pix_fmt == VideoFrame::kITU_709_YCbCr444A  )
    // {
    //  _inv_table = sws_getCoefficients( SWS_CS_ITU709 );
    // }

    // We get this here for timecode and for color space
    dump_metadata( _context->metadata );

    Attributes::const_iterator i = _attrs.begin();
    Attributes::const_iterator e = _attrs.end();
    for ( ; i != e; ++i )
    {
        if ( i->first.find( "YCbCrMatrix" ) != std::string::npos  )
        {
            Imf::Attribute* attr = i->second;
            Imf::StringAttribute* str;
            if ( str = dynamic_cast< Imf::StringAttribute* >( attr ) )
            {
                std::string outcol = str->value();
                if ( outcol == "Rec 709" || outcol == "ITU 709" ||
                        outcol == "BT709" )
                {
                    _inv_table = sws_getCoefficients( SWS_CS_ITU709 );
                }
                else if ( outcol == "Rec 601" || outcol == "ITU 601" ||
                          outcol == "BT601"  )
                {
                    _inv_table = sws_getCoefficients( SWS_CS_ITU601 );
                }
                else if ( outcol == "Rec 2020" || outcol == "ITU 2020" ||
                          outcol == "BT2020"   )
                {
                    _inv_table = sws_getCoefficients( SWS_CS_BT2020 );
                }
                else if ( outcol == "SMPTE 240M" || outcol == "SMPTE240M"  )
                {
                    _inv_table = sws_getCoefficients( SWS_CS_SMPTE240M );
                }
                else if ( outcol == "FCC" )
                {
                    _inv_table = sws_getCoefficients( SWS_CS_FCC );
                }
            }
        }
    }

    char buf[128];

    for (unsigned i = 0; i < _context->nb_chapters; ++i)
    {
        AVChapter *ch = _context->chapters[i];
        sprintf( buf, "Chapter %d ", i+1 );
        dump_metadata(ch->metadata, buf);
    }

    for (unsigned i = 0; i < _context->nb_programs; ++i)
    {
        AVDictionaryEntry* tag =
            av_dict_get(_context->programs[i]->metadata,
                        "name", NULL, 0);
        if ( tag )
        {
            sprintf( buf, _("Program %d: %s"), i+1, tag->key );
            Imf::StringAttribute* value = new Imf::StringAttribute( tag->value );
            _attrs.insert( std::make_pair(buf, value) );
        }
        sprintf( buf, _("Program %d "), i+1 );
        dump_metadata( _context->programs[i]->metadata, buf );

        //dump_metadata( pkt->side_data, "" );
    }


    int64_t dts = _frameStart;

    unsigned audio_bytes = 0;
    unsigned bytes_per_frame = audio_bytes_per_frame();

    if ( has_video() || has_audio() )
    {

        // Loop until we get first frame
        AVPacket pkt = {0};
        // Clear the packet
        av_init_packet( &pkt );
        pkt.size = 0;
        pkt.data = NULL;

        int force_exit = 0;
        bool eof = false;
        short counter = 0;
        bool got_audio = ! has_audio();
        bool got_video = ! has_video();
        while( !got_video || !got_audio )
        {
            // Hack to exit loop if got_video or got_audio fails
            ++force_exit;
            if ( force_exit == 200 )  break;

            int error = av_read_frame( _context, &pkt );
            if ( error < 0 )
            {
                int err = _context->pb ? _context->pb->error : 0;
                if ( err != 0 )
                {
                    char buf[128];
                    av_strerror(err, buf, 128);
                    IMG_ERROR("populate: Could not read frame 1 error: "
                              << buf );
                    break;
                }
            }

            if ( has_video() && pkt.stream_index == video_stream_index() )
            {
                if ( !got_video )
                {
                    DecodeStatus status = decode_image( _frameStart, pkt );
                    if ( status == kDecodeOK )
                    {
                        got_video = true;
                    }
                    else
                    {
                        ++_frame_offset;
                    }
                }
                else
                {
                    _video_packets.push_back( pkt );
                    continue;
                }
            }
            else if ( has_audio() && pkt.stream_index == audio_stream_index() )
            {
                int64_t pktframe = get_frame( get_audio_stream(),
                                              pkt ) - _frame_offset;
                _adts = pktframe;

                if ( playback() == kBackwards )
                {
                    // Only add packet if it comes before first frame
                    if ( pktframe >= first_frame() )
                        _audio_packets.push_back( pkt );
                    if ( !has_video() && pktframe < dts ) dts = pktframe;
                }
                else
                {
                    _audio_packets.push_back( pkt );
                    if ( !has_video() && pktframe > dts ) dts = pktframe;
                }

                if ( !got_audio )
                {
                    if ( pktframe > _frameStart ) got_audio = true;
                    else if ( pktframe == _frameStart )
                    {
                        audio_bytes += pkt.size;
                        if ( audio_bytes >= bytes_per_frame )
                            got_audio = true;
                    }
                }

                if ( !has_video() )
                {
                    AVPacket pkt;
                    av_init_packet( &pkt );
                    pkt.size = 0;
                    pkt.data = NULL;
                    pkt.dts = pkt.pts = _dts;
                    _video_packets.push_back( pkt );
                }

#ifdef DEBUG_DECODE_POP_AUDIO
                fprintf( stderr, "\t[avi]POP. A f: %05" PRId64 " audio pts: %07" PRId64
                         " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
                         pktframe, pkt.pts, pkt.dts, pktframe );
#endif
                continue;
            }

            av_packet_unref( &pkt );
        }


        if ( got_video && (!has_audio() || audio_context() == _context) )
        {
            find_image( _frameStart );
        }
    }

    _dts = dts;
    _frame = _audio_frame = _frameStart;
    _expected = dts + 1;
    _expected_audio = _adts + 1;

    //if ( _frame_offset > 3 ) _frame_offset = 0;

    if ( !has_video() )
    {
        mrv::image_type_ptr canvas;
        if ( !_hires )
        {
            _w = 640;
            _h = 480;
            allocate_pixels( canvas, _frameStart, 3, image_type::kRGB,
                             image_type::kByte );
            rgb_layers();
        }
        canvas->frame( _frameStart );
        uint8_t* ptr = (uint8_t*) canvas->data().get();
        memset( ptr, 0, 3*_w*_h*sizeof(uint8_t));
    }

    //
    // Format
    //
    if ( _context->iformat )
        _format = _context->iformat->name;
    else
        _format = _("Unknown");

    //
    // Miscellaneous information
    //



    if ( has_audio() )
    {
        AVStream* stream = get_audio_stream();
        if ( stream->metadata ) dump_metadata( stream->metadata, N_("Audio ") );
    }

    if ( has_video() )
    {
        AVStream* stream = get_video_stream();
        if ( stream->metadata ) dump_metadata( stream->metadata, N_("Video ") );

    }

    default_ocio_input_color_space();

}

void aviImage::probe_size( unsigned p )
{
    if ( !_context ) return;
    _context->probesize = p;
}

bool aviImage::initialize()
{
    if ( !_initialize )
    {
        avfilter_register_all();


        AVDictionary *opts = NULL;
        av_dict_set(&opts, "initial_pause", "1", 0);

        std::string ext = name();

        std::transform( ext.begin(), ext.end(), ext.begin(),
                        (int(*)(int)) tolower);

        if ( ext.rfind( ".png" )  != std::string::npos ||
                ext.rfind( ".dpx" )  != std::string::npos ||
                ext.rfind( ".jpg" )  != std::string::npos ||
                ext.rfind( ".jpeg" ) != std::string::npos )
        {
            char buf[64];
            sprintf( buf, "%" PRId64, _frameStart );
            _start_number = _frameStart - 1;
            _has_image_seq = _is_sequence = true;

            av_dict_set(&opts, "start_number", buf, 0);

            if ( CMedia::load_library != CMedia::kFFMPEGLibrary ) return false;

        }

        AVInputFormat*     format = NULL;
        // We must open fileroot for png sequences to work
        int error = avformat_open_input( &_context, fileroot(),
                                         format, &opts );

        if ( error >= 0 )
        {
            // Change probesize and analyze duration to 30 secs
            // to detect subtitles.
            if ( _context )
            {
                probe_size( 30 * AV_TIME_BASE );
            }
            error = avformat_find_stream_info( _context, NULL );
        }

        if ( error >= 0 )
        {

            // Allocate an av frame
            _av_frame = av_frame_alloc();
            populate();
            _initialize = true;
        }
        else
        {
            LOG_ERROR( filename() << _(" Could not open file") );
            avformat_free_context( _context );
            _initialize = false;
            _context = NULL;
            return false;
        }
    }

    return true;
}

void aviImage::preroll( const int64_t frame )
{
    _dts = _adts = _frame = _audio_frame = frame;

    _images.reserve( _max_images );

}


int64_t aviImage::queue_packets( const int64_t frame,
                                 const bool is_seek,
                                 bool& got_video,
                                 bool& got_audio,
                                 bool& got_subtitle )
{

    int64_t dts = frame;

    int64_t vpts, apts, spts;

    if ( !got_video ) {
        vpts = frame2pts( get_video_stream(), frame );
    }

    if ( !got_audio ) {
        if ( _acontext )
        {
            assert( get_audio_stream() != NULL );
            apts = frame2pts( get_audio_stream(), frame + _audio_offset );
        }
        else
        {
            assert( get_audio_stream() != NULL );
            apts = frame2pts( get_audio_stream(), frame );
        }
    }

    if ( !got_subtitle ) {
        spts = frame2pts( get_subtitle_stream(), frame );
    }

    if ( vpts < 0 ) vpts = 0;
    if ( apts < 0 ) apts = 0;
    if ( spts < 0 ) spts = 0;

    AVPacket pkt = {0};

    // Clear the packet
    av_init_packet( &pkt );
    pkt.size = 0;
    pkt.data = NULL;

    unsigned int bytes_per_frame = audio_bytes_per_frame();
    unsigned int audio_bytes = 0;

    bool eof = false;


    // Loop until an error or we have what we need
    while( !got_video || (!got_audio && audio_context() == _context) )
    {

        if (eof) {
            if (!got_video && video_stream_index() >= 0) {
                av_init_packet(&pkt);
                pkt.size = 0;
                pkt.data = NULL;
                pkt.stream_index = video_stream_index();
                _video_packets.push_back( pkt );
                got_video = true;
                got_subtitle = true;
                if ( is_seek || playback() == kBackwards )
                {
                    _video_packets.seek_end(vpts);
                }
            }

            AVStream* stream = get_audio_stream();
            if (!got_audio )
            {
                if (audio_context() == _context && _audio_ctx &&
                        _audio_ctx->codec->capabilities & AV_CODEC_CAP_DELAY) {
                    av_init_packet(&pkt);
                    pkt.size = 0;
                    pkt.data = NULL;
                    pkt.stream_index = audio_stream_index();
                    _audio_packets.push_back( pkt );
                }

                got_audio = true;

                if ( is_seek || playback() == kBackwards )
                {
                    _audio_packets.seek_end(apts);
                }
            }

            if ( !got_subtitle && ( is_seek || playback() == kBackwards ) )
            {
                _subtitle_packets.seek_end(spts);
            }

            eof = false;
            break;
        }

        int error = av_read_frame( _context, &pkt );

        if ( error < 0 )
        {
            if ( error == AVERROR_EOF )
            {
                eof = true;
                continue;
            }
            int err = _context->pb ? _context->pb->error : 0;
            if ( err != 0 )
            {
                char buf[128];
                av_strerror(err, buf, 128);
                IMG_ERROR("fetch: Could not read frame " << frame << " error: "
                          << buf );
            }

            if ( is_seek )
            {
                if ( !got_video )    _video_packets.seek_end(vpts);
                if ( !got_audio && apts >= 0 ) _audio_packets.seek_end(apts);
                if ( !got_subtitle ) _subtitle_packets.seek_end(spts);
            }


            av_packet_unref( &pkt );

            break;
        }


        if ( has_video() && pkt.stream_index == video_stream_index() )
        {
            int64_t pktframe = pts2frame( get_video_stream(), pkt.dts )                                      - _frame_offset + _start_number; // needed

            if ( playback() == kBackwards )
            {
                if ( pktframe <= frame )
                {
                    _video_packets.push_back( pkt );
                }
                // should be pktframe without +1 but it works better with it.
                if ( pktframe < dts ) dts = pktframe + 1;
            }
            else
            {
                // std::cerr << "push back pkt "
                //           << get_frame( get_video_stream(), pkt )
                //           << std::endl;
                _video_packets.push_back( pkt );
                if ( pktframe > dts ) dts = pktframe;
            }

            if ( !got_video && pktframe >= frame )
            {
                got_video = true;
                if ( is_seek ) _video_packets.seek_end(vpts);
            }
#ifdef DEBUG_DECODE
            char ftype = av_get_picture_type_char(_av_frame->pict_type );
            fprintf( stderr, "\t[avi] FETCH V f: %05" PRId64
                     " video pts: %07" PRId64
                     " dts: %07" PRId64 " %c as frame: %05" PRId64 "\n",
                     frame, pkt.pts, pkt.dts, ftype, pktframe );
#endif
            continue;
        }
        else if ( has_subtitle()  &&
                  pkt.stream_index == subtitle_stream_index() )
        {
            int64_t pktframe = get_frame( get_subtitle_stream(), pkt );
            if ( playback() == kBackwards )
            {
                if ( pktframe <= frame )
                    _subtitle_packets.push_back( pkt );
            }
            else
            {
                _subtitle_packets.push_back( pkt );
            }

            if ( !got_subtitle && pktframe >= frame )
            {
                got_subtitle = true;
                if ( is_seek ) _subtitle_packets.seek_end(spts);
            }
            continue;
        }
        else
        {

            if ( has_audio() && audio_context() == _context &&
                    pkt.stream_index == audio_stream_index() )
            {
                int64_t pktframe = pts2frame( get_audio_stream(), pkt.dts )                                      - _frame_offset; // needed
                _adts = pktframe;

                if ( playback() == kBackwards )
                {
                    // Only add packet if it comes before seek frame
                    //if ( pktframe <= frame )
                    _audio_packets.push_back( pkt );
                    if ( !has_video() && pktframe < dts ) dts = pktframe;
                }
                else
                {
                    // ffmpeg @bug:  audio seeks in long mp3s while playing can
                    // result in ffmpeg going backwards too far.
                    // This pktframe >= frame-10 is to avoid that.
                    _audio_packets.push_back( pkt );
                    if ( !has_video() && pktframe > dts ) dts = pktframe;
                }

                if ( !got_audio )
                {
                    if ( pktframe > frame ) got_audio = true;
                    else if ( pktframe == frame )
                    {
                        audio_bytes += pkt.size;
                        if ( audio_bytes >= bytes_per_frame ) got_audio = true;
                    }
                    if ( got_audio && !has_video() )
                    {
                        for (int64_t t = frame; t <= pktframe; ++t )
                        {
                            AVPacket pkt;
                            av_init_packet( &pkt );
                            pkt.size = 0;
                            pkt.data = NULL;
                            pkt.dts = pkt.pts = t;
                            _video_packets.push_back( pkt );
                        }
                    }

                    if ( is_seek && got_audio ) {
                        if ( !has_video() ) _video_packets.seek_end(vpts);
                        _audio_packets.seek_end(apts);
                    }
                }
#ifdef DEBUG_DECODE_AUDIO
                fprintf( stderr, "\t[avi] FETCH A f: %05" PRId64
                         " audio pts: %07" PRId64
                         " dts: %07" PRId64 "   as frame: %05" PRId64 "\n",
                         frame, pkt.pts, pkt.dts, pktframe );
#endif
                continue;
            }
        }

        av_packet_unref( &pkt );


    } // (!got_video || !got_audio)

    // For secondary audio
    if ( _acontext )
    {
        _adts = CMedia::queue_packets( frame + _audio_offset,
                                       is_seek,
                                       got_video, got_audio, got_subtitle );
        _expected_audio = _adts + 1;
    }


    if ( dts > last_frame() ) dts = last_frame();
    else if ( dts < first_frame() ) dts = first_frame();

    return dts;
}



bool aviImage::fetch(mrv::image_type_ptr& canvas, const int64_t frame)
{
#ifdef DEBUG_DECODE
    cerr << "FETCH BEGIN: " << frame << " EXPECTED: " << _expected
         << " DTS: " << _dts << endl;
#endif


    if ( _right_eye && (playback() == kStopped || playback() == kSaving) )
    {
        _right_eye->stop();
        mrv::image_type_ptr canvas;
        _right_eye->fetch( canvas, frame );
        _stereo[1] = canvas;
    }

    bool got_video = !has_video();
    bool got_audio = !has_audio();
    bool got_subtitle = !has_subtitle();

    int64_t f = handle_loops( frame );


    if ( ( got_audio || in_audio_store( f + _audio_offset ) ) &&
            in_video_store( f ) )
    {
        int64_t pts = frame2pts( get_video_stream(), f );
        _video_packets.jump( pts );
        pts = frame2pts( get_audio_stream(), f );
        if ( !got_audio ) _audio_packets.jump( pts );
        _dts = _adts = f;
        _expected = _dts + 1;
        _expected_audio = _dts + 1;
        //_expected = -99999;
        //_expected_audio = -99999;

        return true;
    }

    if ( (!got_video || !got_audio || !got_subtitle) && f != _expected  )
    {
        bool ok = seek_to_position( f );
        if ( !ok )
            IMG_ERROR( ("seek_to_position: Could not seek to frame ")
                       << frame );
        return ok;
    }


#ifdef DEBUG_DECODE
    cerr << "------------------------------------------------------" << endl;
    cerr << "FETCH START: " << frame << " gotV:" << got_video << " gotA:" << got_audio << endl;
#endif

#ifdef DEBUG_VIDEO_PACKETS
    debug_video_packets(frame, "Fetch", true);
#endif
#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "Fetch", true);
#endif

#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "rFetch" :"Fetch", true);
#endif
#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "rFetch" :"Fetch");
#endif

    int64_t dts = queue_packets( f, false, got_video,
                                 got_audio, got_subtitle);


    _dts = dts;
    assert( _dts >= first_frame() && _dts <= last_frame() );

    _expected = _dts + 1;
    _expected_audio = _adts + 1;


#ifdef DEBUG_DECODE
    LOG_INFO( "------------------------------------------------------" );
    LOG_INFO( "FETCH DONE: " << _dts << "   expected: " << _expected
              << " gotV: " << got_video << " gotA: " << got_audio );
    LOG_INFO( "------------------------------------------------------" );
#endif

#ifdef DEBUG_VIDEO_PACKETS
    debug_video_packets(frame, "FETCH DONE");
#endif

#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "FETCH DONE");
#endif

#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "RFETCH DONE" : "FETCH DONE", true);
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RFETCH DONE" : "FETCH DONE");
#endif


    return true;
}


bool aviImage::frame( const int64_t f )
{


    size_t vpkts = _video_packets.size();
    size_t apkts = _audio_packets.size();

    if ( playback() != kStopped && playback() != kSaving &&
            ( (_video_packets.bytes() +  _audio_packets.bytes() +
               _subtitle_packets.bytes() )  >  kMAX_QUEUE_SIZE ) ||
            ( ( apkts > kMIN_FRAMES || !has_audio() ) &&
              ( vpkts > kMIN_FRAMES || !has_video() )
            ) )
    {
        // std::cerr << "false return: " << std::endl;
        // std::cerr << "vp: " << vpkts
        //           << " vs: " << _images.size()
        //           << " ap: " << apkts
        //           << " as: " << _audio.size()
        //           << std::endl;
        // std::cerr << "sum: " <<
        // ( _video_packets.bytes() +  _audio_packets.bytes() +
        //       _subtitle_packets.bytes() ) << " > " <<  kMAX_QUEUE_SIZE
        //               << std::endl;
        return false;
    }


    if ( f < _frameStart )    _dts = _adts = _frameStart;
    else if ( f > _frameEnd ) _dts = _adts = _frameEnd;
    // else                      _dts = _adts = f;

    image_type_ptr canvas;
    bool ok = fetch(canvas, f);

#ifdef DEBUG_DECODE
    IMG_INFO( "------- FRAME DONE _dts: " << _dts << " _frame: "
              << _frame << " _expected: "  << _expected );
    debug_video_packets( _dts, "fetch", false );
#endif

    return ok;
}

CMedia::DecodeStatus aviImage::decode_vpacket( int64_t& ptsframe,
        const int64_t& frame,
        const AVPacket& pkt )
{
    //int64_t oldpktframe = pktframe;
    CMedia::DecodeStatus status = decode_video_packet( ptsframe, frame, &pkt );
    if ( status == kDecodeOK )
    {
        store_image( ptsframe, _av_frame->pts );
        do {
            status = decode_video_packet( ptsframe, frame, NULL );
            if ( status == kDecodeOK )
            {
                store_image( ptsframe, _av_frame->pts );
            }
        } while ( status == kDecodeOK );
    }
    av_frame_unref(_av_frame);
    av_frame_unref(_filt_frame);
    if ( status == kDecodeDone ) status = kDecodeOK;
    return status;
}

CMedia::DecodeStatus
aviImage::handle_video_packet_seek( int64_t& frame, const bool is_seek )
{
#ifdef DEBUG_HSEEK_VIDEO_PACKETS
    debug_video_packets(frame, "BEFORE HSEEK", true);
#endif

#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "BEFORE HSEEK");
#endif


    if ( _video_packets.empty() || _video_packets.is_flush() )
        LOG_ERROR( _("Wrong packets in handle_video_packet_seek" ) );


    if ( is_seek && _video_packets.is_seek() )
    {
        _video_packets.pop_front();  // pop seek begin packet

    }
    else if ( !is_seek && _video_packets.is_preroll() )
    {
        _video_packets.pop_front();  // pop preroll begin packet
    }
    else
        IMG_ERROR( "handle_video_packet_seek error - no seek/preroll packet" );


    DecodeStatus got_video = kDecodeMissingFrame;
    DecodeStatus status;
    unsigned count = 0;


    while ( !_video_packets.empty() && !_video_packets.is_seek_end() )
    {
        const AVPacket& pkt = _video_packets.front();
        ++count;

        int64_t pktframe;
        if ( pkt.dts != AV_NOPTS_VALUE )
            pktframe = pts2frame( get_video_stream(), pkt.dts );
        else
            pktframe = frame;

        if ( !is_seek && playback() == kBackwards )
        {
            // std::cerr << "pkt " << pktframe << " frame " << frame << std::endl;

            status = decode_image( pktframe, (AVPacket&)pkt );

            if ( status == kDecodeOK && pktframe <= frame )  got_video = status;
        }
        else
        {
            status = decode_image( pktframe, (AVPacket&)pkt );

            if ( status == kDecodeOK && pktframe >= frame )
                got_video = status;
        }

        assert( !_video_packets.empty() );
        _video_packets.pop_front();
    }


    if ( _video_packets.empty() ) {
        LOG_ERROR( _("Empty packets for video seek") );
        return kDecodeError;
    }

    if ( count > 0 && is_seek )
    {
        const AVPacket& pkt = _video_packets.front();
        frame = pts2frame( get_video_stream(), pkt.dts );
    }

    if ( _video_packets.is_seek_end() )
    {
        _video_packets.pop_front();  // pop seek end packet
    }

    if ( count == 0 ) {
        LOG_ERROR( _("Empty seek or preroll") );
        return kDecodeError;
    }

#ifdef DEBUG_HSEEK_VIDEO_PACKETS
    debug_video_packets(frame, "AFTER HSEEK", true);
#endif

#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "AFTER HSEEK");
#endif
    return got_video;
}



void aviImage::wait_image()
{
    mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
    SCOPED_LOCK( vpm );


    for(;;)
    {
        if ( stopped() || saving() || ! _video_packets.empty() ) break;

        CONDITION_WAIT( _video_packets.cond(), vpm );
    }
    return;
}

bool aviImage::in_video_store( const int64_t frame )
{
    SCOPED_LOCK( _mutex );

    // Check if video is already in video store
    video_cache_t::iterator end = _images.end();
    video_cache_t::iterator i = std::find_if( _images.begin(), end,
                                EqualFunctor(frame) );
    if ( i != end ) return true;
    return false;
}


//
// This routine is a simplified copy of the one in ffplay,
// which is (c) 2003 Fabrice Bellard
//

CMedia::DecodeStatus
aviImage::audio_video_display( const int64_t& frame )
{

    if (! _video_packets.empty() )
    {
        assert( !_video_packets.empty() );
        _video_packets.pop_front();
    }

    if ( frame > _frameEnd ) return kDecodeLoopEnd;
    else if ( frame < _frameStart ) return kDecodeLoopStart;


    SCOPED_LOCK( _audio_mutex );
    audio_type_ptr result;

    audio_cache_t::iterator end = _audio.end();
    audio_cache_t::iterator it = std::lower_bound( _audio.begin(), end,
                                 frame,
                                 LessThanFunctor() );
    if ( it == end ) {
        return kDecodeMissingFrame;
    }

    result = *it;


    SCOPED_LOCK( _mutex );
    _hires->frame( frame );
    uint8_t* ptr = (uint8_t*) _hires->data().get();
    memset( ptr, 0, 3*_w*_h*sizeof(uint8_t));

    int channels = result->channels();

    /* total height for one channel */
    int h = _h / channels;
    /* graph height / 2 */
    int h2 = (h * 9) / 20;
    int y1, y, ys, i;
    int i_start = 0;

    if ( _audio_ctx->sample_fmt == AV_SAMPLE_FMT_FLTP ||
            _audio_ctx->sample_fmt == AV_SAMPLE_FMT_FLT )
    {
        float* data = (float*)result->data();
        unsigned size = result->size();
        for (int ch = 0; ch < channels; ++ch)
        {
            i = i_start + ch;
            y1 = ch * h + ( h / 2 );
            for (unsigned x = 0; x < _w; ++x )
            {
                if ( i >= (int)size ) break;
                y = (int(data[i] * 24000 * h2)) >> 15;
                if (y < 0) {
                    y = -y;
                    ys = y1 - y;
                } else {
                    ys = y1;
                }
                // These two checks should not be needed
                // but are here to avoid crashes on some samples
                if ( ys < 0 ) ys = y1;
                if ( y >= h/2 ) y = 0;
                fill_rectangle(ptr, x, ys, 1, y);
                i += channels;
            }
        }
    }
    else if ( _audio_ctx->sample_fmt == AV_SAMPLE_FMT_S16P ||
              _audio_ctx->sample_fmt == AV_SAMPLE_FMT_S16  )
    {
        int16_t* data = (int16_t*)result->data();

        for (int ch = 0; ch < channels; ch++)
        {
            i = i_start + ch;
            y1 = ch * h + ( h / 2 );
            for (unsigned x = 0; x < _w; ++x )
            {
                y = (data[i] * h2) >> 15;
                if (y < 0) {
                    y = -y;
                    ys = y1 - y;
                } else {
                    ys = y1;
                }
                // These two checks should not be needed
                // but are here to avoid crashes on some samples
                if ( ys < 0 ) ys = y1;
                if ( y >= h/2 ) y = 0;
                fill_rectangle(ptr, x, ys, 1, y);
                i += channels;
            }
        }
    }




    _frame = frame;
    refresh();
    return kDecodeOK;

}

CMedia::DecodeStatus aviImage::decode_video( int64_t& f )
{
    int64_t frame = f;


    if ( !has_video() )
    {
        return audio_video_display(_audio_frame);
    }

#ifdef DEBUG_VIDEO_PACKETS
    debug_video_packets(frame, "decode_video", true);
#endif

    Mutex& vpm = _video_packets.mutex();
    SCOPED_LOCK( vpm );

    if ( _video_packets.empty() )
    {
        bool ok = in_video_store( frame );
        if ( ok ) return kDecodeOK;
        return kDecodeError;
    }

    DecodeStatus got_video = kDecodeMissingFrame;


    while ( !_video_packets.empty() && got_video != kDecodeOK )
    {
        if ( _video_packets.is_flush() )
        {
            flush_video();
            _video_packets.pop_front();
            continue;
        }
        else if ( _video_packets.is_seek() )
        {
            got_video = handle_video_packet_seek( frame, true );
            continue;
        }
        else if ( _video_packets.is_preroll() )
        {
            bool ok = in_video_store( frame );
            if ( ok )
            {
                SCOPED_LOCK( _mutex );
                AVPacket& pkt = _video_packets.front();
                int64_t pktframe = pts2frame( get_video_stream(), pkt.dts )
                                   - _frame_offset;
                // if ( pktframe >= frame )
                {
                    got_video = handle_video_packet_seek( frame, false );
                }
                return kDecodeOK;
            }

            got_video = handle_video_packet_seek( frame, false );
            continue;
        }
        else if ( _video_packets.is_loop_start() )
        {
            // With prerolls, Loop indicator remains on before all frames
            // in preroll have been shown.  That's why we check video
            // store here.
            bool ok = in_video_store( frame );

            if ( ok && frame >= loop_start() )
            {
                return kDecodeOK;
            }

            if ( frame < loop_start() )
            {
                assert( !_video_packets.empty() );
                _video_packets.pop_front();
                return kDecodeLoopStart;
            }
            else
            {
                return got_video;
            }
        }
        else if ( _video_packets.is_loop_end() )
        {
            bool ok = in_video_store( frame );

            if ( ok && frame < loop_end() )
            {
                return kDecodeOK;
            }


            assert( !_video_packets.empty() );
            _video_packets.pop_front();
            return kDecodeLoopEnd;
        }
        else if ( _video_packets.is_jump() )
        {
            assert( !_video_packets.empty() );
            _video_packets.pop_front();
            return kDecodeOK;
        }
        else
        {
            assert( !_video_packets.empty() );
            AVPacket& pkt = _video_packets.front();

            int64_t pktframe;
            if ( pkt.dts != AV_NOPTS_VALUE )
            {
                pktframe = pts2frame( get_video_stream(), pkt.dts );
            }
            else
            {
                pktframe = frame;
            }


            // Avoid storing too many frames in advance
            if ( playback() == kForwards &&
                    pktframe > _frame + max_video_frames() )
            {
                got_video = kDecodeOK;
                continue;
            }

            bool ok = in_video_store( pktframe );
            if ( ok )
            {
                // if ( pktframe == frame )
                {
                    got_video = decode_vpacket( pktframe, frame, pkt );
                    //assert( !_video_packets.empty() );
                    _video_packets.pop_front();
                }
                continue;
            }
;
            got_video = decode_image( pktframe, pkt );
            //assert( !_video_packets.empty() );
            _video_packets.pop_front();
            continue;
        }

    }



#ifdef DEBUG_VIDEO_STORES
    debug_video_stores(frame, "decode_video");
#endif

    return got_video;
}




void aviImage::debug_subtitle_stores(const int64_t frame,
                                     const char* routine,
                                     const bool detail)
{

    SCOPED_LOCK( _subtitle_mutex );

    subtitle_cache_t::const_iterator iter = _subtitles.begin();
    subtitle_cache_t::const_iterator last = _subtitles.end();

    std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame
              << " " << routine << " subtitle stores  #"
              << _subtitles.size() << ": "
              << std::endl;

    if ( detail )
    {
        for ( ; iter != last; ++iter )
        {
            int64_t f = (*iter)->frame();
            if ( f == frame )  std::cerr << "S";
            if ( f == _dts )   std::cerr << "D";
            if ( f == _frame ) std::cerr << "F";
            std::cerr << f << " ";
        }
        std::cerr << endl;
    }
}

void aviImage::debug_video_stores(const int64_t frame,
                                  const char* routine,
                                  const bool detail )
{

    SCOPED_LOCK( _mutex );

    video_cache_t::const_iterator iter = _images.begin();
    video_cache_t::const_iterator last = _images.end();

    std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame
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
            std::cerr << f << " (" << (*iter)->pts() << ") ";
        }
        std::cerr << endl;
    }
}


void aviImage::debug_subtitle_packets(const int64_t frame,
                                      const char* routine,
                                      const bool detail )
{
    if ( !has_subtitle() ) return;

    mrv::PacketQueue::Mutex& spm = _subtitle_packets.mutex();
    SCOPED_LOCK( spm );

    mrv::PacketQueue::const_iterator iter = _subtitle_packets.begin();
    mrv::PacketQueue::const_iterator last = _subtitle_packets.end();
    std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame
              << " " << routine << " subtitle packets #"
              << _subtitle_packets.size() << " (bytes:"
              << _subtitle_packets.bytes() << "): "
              << std::endl;

    if ( detail )
    {
        bool in_preroll = false;
        bool in_seek = false;
        for ( ; iter != last; ++iter )
        {
            if ( _subtitle_packets.is_flush( *iter ) )
            {
                std::cerr << "* ";
                continue;
            }
            else if ( _subtitle_packets.is_loop_start( *iter ) ||
                      _subtitle_packets.is_loop_end( *iter ) )
            {
                std::cerr << "L ";
                continue;
            }

            assert( (*iter).dts != MRV_NOPTS_VALUE );
            int64_t f = pts2frame( get_subtitle_stream(), (*iter).dts );
            if ( _subtitle_packets.is_seek_end( *iter ) )
            {
                if ( in_preroll )
                {
                    std::cerr << "[PREROLL END: " << f << "]";
                    in_preroll = false;
                }
                else if ( in_seek )
                {
                    std::cerr << "<SEEK END:" << f << ">";
                    in_seek = false;
                }
                else
                {
                    std::cerr << "+ERROR:" << f << "+";
                }
            }
            else if ( _subtitle_packets.is_seek( *iter ) )
            {
                std::cerr << "<SEEK:" << f << ">";
                in_seek = true;
            }
            else if ( _subtitle_packets.is_preroll( *iter ) )
            {
                std::cerr << "[PREROLL:" << f << "]";
                in_preroll = true;
            }
            else
            {
                if ( f == frame )  std::cerr << "S";
                if ( f == _dts )   std::cerr << "D";
                if ( f == _frame ) std::cerr << "F";
                std::cerr << f << " ";
            }
        }
        std::cerr << std::endl;
    }

}


void aviImage::do_seek()
{
    // No need to set seek frame for right eye here
    if ( _right_eye )  _right_eye->do_seek();

    _seek_frame = handle_loops( _seek_frame );

    _dts = _adts = _seek_frame;

    bool got_video = !has_video();
    bool got_audio = !has_audio();


    if ( !got_audio || !got_video )
    {
        if ( _seek_frame != _expected )
            clear_packets();

        image_type_ptr canvas;
        fetch( canvas, _seek_frame );
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

        if ( has_video() || has_audio() )
        {
            status = decode_video( _seek_frame );

            if ( !find_image( _seek_frame ) && status != kDecodeOK )
                IMG_ERROR( _("Decode video error seek frame " )
                           << _seek_frame
                           << _(" status: ") << decode_error( status ) );
        }

        if ( has_subtitle() && !saving() )
        {
            decode_subtitle( _seek_frame );
            find_subtitle( _seek_frame );
        }

#ifdef DEBUG_VIDEO_STORES
        debug_video_stores(_seek_frame, "doseek" );
#endif

        // Queue thumbnail for update
        image_damage( image_damage() | kDamageThumbnail );
    }

}

//
// SUBTITLE STUFF
//



void aviImage::subtitle_rect_to_image( const AVSubtitleRect& rect )
{
    int imgw = width();
    int imgh = height();


    int dstx = FFMIN(FFMAX(rect.x, 0), imgw);
    int dsty = FFMIN(FFMAX(rect.y, 0), imgh);
    int dstw = FFMIN(FFMAX(rect.w, 0), imgw - dstx);
    int dsth = FFMIN(FFMAX(rect.h, 0), imgh - dsty);

    boost::uint8_t* root = (boost::uint8_t*) _subtitles.back()->data().get();
    assert( root != NULL );

    unsigned char a;
    ImagePixel yuv, rgb;

    unsigned* pal = (unsigned*)rect.data[1];
    for ( unsigned i = 0; i < 16; i += 4 )
    {
        pal[i] = 0x00000000;
        pal[i+1] = 0xFFFFFFFF;
        pal[i+2] = 0xFF000000;
        pal[i+3] = 0xFF000000;
    }

    for ( int x = dstx; x < dstx + dstw; ++x )
    {
        for ( int y = dsty; y < dsty + dsth; ++y )
        {
            boost::uint8_t* d = root + 4 * (x + y * imgw);
            assert( d != NULL );

            boost::uint8_t* const s = rect.data[0] + (x-dstx) +
                                      (y-dsty) * dstw;

            unsigned t = pal[*s];
            a = static_cast<unsigned char>( (t >> 24) & 0xff );
            yuv.b = float( (t >> 16) & 0xff );
            yuv.g = float( (t >> 8) & 0xff );
            yuv.r = float( t & 0xff );

            // rgb = mrv::color::yuv::to_rgb( yuv );
            rgb = yuv;

            if ( rgb.r < 0x00 ) rgb.r = 0x00;
            else if ( rgb.r > 0xff ) rgb.r = 0xff;
            if ( rgb.g < 0x00 ) rgb.g = 0x00;
            else if ( rgb.g > 0xff ) rgb.g = 0xff;
            if ( rgb.b < 0x00 ) rgb.b = 0x00;
            else if ( rgb.b > 0xff ) rgb.b = 0xff;

            float w = a / 255.0f;
            rgb.r = rgb.g * w;
            rgb.g *= w;
            rgb.b *= w;

            *d++ = uint8_t( rgb.r );
            *d++ = uint8_t( rgb.g );
            *d++ = uint8_t( rgb.b );
            *d++ = a;
        }
    }
}


// Flush subtitle buffers
void aviImage::flush_subtitle()
{
    if ( _subtitle_ctx && _subtitle_index >= 0)
    {
        SCOPED_LOCK( _subtitle_mutex );
        avcodec_flush_buffers( _subtitle_ctx );
    }
}

void aviImage::subtitle_stream( int idx )
{
    if ( idx < -1 || unsigned(idx) >= number_of_subtitle_streams() )
        idx = -1;

    if ( idx == _subtitle_index ) return;

    mrv::PacketQueue::Mutex& apm = _subtitle_packets.mutex();
    SCOPED_LOCK( apm );

    flush_subtitle();
    close_subtitle_codec();
    _subtitle_packets.clear();

    _subtitle_index = idx;

    if ( _subtitle_index >= 0 && !filter_graph )
    {
        open_subtitle_codec();
        seek( _frame );
    }

    image_damage( image_damage() | kDamageContents | kDamageSubtitle );
}

void aviImage::store_subtitle( const int64_t& frame,
                               const int64_t& repeat )
{
    if ( _sub.format != 0 )
    {
        if ( _subtitle_file.empty() )
        {
            subtitle_file( filename() );
            return;
        }
        else
            IMG_ERROR( _("Subtitle type ") << _sub.format
                       << _(" not yet supported") );
        return;
    }

    unsigned w = width();
    unsigned h = height();


    image_type_ptr pic( new image_type(
                            frame,
                            w, h, 4,
                            image_type::kRGBA,
                            image_type::kByte,
                            repeat
                        )
                      );

    {
        SCOPED_LOCK( _subtitle_mutex );
        _subtitles.push_back( pic );


        boost::uint8_t* data = (boost::uint8_t*)
                               _subtitles.back()->data().get();

        // clear image
        memset( data, 0, w*h*4 );

        for (unsigned i = 0; i < _sub.num_rects; ++i)
        {
            const AVSubtitleRect* rect = _sub.rects[i];

            assert( rect->type != SUBTITLE_NONE );

            switch( rect->type )
            {
            case SUBTITLE_NONE:
                break;
            case SUBTITLE_BITMAP:
                subtitle_rect_to_image( *rect );
                break;
            case SUBTITLE_TEXT:
            case SUBTITLE_ASS:
                // subtitle_text_to_image( *rect );
                if ( _subtitle_file.empty() )
                    subtitle_file( filename() );
                break;
            }

        }
    }


    avsubtitle_free( &_sub );

}


CMedia::DecodeStatus
aviImage::decode_subtitle_packet( int64_t& ptsframe,
                                  int64_t& repeat,
                                  const int64_t frame,
                                  const AVPacket& pkt
                                )
{
    AVStream* stream = get_subtitle_stream();

    int64_t endframe;
    if ( pkt.pts != AV_NOPTS_VALUE )
    {
        ptsframe = pts2frame( stream, int64_t( double(pkt.pts) +
                                               _sub.start_display_time /
                                               1000.0 ) );
        endframe = pts2frame( stream, int64_t( double(pkt.pts) +
                                               _sub.end_display_time /
                                               1000.0 ) );
        repeat = endframe - ptsframe + 1;
    }
    else
    {
        ptsframe = pts2frame( stream, int64_t( double(pkt.dts) +
                                               _sub.start_display_time /
                                               1000.0 ) );
        endframe = pts2frame( stream, int64_t( double(pkt.dts) +
                                               _sub.end_display_time /
                                               1000.0 ) );
        repeat = endframe - ptsframe + 1;
        IMG_ERROR("Could not determine pts for subtitle frame, "
                  "using " << ptsframe );
    }

    if ( repeat <= 1 )
    {
        repeat = int64_t( fps() * 4 );
    }

    int got_sub = 0;
    avcodec_decode_subtitle2( _subtitle_ctx, &_sub, &got_sub,
                              (AVPacket*)&pkt );
    if ( got_sub == 0 ) return kDecodeError;

    // AVSubtitle has a start display time in ms. relative to pts
    // ptsframe = ptsframe + int64_t( _sub.start_display_time * fps() /
    //                                    1000 );

    return kDecodeOK;
}

// Decode the subtitle
CMedia::DecodeStatus
aviImage::decode_subtitle( const int64_t frame, const AVPacket& pkt )
{
    int64_t ptsframe, repeat;

    DecodeStatus status = decode_subtitle_packet( ptsframe, repeat, frame, pkt );
    if ( status != kDecodeOK )
    {
        IMG_WARNING("Could not decode subtitle frame " << ptsframe
                    << " pts: "
                    << pkt.pts << " dts: " << pkt.dts
                    << " data: " << (void*)pkt.data);
    }
    else
    {
        store_subtitle( ptsframe, repeat );
    }

    return status;
}

CMedia::DecodeStatus
aviImage::handle_subtitle_packet_seek( int64_t& frame,
                                       const bool is_seek )
{
#ifdef DEBUG_PACKETS
    debug_subtitle_packets(frame, "BEFORE PREROLL");
#endif

#ifdef DEBUG_SUBTITLE_STORES
    debug_subtitle_stores(frame, "BEFORE PREROLL");
#endif

    Mutex& mutex = _subtitle_packets.mutex();
    SCOPED_LOCK( mutex );

    if ( _subtitle_packets.is_seek() || _subtitle_packets.is_preroll() )
        _subtitle_packets.pop_front();  // pop seek begin packet

    DecodeStatus got_subtitle = kDecodeMissingFrame;
    DecodeStatus status;
    unsigned count = 0;

    while ( !_subtitle_packets.empty() && !_subtitle_packets.is_seek_end() )
    {
        const AVPacket& pkt = _subtitle_packets.front();
        ++count;

        int64_t repeat = 0;
        int64_t pktframe = get_frame( get_subtitle_stream(), pkt );


        if ( !is_seek && _playback == kBackwards &&
                pktframe >= frame )
        {
            int64_t ptsframe, repeat;
            status = decode_subtitle_packet( ptsframe, repeat,
                                             frame, pkt );
            if ( status == kDecodeOK || status == kDecodeMissingFrame )
            {
                store_subtitle( ptsframe, repeat );

                if ( status == kDecodeOK ) got_subtitle = status;
            }
        }
        else
        {
            if ( pktframe >= frame )
            {
                status = decode_subtitle( frame, pkt );
            }
            else
            {
                status = decode_subtitle_packet( pktframe, repeat, frame, pkt );
            }
        }

        if ( status == kDecodeOK && pktframe <= frame &&
                pktframe + repeat >= frame )  got_subtitle = status;

        _subtitle_packets.pop_front();
    }

    if ( _subtitle_packets.empty() )
    {
        //LOG_ERROR( _("Empty packets for subtitle seek") );
        return kDecodeMissingFrame;
    }

    if ( count > 0 && is_seek )
    {
        const AVPacket& pkt = _subtitle_packets.front();
        frame = get_frame( get_subtitle_stream(), pkt );
    }

    if ( _subtitle_packets.is_seek_end() )
        _subtitle_packets.pop_front();  // pop seek end packet


    if ( count == 0 ) {
        LOG_ERROR( _("Empty seek or preroll") );
        return kDecodeMissingFrame;
    }
#ifdef DEBUG_PACKETS
    debug_subtitle_packets(frame, "AFTER PREROLL");
#endif

#ifdef DEBUG_SUBTITLE_STORES
    debug_subtitle_stores(frame, "AFTER PREROLL");
#endif
    return got_subtitle;
}


int64_t aviImage::wait_subtitle()
{
    mrv::PacketQueue::Mutex& spm = _subtitle_packets.mutex();
    SCOPED_LOCK( spm );

    for(;;)
    {
        if ( stopped() || saving() ) break;

        if ( ! _subtitle_packets.empty() )
        {
            const AVPacket& pkt = _subtitle_packets.front();
            return pts2frame( get_subtitle_stream(), pkt.pts );
        }

        CONDITION_WAIT( _subtitle_packets.cond(), spm );
    }
    return _frame;
}

CMedia::DecodeStatus aviImage::decode_subtitle( const int64_t f )
{
    if ( _subtitle_index < 0 ) return kDecodeOK;

    int64_t frame = f;

    mrv::PacketQueue::Mutex& spm = _subtitle_packets.mutex();
    SCOPED_LOCK( spm );

    if ( _subtitle_packets.empty() )
    {
        bool ok = in_subtitle_store( frame );
        if ( ok ) return kDecodeOK;
        return kDecodeMissingFrame;
    }

    DecodeStatus got_video = kDecodeMissingFrame;

    while ( got_video != kDecodeOK && !_subtitle_packets.empty() )
    {
        if ( _subtitle_packets.is_flush() )
        {
            flush_subtitle();
            _subtitle_packets.pop_front();
        }
        else if ( _subtitle_packets.is_seek() )
        {
            return handle_subtitle_packet_seek( frame, true );
        }
        else if ( _subtitle_packets.is_preroll() )
        {
            AVPacket& pkt = _subtitle_packets.front();
            bool ok = in_subtitle_store( frame );
            if ( ok && pts2frame( get_subtitle_stream(), pkt.pts ) != frame )
                return kDecodeOK;

            return handle_subtitle_packet_seek( frame, false );
        }
        else if ( _subtitle_packets.is_loop_start() )
        {
            AVPacket& pkt = _subtitle_packets.front();
            // with loops, packet dts is really frame
            if ( frame <= pkt.pts )
            {
                flush_subtitle();
                _subtitle_packets.pop_front();
                return kDecodeLoopStart;
            }

            bool ok = in_subtitle_store( frame );
            if ( ok ) return kDecodeOK;
            return kDecodeError;
        }
        else if ( _subtitle_packets.is_loop_end() )
        {
            AVPacket& pkt = _subtitle_packets.front();
            // with loops, packet dts is really frame
            if ( frame >= pkt.pts )
            {
                flush_subtitle();
                _subtitle_packets.pop_front();
                return kDecodeLoopEnd;
            }

            bool ok = in_subtitle_store( frame );
            if ( ok ) return kDecodeOK;
            return kDecodeError;
        }
        else
        {
            AVPacket& pkt = _subtitle_packets.front();

            bool ok = in_subtitle_store( frame );
            if ( ok )
            {
                assert( pkt.pts != MRV_NOPTS_VALUE );
                if ( pts2frame( get_subtitle_stream(), pkt.pts ) == frame )
                {
                    int64_t ptsframe, repeat;
                    decode_subtitle_packet( ptsframe, repeat, frame, pkt );
                    _subtitle_packets.pop_front();
                }
                return kDecodeOK;
            }

            got_video = decode_subtitle( frame, pkt );

            _subtitle_packets.pop_front();
        }

    }


#ifdef DEBUG_SUBTITLE_STORES
    debug_subtitle_stores(frame, "decode_subtitle");
#endif

    return got_video;
}


bool aviImage::in_subtitle_store( const int64_t frame )
{
    SCOPED_LOCK( _subtitle_mutex );

    // This is wrong.  Subtitle spans several frames and has gaps without
    // frames.  Searching for equal frame is bound not to work in most cases.
    subtitle_cache_t::iterator end = _subtitles.end();
    subtitle_cache_t::iterator i = std::find_if( _subtitles.begin(), end,
                                   EqualFunctor(frame) );
    if ( i != end ) return true;
    return false;
}


} // namespace mrv
