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
 * @file   CMedia_audio.cpp
 * @author gga
 * @date   Sat Aug 25 00:14:54 2007
 *
 * @brief
 *
 *
 */


#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS
#include <inttypes.h>



#ifdef _WIN32
#  include <float.h>
#  include <direct.h>
#  define isnan  _isnan
#  define getcwd _getcwd
#else
#  include <unistd.h>
#endif

#include <cstdio>     // for snprintf
#include <cassert>    // for assert


extern "C" {
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavutil/avassert.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}



#include <algorithm>  // for std::min, std::abs
#include <limits>

#include <ImfTimeCodeAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfStringAttribute.h>


#include "core/CMedia.h"
#include "core/Sequence.h"
#include "core/mrvPacketQueue.h"
#include "gui/mrvIO.h"
#include "core/mrvException.h"
#include "core/mrvPlayback.h"
#include "core/mrvFrameFunctors.h"
#include "core/mrvThread.h"
#include "core/mrvAudioEngine.h"
#include "core/mrvSwizzleAudio.h"
#include "core/mrvI8N.h"
#include "core/mrvOS.h"

#include "gui/mrvPreferences.h"

namespace {

const char* kModule = "audio";

}


#ifndef AVCODEC_MAX_AUDIO_FRAMEE
#  define AVCODEC_MAX_AUDIO_FRAME_SIZE 198000
#endif

//#undef DBG
//#define DBG(x) std::cerr << x << std::endl;

// #define DEBUG_AUDIO_PACKETS
// #define DEBUG_AUDIO_PACKETS_DETAIL
// #define DEBUG_AUDIO_STORES
// #define DEBUG_AUDIO_STORES_DETAIL
// #define DEBUG_DECODE
// #define DEBUG_QUEUE
// #define DEBUG_SEEK
// #define DEBUG
// #define DEBUG_AUDIO_SPLIT
// #define DEBUG_SEEK_AUDIO_PACKETS


std::ostream& operator<<( std::ostream& o, mrv::AudioEngine::AudioFormat s )
{
    switch( s )
    {
    case mrv::AudioEngine::kDoubleLSB:
        return o << " dbl_le";
        break;
    case mrv::AudioEngine::kFloatLSB:
        return o << " flt_le";
        break;
    case mrv::AudioEngine::kS32LSB:
        return o << " s32_le";
        break;
    case mrv::AudioEngine::kS16LSB:
        return o << " s16_le";
        break;
    case mrv::AudioEngine::kU8:
        return o << " u8";
        break;
    default:
        return o << " unknown";
    }
}

namespace mrv {


void CMedia::clear_video_packets()
{
    _video_packets.clear();
    _subtitle_packets.clear();
}

void CMedia::clear_audio_packets()
{
    _audio_packets.clear();
    // SCOPED_LOCK( _audio_buf_mutex );
    _audio_buf_used = 0;
}

/**
 * Clear (audio) packets
 *
 */
void CMedia::clear_packets()
{
    clear_video_packets();
    clear_audio_packets();
}




/**
 * Returns the FFMPEG stream index of current audio channel.
 *
 * @return
 */
int CMedia::audio_stream_index() const
{
    if ( _audio_index < 0 ) return -1;
    assert( _audio_index >= 0 );
    assert( unsigned(_audio_index) < _audio_info.size());
    return _audio_info[ _audio_index ].stream_index;
}


// Returns the current audio stream
AVStream* CMedia::get_audio_stream() const
{
    if ( _audio_index < 0 ) return NULL;
    assert( (unsigned) _audio_index < _audio_info.size() );
    if ( _audio_info[ _audio_index ].context == NULL ) return NULL;

    return _audio_info[ _audio_index ].context->streams[ audio_stream_index() ];
}

// Opens the audio codec associated to the current stream
void CMedia::open_audio_codec()
{
    AVStream *stream = get_audio_stream();
    if ( stream == NULL ) return;

    AVCodecParameters* cpar = stream->codecpar;
    if ( cpar == NULL )
    {
        IMG_ERROR( _("No codec context for audio stream.") );
        _audio_index = -1;
        return;
    }

    _audio_codec = (AVCodec*)avcodec_find_decoder( cpar->codec_id );
    if ( _audio_codec == NULL )
    {
        IMG_ERROR( _("No decoder found for audio stream. ID: ")
                   << cpar->codec_id );
        _audio_index = -1;
        return;
    }

    _audio_ctx = avcodec_alloc_context3(_audio_codec);
    int r = avcodec_parameters_to_context(_audio_ctx, cpar);
    if ( r < 0 )
    {
        LOG_ERROR( _("avcodec_copy_context failed for audio") );
        _audio_index = -1;
        return;
    }

    _audio_ctx->pkt_timebase = get_audio_stream()->time_base;

    AVDictionary* opts = NULL;
    av_dict_set(&opts, "threads", "1", 0);
    av_dict_set(&opts, "refcounted_frames", "1", 0);

    if ( avcodec_open2( _audio_ctx, _audio_codec, &opts ) < 0 )
    {
        IMG_ERROR( _("Could not open audio codec.") );
        _audio_index = -1;
    }
    else
    {
        if ( !_audio_buf )
        {
            _audio_max = AVCODEC_MAX_AUDIO_FRAME_SIZE * 4;
            _audio_buf = new aligned16_uint8_t[ _audio_max ];
            assert( (((unsigned long)_audio_buf) % 16) == 0 );
            memset( _audio_buf, 0, _audio_max );
            memory_used += _audio_max;
        }
    }

    frequency = _audio_ctx->sample_rate;

    if ( ! _aframe )
    {

        if ( ! (_aframe = av_frame_alloc()) )
        {
            IMG_ERROR( _("No memory for audio frame") );
            return;
        }
    }
}

// NO AUDIO OFFSET IN THIS FUNCTION
int64_t CMedia::queue_packets( const int64_t frame,
                               const bool is_seek,
                               bool& got_video,
                               bool& got_audio,
                               bool& got_subtitle )
{

    if ( audio_context() != _acontext ) return frame;

#ifdef DEBUG_QUEUE
    LOG_INFO( "BEFORE QUEUE:  D: " << _adts << " E: " << _expected_audio );
    debug_audio_packets(frame, "queue", true);
#endif

    int64_t dts = frame;

    AVStream* stream = get_audio_stream();
    assert( stream != NULL );

    int64_t apts = frame2pts( stream, dts );
    if ( apts < 0 ) {
        apts = 0;
        //return frame;
    }


    mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
    SCOPED_LOCK( apm );

    AVPacket* pkt = av_packet_alloc();

    // Clear the packet
    pkt->size = 0;
    pkt->data = NULL;

    int bytes_per_frame = audio_bytes_per_frame();

    bool eof = false;
    unsigned counter = 0;
    int audio_bytes = 0;

    while (!got_audio)
    {

        if (eof) {
            if (!got_audio && _audio_ctx &&
                _audio_ctx->codec->capabilities & AV_CODEC_CAP_DELAY) {
                pkt = av_packet_alloc();
                pkt->dts = pkt->pts = apts;
                pkt->data = NULL;
                pkt->size = 0;
                pkt->stream_index = audio_stream_index();
                _audio_packets.push_back( *pkt );
            }

            eof = false;
        }

        int error = av_read_frame( _acontext, pkt );

        if ( error < 0 )
        {
            if ( error == AVERROR_EOF )
            {
                counter++;
                if ( counter >= _frame_offset )
                {
                    if ( is_seek )
                    {
                        if ( !got_audio && apts >= 0 )
                        {
                            got_audio = true;
                            DBGM1( "+++ ADD AUDIO SEEK END <<<<<<<<<<<< " );
                            _audio_packets.seek_end(apts);
                        }
                    }
                    break;
                }
                eof = true;
                continue;
            }
            int err = _acontext->pb ? _acontext->pb->error : 0;
            if ( err != 0 )
            {
                IMG_ERROR("fetch: Could not read frame " << frame << " error: "
                          << strerror(err) );
            }

            if ( is_seek || playback() == kBackwards)
            {
                if ( !got_audio && apts >= 0 ) {
                    got_audio = true;
                    DBGM1( "+++ ADD AUDIO SEEK END <<<<<<<<<<<< " );
                    _audio_packets.seek_end(apts);
                 }
            }

            av_packet_unref( pkt );

            break;
        }

        if ( pkt->stream_index == audio_stream_index() )
        {
            int64_t pktframe = get_frame( stream, *pkt );

            if ( playback() == kBackwards )
            {
                // Only add packet if it comes before seek frame
                if ( pktframe <= frame ) _audio_packets.push_back( *pkt );
                if ( pktframe < dts ) dts = pktframe;
            }
            else
            {
                _audio_packets.push_back( *pkt );
                if ( pktframe > dts ) dts = pktframe;
            }

            if ( !got_audio )
            {
                if ( playback() != kBackwards && pktframe > frame )
                    got_audio = true;
                if ( playback() == kBackwards && pktframe < frame )
                    got_audio = true;
                else if ( pktframe == frame )
                {
                    audio_bytes += pkt->size;
                    if ( audio_bytes >= bytes_per_frame ) got_audio = true;
                }
                if ( (is_seek || playback() == kBackwards) &&
                     got_audio && apts >= 0 )
                {
                    DBGM1( "+++ ADD AUDIO SEEK END <<<<<<<<<<<< " );
                    _audio_packets.seek_end(apts);
                }
            }


#ifdef DEBUG_DECODE
            fprintf( stderr, "\t[avi] FETCH A f: %05" PRId64
                     " audio pts: %07" PRId64
                     " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
                     frame, pkt->pts, pkt->dts, pktframe );
#endif
            continue;
        }

        av_packet_unref( pkt );
    }

    int64_t last = last_frame() + _audio_offset;
    int64_t first = first_frame() + _audio_offset;

    if ( dts > last ) dts = last;
    else if ( dts < first ) dts = first;

    return dts;
}

// Seek to the requested frame
bool CMedia::seek_to_position( const int64_t frame )
{
    if ( _acontext == NULL )
    {
        _seek_req = false;
        return true;
    }


    int64_t start = frame - 1;
    //    if ( playback() == kBackwards && start > 0 ) --start;
    if ( start < first_frame() ) start = first_frame();

    int64_t offset = int64_t( double(start) * AV_TIME_BASE
                              / fps() );
    if ( offset < 0 ) offset = 0;

    int flag = AVSEEK_FLAG_BACKWARD;
    // int flag;
    // flag &= ~AVSEEK_FLAG_BYTE;

    int ret = av_seek_frame( _acontext, -1, offset, flag );
    if (ret < 0)
    {
        IMG_ERROR( "Could not seek to frame " << frame << ". Error: "
                   << get_error_text(ret) );
        return false;
    }

    bool got_audio    = !has_audio();


    int64_t apts = 0;

    mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
    SCOPED_LOCK( apm );

    if ( !got_audio ) {
        apts = frame2pts( get_audio_stream(), start );
    }

    if ( !got_audio && apts >= 0 )
    {
        if ( !_seek_req && _playback == kBackwards )
        {
            _audio_packets.preroll(apts);
        }
        else
        {
            _audio_packets.seek_begin(apts);
        }
    }

    bool got_video = true;
    bool got_subtitle = true;


    int64_t dts = queue_packets( frame, true, got_video,
                                 got_audio, got_subtitle );


    _adts = dts;
    _seek_req = false;


#ifdef DEBUG_SEEK
    LOG_INFO( "AFTER SEEK:  D: " << _adts << " E: " << _expected_audio );
    debug_audio_packets(frame);
#endif
    if ( got_audio ) return true;
    else             return false;
}

/**
 * Closed the audio codec if one is open.
 *
 */
void CMedia::close_audio_codec()
{
    if ( _audio_ctx && _audio_index >= 0 )
    {
        avcodec_free_context( &_audio_ctx );
        _audio_ctx = NULL;

        if ( _aframe )
        {
            av_frame_unref(_aframe);
            av_frame_free(&_aframe);
        }
    }
}


/**
 * Given an audio codec context, calculate the approximate bitrate.
 *
 * @param enc   codec context
 *
 * @return bitrate
 */
unsigned int CMedia::calculate_bitrate( const AVStream* stream,
                                        const AVCodecParameters* enc )
{
    unsigned int bitrate;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
    unsigned channels = enc->ch_layout.nb_channels;
#else
    unsigned channels = enc->channels;
#endif
    /* for PCM codecs, compute bitrate directly */
    switch(enc->codec_id) {
    case AV_CODEC_ID_PCM_S32LE:
    case AV_CODEC_ID_PCM_S32BE:
    case AV_CODEC_ID_PCM_U32LE:
    case AV_CODEC_ID_PCM_U32BE:
        bitrate = enc->sample_rate * channels * 32;
        break;
    case AV_CODEC_ID_PCM_S24LE:
    case AV_CODEC_ID_PCM_S24BE:
    case AV_CODEC_ID_PCM_U24LE:
    case AV_CODEC_ID_PCM_U24BE:
    case AV_CODEC_ID_PCM_S24DAUD:
        bitrate = enc->sample_rate * channels * 24;
        break;
    case AV_CODEC_ID_PCM_S16LE:
    case AV_CODEC_ID_PCM_S16BE:
    case AV_CODEC_ID_PCM_U16LE:
    case AV_CODEC_ID_PCM_U16BE:
        bitrate = enc->sample_rate * channels * 16;
        break;
    case AV_CODEC_ID_PCM_S8:
    case AV_CODEC_ID_PCM_U8:
    case AV_CODEC_ID_PCM_ALAW:
    case AV_CODEC_ID_PCM_MULAW:
        bitrate = enc->sample_rate * channels * 8;
        break;
    default:
        bitrate = (unsigned int) enc->bit_rate;
        break;
    }
    return bitrate;
}


int CMedia::audio_bytes_per_frame()
{
    int ret = 0;
    if ( !has_audio() ) return ret;

    int channels = _audio_channels;
    if (_audio_engine->channels() > 0 && channels > 0 ) {
        channels = FFMIN(_audio_engine->channels(), channels);
    }
    if ( channels <= 0 || _audio_format == AudioEngine::kNoAudioFormat)
        return ret;

    // AVSampleFormat fmt = AudioEngine::ffmpeg_format( _audio_format );
    // unsigned bps = av_get_bytes_per_sample( fmt );
    unsigned bps = AudioEngine::bits_for_format( _audio_format );

    if ( _orig_fps <= 0.0f ) _orig_fps = _fps.load();
    ret = (int)( (double) frequency / _orig_fps ) * channels * bps;
    return ret;
}

// Analyse streams and set input values
void CMedia::populate_audio()
{

    bool separate = true;
    if ( _audio_file == filename() )
    {
        separate = false;
    }

    std::ostringstream msg;

    AVFormatContext* c = _context;
    if ( separate || c == NULL ) c = _acontext;

    // Iterate through all the streams available
    for( unsigned i = 0; i < c->nb_streams; ++i )
    {
        // Get the codec context
        const AVStream* stream = c->streams[ i ];
        assert( stream != NULL );

        //const AVCodecContext* ctx = stream->codec;
        const AVCodecParameters* par = stream->codecpar;

        // Determine the type and obtain the first index of each type
        switch( par->codec_type )
        {
        case AVMEDIA_TYPE_SUBTITLE:
        case AVMEDIA_TYPE_VIDEO:
        case AVMEDIA_TYPE_DATA:
        case AVMEDIA_TYPE_ATTACHMENT:
            break;
        case AVMEDIA_TYPE_AUDIO:
        {
            audio_info_t s;
            populate_stream_info( s, msg, c, par, i );
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
            s.channels   = par->ch_layout.nb_channels;
#else
            s.channels   = par->channels;
#endif
            s.frequency  = frequency = par->sample_rate;
            s.bitrate    = calculate_bitrate( stream, par );

            if ( stream->metadata )
            {
                AVDictionaryEntry* lang = av_dict_get(stream->metadata,
                                                      "language", NULL, 0);
                if ( lang && lang->value )
                    s.language = lang->value;
            }
            else
            {
                s.language = "und";
            }

            const char* fmt = av_get_sample_fmt_name( (AVSampleFormat)
                                                      par->format );
            if ( fmt ) s.format = fmt;
            else fmt = "Unknown";

            _audio_info.push_back( s );

            if ( _audio_index < 0 && s.has_codec )
                _audio_index = ((int) _audio_info.size()) - 1;
            break;
        }
        default:
        {
            const char* stream = stream_type( par );
            msg << _("\n\nNot a known stream type for stream #")
                << i << (", type ") << stream;
            break;
        }
        }
    }

    if ( msg.str().size() > 0 )
    {
        LOG_ERROR( filename() << msg.str() );
    }

    if ( _audio_index < 0 )
    {
        LOG_ERROR( filename() << _("\nNo audio stream in file") );
        return;
    }

    // Open the video and audio codecs
    if ( has_audio() )    open_audio_codec();

    // Configure video input properties
    AVStream* stream = NULL;

    if ( has_audio() )
    {
        stream = get_audio_stream();
        assert( stream != NULL );
    }
    else
    {
        return;  // no stream detected
    }



    if ( ( !has_video() && !is_sequence() ) || _fps == 0.0 )
    {
        _orig_fps = _fps = _play_fps = calculate_fps( _context, stream );


#ifdef DEBUG_STREAM_INDICES
        debug_stream_index( stream );
#elif defined(DEBUG_STREAM_KEYFRAMES)
        debug_stream_keyframes( stream );
#endif


        //
        // Calculate frame start and frame end if possible
        //

        _frameStart = 1;

        double start = 0;
        if ( c->start_time != AV_NOPTS_VALUE )
        {
            start = ( ( double )c->start_time / ( double )AV_TIME_BASE );
        }
        else
        {
            start = ( ( double )_audio_info[ _audio_index ].start /
                      ( double ) AV_TIME_BASE );
        }

        _frameStart = int64_t( _fps * start ) + 1;

        int64_t duration;
        if ( c->duration != AV_NOPTS_VALUE )
        {
            duration = int64_t( (_fps * ( double )(c->duration) /
                                 ( double )AV_TIME_BASE ) );
        }
        else
        {
            double length = 0;

            double d = _audio_info[ _audio_index ].duration;
            if ( d > length ) length = d;

            duration = int64_t( length * _fps );
        }
    }


    _adts = _frameStart;
    _expected_audio = _adts + 1;


    //
    // Miscellaneous information
    //
    stream = get_audio_stream();

    dump_metadata( c->metadata );
    if ( stream ) dump_metadata( stream->metadata, _("Audio ") );

}

void CMedia::process_timecode( const Imf::TimeCode& tc )
{
    int hours = tc.hours();
    int minutes = tc.minutes();
    int seconds = tc.seconds();
    int frames = tc.frame();


    if ( tc.dropFrame() )
    {
        // ((30 * 60 - 2) * 10 + 2) * 6 drop-frame frames in 1 hour
        _tc_frame = hours * 107892;
        // 30 * 60 - 2 drop-frame frames in one minute
        _tc_frame += minutes * 1798;
        // for each minute except each 10th minute add 2 frames
        _tc_frame += (minutes / 10) * 2;
        // 30 drop-frame frames in one second
        _tc_frame += seconds * 30;
        _tc_frame += frames;              // frames
    }
    else
    {
        int ifps = (int)round(_fps);
        int hh = hours*3600*ifps;
        int mm = minutes*60*ifps;
        int ss = seconds*ifps;
        _tc_frame = hh + mm + ss + frames;
    }

}

Imf::TimeCode CMedia::str2timecode( const std::string text )
{
    bool drop_frame = false;
    stringArray tc;
    stringArray tcp;

    // Parse 00:00:00:00
    split( tc, text, ':' );

    // DROP FRAME TIMECODE USES ; as SEPARATOR
    // We accept 00;00;00;00 as well as 00:00:00;00
    // We also accept 00.00.00.00 as well as 00:00:00.00
    if ( tc.size() != 4 ) {
        // Parse 00;00;00;00
        split( tcp, text, ';' );
        if ( tcp.size() != 4 )
        {
            // Parse 00.00.00.00 as well as 00:00:00.00
            split( tc, text, '.' );
            if ( tcp.size() == 2 || tc.size() == 2 )
            {
                std::string frames = tc[1].substr( 0, 2 );
                // Parse 00:00:00.00
                split( tc, text, ':' );
                if ( tc.size() != 3 )
                {
                    LOG_ERROR( _("Invalid timecode ") << text );
                    return Imf::TimeCode();
                }

                // tc[2] contains the .00 frames too, so we remove them
                tc[2] = tc[2].substr( 0, 2 );

                tc.push_back( frames );  // add frames to list
            }
        }
        if ( tc.size() != 4 )
        {
            LOG_ERROR( _("Invalid timecode ") << text );
            return Imf::TimeCode();
        }
        drop_frame = true;
    }

    int hours = atoi( tc[0].c_str() );
    int minutes = atoi( tc[1].c_str() );
    int seconds = atoi( tc[2].c_str() );
    int frames = atoi( tc[3].c_str() );

    // to avoid raising an exception
    if ( hours   < 0 || hours   > 23 ) hours = 0;
    if ( minutes < 0 || minutes > 59 ) minutes = 0;
    if ( seconds < 0 || seconds > 59 ) seconds = 0;
    if ( frames  < 0 || frames  > 59 ) frames = 0;

    Imf::TimeCode t( hours, minutes, seconds, frames, drop_frame );
    return t;
}

void CMedia::dump_metadata( AVDictionary *m, const std::string prefix )
{
    if(!m) return;

    AVDictionaryEntry* tag = NULL;

    if ( _attrs.find( _frame ) == _attrs.end() )
    {
        _attrs.insert( std::make_pair( _frame.load(), Attributes() ) );
    }

    while((tag=av_dict_get(m, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        std::string name = prefix;
        name += tag->key;
        if ( name == N_("timecode") || name == N_("Video timecode") ||
             name == N_("Timecode") || name == N_("timeCode") )
        {
            Imf::TimeCode t = str2timecode( tag->value );
            if ( _frame == first_frame() )
            {
                process_timecode( t );
            }
            Imf::TimeCodeAttribute attr( t );
            _attrs[_frame].insert( std::make_pair( name, attr.copy() ) );
            image_damage( image_damage() | kDamageTimecode );
        }
        else if ( name == N_("Video rotate") || name == _("Video rotate") ||
                  name == N_("rotate") )
        {
            Imf::FloatAttribute attr( atof( tag->value ) );
            _attrs[_frame].insert( std::make_pair( name, attr.copy() ) );
        }
        else
        {
            Imf::StringAttribute attr( tag->value );
            _attrs[_frame].insert( std::make_pair( name, attr.copy() ) );
        }
    }
}


/**
 * Attach an audio file to this image.
 *
 * @param file  audio file to attach.
 */
void CMedia::audio_file( const char* file )
{
    flush_audio();

    SCOPED_LOCK( _audio_mutex );

    _audio_file.clear();
    close_audio();
    close_audio_codec();
    _audio_packets.clear();
    swr_free( &forw_ctx );
    forw_ctx = NULL;
    audio_stream( -1 );
    audio_offset( 0 );
    _audio_format = _audio_engine->default_format(); // AudioEngine::kFloatLSB;

    if ( _acontext )
    {
        audio_info_list_t::iterator i = _audio_info.begin();
        for ( ; i != _audio_info.end(); )
        {
            if ( (*i).context == _acontext )
            {
                i = _audio_info.erase( i );
            }
            else
            {
                ++i;
            }
        }

        avformat_close_input( &_acontext );
        _acontext = NULL;
    }


    if ( file == NULL || strlen(file) == 0 )
    {
        file = filename();
        return;
    }

    AVInputFormat*   format = NULL;
    AVDictionary* opts = NULL;
    av_dict_set( &opts, "initial_pause", "1", 0 );
    av_dict_set( &opts, "reconnect", "1", 0 );
    av_dict_set( &opts, "reconnect_streamed", "1", 0 );

    int error = avformat_open_input( &_acontext, file,
                                     format, &opts );
    if ( error < 0 || _acontext == NULL )
    {
        LOG_ERROR( file << _(": Could not open filename.") );
        return;
    }

    error = avformat_find_stream_info( _acontext, NULL );

    if ( error < 0 )
    {
        mrvALERT( file << _(": Could not find stream info.") );
        return;
    }

    _expected_audio = 0;
    _audio_file = file;


    populate_audio();
}


void CMedia::timed_limit_audio_store(const int64_t frame)
{
    unsigned max_frames = max_video_frames();

    if ( max_audio_frames() > max_frames )
        max_frames = max_audio_frames();

#undef timercmp
# define timercmp(a, b, CMP)					\
    (((a).tv_sec == (b).tv_sec) ?				\
     ((a).tv_usec CMP (b).tv_usec) :				\
     ((a).tv_sec CMP (b).tv_sec))

    struct customMore {
        inline bool operator()( const timeval& a,
                                const timeval& b ) const
        {
            return timercmp( a, b, < );
        }
    };

    typedef std::multimap< timeval, int64_t, customMore > TimedSeqMap;
    TimedSeqMap tmp;
    {
        SCOPED_LOCK( _audio_mutex );
        {
            audio_cache_t::iterator  it = _audio.begin();
            audio_cache_t::iterator end = _audio.end();
            for ( ; it != end; ++it )
            {
                if ( (*it)->frame() + max_frames < frame )
                {
                    tmp.insert( std::make_pair( (*it)->ptime(),
                                                (*it)->frame() ) );
                }
            }
        }

        TimedSeqMap::iterator it = tmp.begin();
        for ( ; it != tmp.end() &&
                  ( memory_used >= Preferences::max_memory ||
                    _audio.size() > max_frames ); ++it )
        {
            // Remove and erase this audio frame
            _audio.erase( std::remove_if( _audio.begin(), _audio.end(),
                                          EqualFunctor( it->second ) ),
                          _audio.end() );
        }
    }

}

//
// Limit the audio store to approx. max frames images on each side.
// We have to check both where frame is as well as where _adts is.
//
void CMedia::limit_audio_store(const int64_t frame)
{

    if ( playback() == kForwards )
        return timed_limit_audio_store( frame );

    SCOPED_LOCK( _audio_mutex );

    unsigned max_frames = max_video_frames();

    if ( max_audio_frames() > max_frames )
        max_frames = max_audio_frames();


    int64_t first, last;

    switch( playback() )
    {
        case kBackwards:
            first = frame - max_frames;
            last  = frame + max_frames / 2;
            if ( _adts < first ) first = _adts;
            if ( _adts > last )   last = _adts;
            break;
        case kForwards:
            first = frame - max_frames;
            last  = frame + max_frames;
            if ( _adts < first ) first = _adts;
            if ( _adts > last )   last = _adts;
            break;
        default:
            first = frame - max_frames;
            last  = frame + max_frames;
            if ( _adts < first ) first = _adts;
            if ( _adts > last )   last = _adts;
            break;
    }


#if 1
    if ( first > last )
    {
        int64_t tmp = last;
        last = first;
        first = tmp;
    }
#endif

    _audio.erase( std::remove_if( _audio.begin(), _audio.end(),
                                  NotInRangeFunctor( first, last ) ),
                  _audio.end() );

}


/**
 * Clear (audio) stores
 *
 */
void CMedia::clear_stores()
{
    SCOPED_LOCK( _audio_mutex );

#ifdef DEBUG_DECODE
    std::cerr << ">>>>>>>>>>> clear audio stores" << std::endl;
#endif

    _audio.clear();
    _audio_buf_used = 0;
}

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(59, 33, 100)

uint64_t get_valid_channel_layout(uint64_t channel_layout, int channels)
{
    if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
        return channel_layout;
    else
        return 0;
}

#endif


int CMedia::decode_audio3(AVCodecContext *ctx, int16_t *samples,
                          int* audio_size,
                          AVPacket *avpkt)
{
    int ret = -1;
    bool eof = false;

    int got_audio = 0;
    ret = decode( ctx, _aframe, &got_audio, avpkt, eof );
    if ( !got_audio ) return ret;


#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
    unsigned channels   = ctx->ch_layout.nb_channels;
#else
    unsigned channels   = ctx->channels;
#endif

    assert( _aframe->nb_samples > 0 );
    assert( channels > 0 );
    int data_size = av_samples_get_buffer_size(NULL, channels,
                                               _aframe->nb_samples,
                                               ctx->sample_fmt, 0);
    if (*audio_size < data_size) {
        IMG_ERROR( _("Output buffer size is too small for "
                     "the current frame (")
                   << *audio_size << " < " << data_size << ")" );
        return AVERROR(EINVAL);
    }
    if ( data_size <= 0 )
    {
        IMG_ERROR( _("Output data size is too small for "
                     "the current frame ( ") << data_size << N_(")") );
        return AVERROR(EINVAL);
    }


    if ( ctx->sample_fmt == AV_SAMPLE_FMT_S16P ||
         ctx->sample_fmt == AV_SAMPLE_FMT_S16  )
    {
        _audio_format = AudioEngine::kS16LSB;
    }


#if defined(_WIN32)
    if ( ( channels == 1 || channels >= 6 ) &&
         ctx->sample_fmt == AV_SAMPLE_FMT_FLTP )
        _audio_format = AudioEngine::kS32LSB;
    if ( channels >= 7 && ( ctx->sample_fmt == AV_SAMPLE_FMT_FLTP ||
                            ctx->sample_fmt == AV_SAMPLE_FMT_S32P ||
                            ctx->sample_fmt == AV_SAMPLE_FMT_S32 ) )
        _audio_format = AudioEngine::kS16LSB;
#endif

    AVSampleFormat fmt = AudioEngine::ffmpeg_format( _audio_format );


    if ( ctx->sample_fmt != fmt || channels != _audio_channels )
    {
#if defined( OSX )
        if ( channels > 2 ) _audio_channels = 2;
        else _audio_channels = channels;
#else
        _audio_channels = channels;
#endif
        if (!forw_ctx)
        {
            char buf[256];

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
            AVChannelLayout in_ch_layout = ctx->ch_layout;

            av_channel_layout_describe( &in_ch_layout, buf, 256 );
#else
            uint64_t  in_ch_layout =
            get_valid_channel_layout(ctx->channel_layout, ctx->channels);

            if ( in_ch_layout == 0 )
                in_ch_layout = get_valid_channel_layout( AV_CH_LAYOUT_STEREO,
                                                         ctx->channels);

            if ( in_ch_layout == 0 )
                in_ch_layout = get_valid_channel_layout( AV_CH_LAYOUT_MONO,
                                                         ctx->channels);

            av_get_channel_layout_string( buf, 256, ctx->channels,
                                          in_ch_layout );

#endif
            IMG_INFO( _("Create audio conversion from ") << buf
                      << _(", channels ") << channels
                      << N_(", ") );
            IMG_INFO( _("format ")
                      << av_get_sample_fmt_name( ctx->sample_fmt )
                      << _(", frequency ") << ctx->sample_rate
                      << _(" to") );


#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
            AVChannelLayout out_ch_layout;
            av_channel_layout_copy( &out_ch_layout, &in_ch_layout );
#else
            uint64_t out_ch_layout = in_ch_layout;
#endif

            unsigned out_channels = channels;

#ifdef OSX
#  if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
            if ( _audio_channels == 2 )
            {
               out_ch_layout.order = AV_CHANNEL_ORDER_NATIVE;
               out_ch_layout.nb_channels = 2;
               out_ch_layout.u.mask = AV_CH_LAYOUT_STEREO;
            }
#  else
            if ( _audio_channels == 2 )
                out_ch_layout = AV_CH_LAYOUT_STEREO;
#  endif
#endif

            if ( out_channels > _audio_channels && _audio_channels > 0 )
                out_channels = _audio_channels;
            else
                _audio_channels = channels;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)

            av_channel_layout_describe( &out_ch_layout, buf, 256 );

#else
            av_get_channel_layout_string( buf, 256, out_channels,
                                          out_ch_layout );
#endif

            AVSampleFormat  out_sample_fmt = fmt;
            AVSampleFormat  in_sample_fmt = ctx->sample_fmt;
            int in_sample_rate = ctx->sample_rate;
            int out_sample_rate = in_sample_rate;

            IMG_INFO( buf << _(", channels ") << out_channels
                      << _(", format " )
                      << av_get_sample_fmt_name( out_sample_fmt )
                      << _(", frequency ")
                      << out_sample_rate);


#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
            ret  = swr_alloc_set_opts2(&forw_ctx, &out_ch_layout,
                                       out_sample_fmt,  out_sample_rate,
                                       &in_ch_layout,  in_sample_fmt,
                                       in_sample_rate,
                                       0, NULL);
#else
            forw_ctx  = swr_alloc_set_opts(NULL, out_ch_layout,
                                           out_sample_fmt,  out_sample_rate,
                                           in_ch_layout,  in_sample_fmt,
                                           in_sample_rate,
                                           0, NULL);
#endif
            if(!forw_ctx) {
                LOG_ERROR( _("Failed to alloc swresample library") );
                return 0;
            }

            if(swr_init(forw_ctx) < 0)
            {
                swr_free( &forw_ctx );
                forw_ctx = NULL;
                char buf[256];
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
                av_channel_layout_describe( &in_ch_layout, buf, 256 );
#else
                av_get_channel_layout_string(buf, 256, -1, in_ch_layout);
#endif
                LOG_ERROR( _("Failed to init swresample library with ")
                           << buf << " "
                           << av_get_sample_fmt_name(in_sample_fmt)
                           << _(" frequency: ") << in_sample_rate );
                return 0;
            }
        }

        assert( forw_ctx != NULL );
        assert( ret >= 0 );
        assert( samples != NULL );
        assert( _aframe->nb_samples > 0 );
        assert( _aframe->data != NULL );
        assert( _aframe->data[0] != NULL );
        assert( _aframe->extended_data != NULL );
        assert( _aframe->extended_data[0] != NULL );
        assert( _aframe->buf != NULL );
        assert( _aframe->buf[0] != NULL );

        int len2 = swr_convert(forw_ctx, (uint8_t**)&samples,
                               _aframe->nb_samples,
                               (const uint8_t **)_aframe->extended_data,
                               _aframe->nb_samples );
        if ( len2 <= 0 )
        {
            IMG_ERROR( _("Resampling audio failed") );
            return 0;
        }

        // Just to be safe, we recalc data_size
        data_size = len2 * _audio_channels * av_get_bytes_per_sample( fmt );

#ifdef LINUX
        if ( _audio_channels == 5 )
        {
            if ( fmt == AV_SAMPLE_FMT_FLT )
            {
                Swizzle50<float> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_S32 )
            {
                Swizzle50<int32_t> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_S16 )
            {
                Swizzle50<int16_t> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_U8 )
            {
                Swizzle50<uint8_t> t( samples, len2 );
                t.do_it();
            }
        }
        else if ( _audio_channels == 6 )
        {
            if ( fmt == AV_SAMPLE_FMT_FLT )
            {
                Swizzle51<float> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_S32 )
            {
                Swizzle51<int32_t> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_S16 )
            {
                Swizzle51<int16_t> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_U8 )
            {
                Swizzle51<uint8_t> t( samples, len2 );
                t.do_it();
            }
        }
        else if ( _audio_channels == 8 )
        {
            if ( fmt == AV_SAMPLE_FMT_FLT )
            {
                Swizzle71<float> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_S32 )
            {
                Swizzle71<int32_t> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_S16 )
            {
                Swizzle71<int16_t> t( samples, len2 );
                t.do_it();
            }
            else if ( fmt == AV_SAMPLE_FMT_U8 )
            {
                Swizzle71<uint8_t> t( samples, len2 );
                t.do_it();
            }
        }
#endif

    }
    else
    {
        if ( _audio_channels > 0 )
        {
            assert( _aframe->extended_data != NULL );
            assert( _aframe->extended_data[0] != NULL );
            assert( data_size > 0 );
            memcpy(samples, _aframe->extended_data[0], data_size);
        }
    }

    *audio_size = data_size;


    avpkt->size = 0;
    return ret;
}


/**
 * Given an audio packet, decode it
 *
 * @param ptsframe  returned frame we decoded
 * @param frame     frame we expect
 * @param pkt       packet to decode
 *
 * @return status whether frame was decoded correctly or not.
 */
CMedia::DecodeStatus
CMedia::decode_audio_packet( int64_t& ptsframe,
                             const int64_t frame,
                             const AVPacket& pkt )
{

    SCOPED_LOCK( _audio_mutex );

    AVStream* stream = get_audio_stream();
    if ( !stream ) return kDecodeNoStream;

    // Get the audio codec context
    if ( !_audio_ctx ) return kDecodeNoStream;

#if 0
    assert( !_audio_packets.is_seek_end( pkt ) );
    assert( !_audio_packets.is_seek( pkt ) );
    assert( !_audio_packets.is_flush( pkt ) );
    assert( !_audio_packets.is_preroll( pkt ) );
    assert( !_audio_packets.is_jump( pkt ) );
    assert( !_audio_packets.is_loop_end( pkt ) );
    assert( !_audio_packets.is_loop_start( pkt ) );
#else
    if ( _audio_packets.is_seek_end( pkt ) ||
         _audio_packets.is_seek( pkt ) ||
         _audio_packets.is_flush( pkt ) ||
         _audio_packets.is_preroll( pkt ) ||
         _audio_packets.is_jump( pkt ) ||
         _audio_packets.is_loop_end( pkt ) ||
         _audio_packets.is_loop_start( pkt ) )
        return kDecodeOK;
#endif

    ptsframe = get_frame( stream, pkt );
    if ( ptsframe == AV_NOPTS_VALUE ) ptsframe = frame;

    // Make sure audio frames are continous during playback to
    // accomodate weird sample rates not evenly divisable by frame rate
    if (  _audio_buf_used != 0 && (!_audio.empty())  )
    {
        int64_t tmp = std::abs(ptsframe - _audio_last_frame);
        if ( tmp >= 0 && tmp <= 10 )
        {
            ptsframe = _audio_last_frame + 1;
        }
    }


    // if ( _audio_packets.is_jump( pkt ) )
    // {
    //     return kDecodeOK;
    // }

    AVPacket* pkt_temp = av_packet_alloc();
    pkt_temp->data = pkt.data;
    pkt_temp->size = pkt.size;

    //    assert( pkt.size != 0 && pkt.data != NULL );  // can crash

    assert( _audio_buf != NULL );

    int audio_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;  //< correct
    assert( pkt_temp->size <= audio_size );

    if ( (unsigned)(_audio_buf_used + audio_size) > _audio_max )
    {
        aligned16_uint8_t* old = _audio_buf;
        _audio_buf = new aligned16_uint8_t[ _audio_max + audio_size ];
        assert( (((unsigned long)_audio_buf) % 16) == 0 );
        assert( _audio_max > 0 );
        memcpy( _audio_buf, old, _audio_max );
        delete [] old;
        _audio_max += audio_size;
        memory_used += audio_size;
    }

    {
        // Decode the audio into the buffer
        int ret = decode_audio3( _audio_ctx,
                                 ( int16_t * )( (int8_t*)_audio_buf +
                                                _audio_buf_used ),
                                 &audio_size, pkt_temp );
        if ( ret < 0 )
        {
            IMG_ERROR( _("Decode_audio failed with error: ")
                       << get_error_text(ret));
            return kDecodeMissingSamples;
        }
        assert( audio_size <= AVCODEC_MAX_AUDIO_FRAME_SIZE );

        // If no samples are returned, then break now
        if ( audio_size <= 0 )
        {
            pkt_temp->size = 0;
            return kDecodeMissingSamples;
        }

        assert( audio_size > 0 );


        _audio_buf_used += audio_size;


        do {
            ret = decode_audio3( _audio_ctx,
                                 ( int16_t * )( (uint8_t*)_audio_buf +
                                                _audio_buf_used ),
                                 &audio_size, NULL );
            if ( ret > 0 ) _audio_buf_used += audio_size;
        } while ( ret > 0 );
    }


    return kDecodeOK;
}


/**
 * Decode an audio packet and split it into frames
 *
 * @param frame         frame we expect
 * @param pkt           audio packet
 *
 * @return status whether frame was reached and decoded correctly or not.
 */
CMedia::DecodeStatus
CMedia::decode_audio( const int64_t frame, const AVPacket& pkt )
{

    int64_t audio_frame = frame;

    CMedia::DecodeStatus got_audio = decode_audio_packet( audio_frame,
                                                          frame, pkt );
    if ( got_audio != kDecodeOK ) {
        IMG_ERROR( _("decode_audio_packet ") << frame << _(" failed with ")
                   << get_error_text( got_audio ) );
        return got_audio;
    }

    got_audio = kDecodeMissingFrame;


    unsigned int index = 0;

    int64_t last = audio_frame;

    int bytes_per_frame = audio_bytes_per_frame();
    if ( bytes_per_frame == 0 ) return kDecodeOK;

    if ( last == in_frame() || (stopped() /* || saving() */ ) )
    {
        if ( bytes_per_frame > _audio_buf_used && _audio_buf_used > 0 )
        {
            bytes_per_frame = _audio_buf_used;
        }
    }


    // Split audio read into frame chunks
    {
        //SCOPED_LOCK( _audio_buf_mutex );
        for (;;)
        {
            if ( bytes_per_frame > _audio_buf_used ) break;


            uint32_t skip = store_audio( last,
                                         (uint8_t*)_audio_buf + index,
                                         bytes_per_frame );
            if ( skip == 0 ) break;

            index += skip;

            if ( last >= frame ) got_audio = kDecodeOK;

            //assert( bytes_per_frame <= _audio_buf_used );
            _audio_buf_used -= bytes_per_frame;

            ++last;
        }
    }


    if (_audio_buf_used > 0 && index > 0 )
    {
        //
        // NOTE: audio buffer must remain 16 bits aligned for ffmpeg.
        memmove( _audio_buf, _audio_buf + index, _audio_buf_used );
    }

    if ( _audio_buf_used < 0 ) _audio_buf_used = 0;

    return got_audio;
}


// Return the number of frames cached for jog/shuttle
// or 0 for no cache or numeric_limits<int>max() for full cache
unsigned CMedia::max_audio_frames()
{
    if ( _audio_cache_size > 0 )
        return _audio_cache_size;
    else if ( _audio_cache_size == 0 )
        return unsigned( fps()*2 );
    else
        return std::numeric_limits<unsigned>::max();
}




void CMedia::audio_stream( int idx )
{
    if ( idx < -1 || unsigned(idx) >= number_of_audio_streams() )
        idx = -1;

    if ( _right_eye && _owns_right_eye ) _right_eye->audio_stream(idx);


    if ( idx == _audio_index ) {
        return;
    }

    mrv::PacketQueue::Mutex& am  = _audio_packets.mutex();
    SCOPED_LOCK( am );

    if ( has_audio() )
    {
        flush_audio();
        close_audio();
        close_audio_codec();
        _audio_packets.clear();
        if ( forw_ctx ) swr_free( &forw_ctx );
        forw_ctx = NULL;
        _audio_channels = 0;
        _audio_format = _audio_engine->default_format();
    }

    clear_stores();

    _audio_index = idx;

    if ( _audio_index >= 0 )
    {
        open_audio_codec();
        _audio_muted = false; //true;
        seek( _frame );
        _audio_muted = false;
    }

}


/**
 * Store an audio frame.
 *
 * @param frame            frame to store
 * @param audio_frame      packet audio decoded
 * @param buf              audio data
 * @param size             size of audio data
 *
 * @return data size or 0 on failure.
 */
unsigned int
CMedia::store_audio( const int64_t audio_frame,
                     const uint8_t* buf, const unsigned int size )
{

    assert( buf != NULL );
    if ( buf == NULL || size == 0 )
    {
        IMG_ERROR( _("store_audio: Invalid data.  buf is ")
                   << (void*)buf << _(" size is ") << size );
        return 0;
    }


//    if ( audio_frame % 8 == 0 )
    {
    }

    //  assert( audio_frame <= last_frame()  );
    //  assert( audio_frame >= first_frame() );

    int64_t f = audio_frame;
    _audio_last_frame = f;

    // Get the audio info from the codec context
    unsigned short channels = _audio_channels;

    audio_type_ptr aud = audio_type_ptr( new audio_type( audio_frame,
                                         frequency,
                                         channels,
                                         buf, size ) );
    if ( aud.get() == NULL )
    {
        IMG_ERROR( _("No memory for audio frame") );
        IMG_ERROR( _("Audios #") << _audio.size() );
        return 0;
    }

    if ( _audio.empty() || _audio.back()->frame() < f )
    {
        _audio.push_back( aud );
    }
    else
    {
        audio_cache_t::iterator end = _audio.end();
        audio_cache_t::iterator at = std::lower_bound( _audio.begin(),
                                     end,
                                     f,
                                     LessThanFunctor() );
        // Avoid storing duplicate frames, replace old frame with this one
        if ( at != end )
        {
            if ( (*at)->frame() == f )
            {
                at = _audio.erase(at);
            }
        }

        _audio.insert( at, aud );
    }


    assert( aud->size() == size );

    return aud->size();
}


/**
 * Fetch audio packet for a particular frame.
 *
 * @param frame   frame to find audio for (with offsets taken into account)
 */
void CMedia::fetch_audio( const int64_t frame )
{
    // Seek to the correct position if necessary

    bool got_audio = !has_audio();
    if ( got_audio ) return;

    if ( frame != _expected_audio )
    {
        DBGM3( "FRAME(" << frame << ") != EXPECTED (" << _expected_audio
                  << ")" );
        bool ok = seek_to_position( frame );
        DBGM3( "FRAME(" << frame << ") NEW EXPECTED (" << _expected_audio
              << ")" );
        if (ok) return;
    }

    DBGM3( ">>>>>>>> FRAME " << frame << " IS EXPECTED " << _expected_audio );


    bool got_video = true;
    bool got_subtitle = true;

    DBGM3( "queue packets " << frame << " is_seek " << false
         << " got audio " << got_audio );

    int64_t dts = CMedia::queue_packets( frame, false,
                                         got_video, got_audio,
                                         got_subtitle );


    _adts = dts;
    _expected_audio = dts + 1;

    DBGM3( "DTS " << dts << " EXPECTED " << _expected_audio );

}



void CMedia::audio_initialize()
{
    if ( _audio_engine ) return;

    _audio_engine = mrv::AudioEngine::factory();
    if ( ! _audio_engine ) {
        IMG_ERROR( _("Could not initialize audio engine" ) );
        return;
    }
    _audio_channels = (unsigned short) _audio_engine->channels();
    _audio_format = _audio_engine->default_format();
}



void CMedia::wait_audio()
{
    mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
    SCOPED_LOCK( apm );

    for (;;)
    {
        //        if ( stopped() || ! _audio_packets.empty() ) break;

        bool got_audio = in_audio_store( _frame + _audio_offset );
        if ( stopped() || ( ! _audio_packets.empty() ) || got_audio )
            return;

        CONDITION_WAIT( _audio_packets.cond(), apm );
    }

    return;
}



bool CMedia::open_audio( const short channels,
                         const unsigned nSamplesPerSec )
{
    assert( _audio_engine != NULL );

    AudioEngine::AudioFormat format = _audio_format;

    // Avoid conversion to float if unneeded
    if ( _audio_ctx )
    {
        if ( _audio_ctx->sample_fmt == AV_SAMPLE_FMT_S16P ||
             _audio_ctx->sample_fmt == AV_SAMPLE_FMT_S16 )
        {
            format = AudioEngine::kS16LSB;
        }
    }

    if ( _fps > 100.0 )
    {
        // At this speed, we consume buffers really fast.  Use more buffers
        // This fixes a bug in Windows where sound would not play.
        // On Linux, this does nothing.
        DBGM3("16 audio buffers" );
        _audio_engine->buffers( 16 );
    }

    AVSampleFormat ft = AudioEngine::ffmpeg_format( format );

    bool ok = false;
    int ch = channels;
    for ( int fmt = format; fmt > 0; fmt -= 2 ) // -2 to skip be/le versions
    {
        SCOPED_LOCK( _audio_mutex );
        ok = _audio_engine->open( ch, nSamplesPerSec,
                                  (AudioEngine::AudioFormat)fmt );
        if ( ok ) break;
    }

    _audio_format = _audio_engine->format();
    _audio_channels = _audio_engine->channels();
    _samples_per_sec = nSamplesPerSec;


    return ok;
}


bool CMedia::play_audio( const mrv::audio_type_ptr result )
{
    if ( !_audio_engine ) return true;

    double speedup = _play_fps / _fps;
    unsigned nSamplesPerSec = unsigned( (double) result->frequency() * speedup );
    if ( nSamplesPerSec != _samples_per_sec ||
         result->channels() != _audio_channels ||
         _audio_format == AudioEngine::kNoAudioFormat ||
         AudioEngine::device_index() != AudioEngine::old_device_index() )
    {
        if ( ! open_audio( result->channels(), nSamplesPerSec ) )
        {
            IMG_ERROR( _("Could not open audio driver") );
            _audio_index = -1;
            return false;
        }

    }


    if ( ! _audio_engine->play( (char*)result->data(), result->size() ) )
    {
        IMG_ERROR( _("Playback of audio frame ") << result->frame()
                   << _(" failed") );
        close_audio();
        return false;
    }

    return true;
}

void CMedia::fill_rectangle( uint8_t* buf, int xl, int yl, int w, int h )
{
    int yh = yl+h;
    int xh = xl+w;
    for ( int y = yl; y < yh; ++y )
    {
        uint8_t* d = &buf[3*(y*_w+xl)];
        for ( int x = xl; x < xh; ++x )
        {
            *d++ = 255;
            *d++ = 255;
            d++;
        }
    }
}

bool CMedia::find_audio( const int64_t frame )
{

    if ( frame == _audio_frame ) return false;

    audio_type_ptr result;


    {
#ifdef DEBUG_AUDIO_PACKETS
        debug_audio_packets(frame, _right_eye ? "RFIND" : "FIND");
#endif

#ifdef DEBUG_AUDIO_STORES
        debug_audio_stores(frame, _right_eye ? "RFIND" : "FIND");
#endif

        SCOPED_LOCK( _audio_mutex );

        if ( frame < in_frame() )
            return true;

        audio_cache_t::iterator end = _audio.end();
        audio_cache_t::iterator i = std::lower_bound( _audio.begin(), end,
                                                      frame,
                                                      LessThanFunctor() );

        if ( i == end )
        {
            if ( _audio_offset == 0 && frame <= _frameOut)
            {
                IMG_WARNING( _("Audio frame ") << frame << _(" not found") );
            }
            return false;
        }

        result = *i;

    }


    _audio_frame = result->frame();

    //assert( result->frame() == frame );
    assert( result->size() > 0 );
    assert( result->data() != NULL );

    bool ok = play_audio( result );
    if ( !ok )
    {
        IMG_ERROR( _("Could not play audio frame ") << frame  );
    }


    limit_audio_store( frame );

    _audio_pts = (result->frame() - _audio_offset - _start_number) / _orig_fps;

    _audio_clock = double(av_gettime_relative()) / 1000000.0;
    set_clock_at(&audclk, _audio_pts, 0, _audio_clock );

    sync_clock_to_slave( &extclk, &audclk );

    return ok;
}



void CMedia::flush_audio()
{
    if ( _audio_ctx && _audio_index >= 0 )
    {
        SCOPED_LOCK( _audio_mutex );
        avcodec_flush_buffers( _audio_ctx );
    }
}



void CMedia::close_audio()
{
#ifndef OSX
    if ( _audio_engine ) _audio_engine->close();
#endif
    _samples_per_sec = 0;
}

void CMedia::audio_shutdown()
{
    delete _audio_engine;
    _audio_engine = NULL;
}


/// Change audio volume
void CMedia::volume( float v )
{
    if ( _right_eye ) _right_eye->volume( v );

    if ( !_audio_engine ) return;

    TRACE( name() << " has audio level " << v );


    _audio_engine->volume( v );
}


/// Change audio volume
float CMedia::volume() const
{
    if ( !_audio_engine ) return 0.0f;
    return _audio_engine->volume();
}


CMedia::DecodeStatus
CMedia::handle_audio_packet_seek( int64_t& frame,
                                  const bool is_seek )
{
#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "RHANDLESEEK" : "HANDLESEEK");
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RHANDLESEEK" : "HANDLESEEK");
#endif

    if ( _audio_packets.empty() || _audio_packets.is_flush() )
        LOG_ERROR( _("Wrong packets in handle_audio_packet_seek" ) );

    if ( is_seek && _audio_packets.is_seek() )
    {
        _audio_packets.pop_front();  // pop seek begin packet
    }
    else if ( !is_seek && _audio_packets.is_preroll() )
    {
        if ( playback() == kBackwards && _audio.size() > max_audio_frames() )
            return kDecodeOK;
        _audio_packets.pop_front();
    }
    else
        IMG_ERROR( _("Audio packet is unknown, expected seek or preroll") );

    DecodeStatus got_audio = kDecodeMissingFrame;
    DecodeStatus status;
    unsigned count = 0;

    assert( _audio_buf_used == 0 );

    if ( !_audio_packets.empty() && !_audio_packets.is_seek_end() )
    {
        const AVPacket& pkt = _audio_packets.front();
      _audio_last_frame = get_frame( get_audio_stream(), pkt );
    }


    int64_t last = _audio_last_frame;


    while ( !_audio_packets.empty() && !_audio_packets.is_seek_end() &&
            !_audio_packets.is_loop_end() && !_audio_packets.is_loop_start() )
    {
        const AVPacket& pkt = _audio_packets.front();
        ++count;

        int64_t pktframe = get_frame( get_audio_stream(), pkt );
        if ( pktframe == AV_NOPTS_VALUE )  pktframe = frame;

        if ( !in_audio_store( pktframe ) )
        {
            if ( (status = decode_audio( pktframe, pkt )) == kDecodeOK )
            {
                if ( pktframe <= frame && playback() == kBackwards )
                    got_audio = kDecodeOK;
            }
        }
        else
        {
            status = decode_audio_packet( last, pktframe, pkt ); //frame
            if ( status != kDecodeOK )
                LOG_WARNING( _( "decode_audio_packet failed for frame " )
                             << frame );
        }

        _audio_packets.pop_front();
    }

    if ( _audio_packets.empty() ) {
        IMG_ERROR( _("Audio packets empty at end of seek") );
        return kDecodeMissingFrame;
    }


    if ( count > 0 && is_seek )
    {
        assert( !_audio_packets.empty() );
        const AVPacket& pkt = _audio_packets.front();
        frame = _audio_frame = get_frame( get_audio_stream(), pkt ) /*+ _audio_offset*/ ;
    }

    if ( _audio_packets.is_seek_end() )
    {
        _audio_packets.pop_front();  // pop seek/preroll end packet
    }

#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "RDOSEEK END" : "DOSEEK END");
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RDOSEEK END" : "DOSEEK END");
#endif

    if ( _audio_packets.empty() ) return got_audio;


    return kDecodeOK;
}

bool CMedia::in_audio_store( const int64_t frame )
{
    SCOPED_LOCK( _audio_mutex );
    // Check if audio is already in audio store
    audio_cache_t::iterator end = _audio.end();
    audio_cache_t::iterator i = std::find_if( _audio.begin(), end,
                                EqualFunctor(frame) );
    if ( i != end ) return true;

    return false;
}

CMedia::DecodeStatus CMedia::decode_audio( int64_t& f )
{

    int64_t frame = f;

#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "RDECODE" : "DECODE");
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RDECODE" : "DECODE");
#endif

    DecodeStatus got_audio = kDecodeMissingFrame;


    int64_t first = first_frame();
    int64_t last  = last_frame();

    // if ( frame < loop_start() && looping() != kPingPong )
    //     return kDecodeLoopStart;

    mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
    SCOPED_LOCK( apm );

    if ( _audio_packets.empty() )
    {
        bool ok = in_audio_store( frame );
        if ( ok ) return kDecodeOK;

        return kDecodeMissingFrame;
    }


    AVStream* stream = get_audio_stream();
    if ( stream == NULL ) return kDecodeMissingSamples;

    while ( got_audio != kDecodeOK && !_audio_packets.empty() )
    {
        assert( !_audio_packets.is_seek_end() );
        if ( _audio_packets.is_flush() )
        {
            flush_audio();
            _audio_packets.pop_front();
        }
        else if ( _audio_packets.is_loop_start() )
        {
            // This is all needed, as loop end can be called
            // early due to few timestamps in audio track
            bool ok = in_audio_store( frame );

            if ( ok && frame >= first  )
            {
                return kDecodeOK;
            }

            flush_audio();
            _audio_packets.pop_front();
            return kDecodeLoopStart;
        }
        else if ( _audio_packets.is_loop_end() )
        {
            // This is all needed, as loop end can be called
            // early due to few timestamps in audio track
            bool ok = in_audio_store( frame );

            if ( ok && frame <= last )
            {
                return kDecodeOK;
            }


            flush_audio();
            _audio_packets.pop_front();
            return kDecodeLoopEnd;
        }
        else if ( _audio_packets.is_seek()  )
        {
            clear_stores();  // audio stores MUST be cleared when seeked
            got_audio = handle_audio_packet_seek( frame, true );
            continue;
        }
        else if ( _audio_packets.is_preroll() )
        {
            bool ok = in_audio_store( frame );
            if ( ok ) {
                assert( !_audio_packets.empty() );
                AVPacket& pkt = _audio_packets.front();
                int64_t pktframe = get_frame( stream, pkt );
                if ( pktframe >= frame )
                {
                    // SCOPED_LOCK( _audio_buf_mutex );
                    _audio_buf_used = 0;
                    got_audio = handle_audio_packet_seek( frame, false );
                }
                return kDecodeOK;
            }

            // SCOPED_LOCK( _audio_buf_mutex );
            _audio_buf_used = 0;
            got_audio = handle_audio_packet_seek( frame, false );
            continue;
        }
        // else if ( _audio_packets.is_jump() )
        // {
        //     _audio_packets.pop_front();
        //     return kDecodeOK;
        // }
        else
        {
            assert( !_audio_packets.empty() );
            AVPacket& pkt = _audio_packets.front();

#if 0
            int64_t pktframe = get_frame( stream, pkt );
            // This does not work as decode_audio_packet may decode more
            // than one frame of audio (see Essa.wmv)
            bool ok = in_audio_store( frame );
            if ( ok )
            {
                got_audio = decode_audio_packet( pktframe, frame, pkt );
                _audio_packets.pop_front();
                continue;
            }
#endif
            got_audio = decode_audio( frame, pkt );
            _audio_packets.pop_front();
            continue;
        }

    }

#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "RDECODE END" : "DECODE END");
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RDECODE END" : "DECODE END");
#endif


    return got_audio;
}



void CMedia::do_seek()
{

    if ( _right_eye ) _right_eye->do_seek();

    bool got_audio = !has_audio();


    int64_t x = _seek_frame + _audio_offset;

    if ( !got_audio )
    {
        if ( x != _expected_audio )
            clear_audio_packets();

        fetch_audio( x );
    }

    // Seeking done, turn flag off
    if ( stopped() || saving() )
    {

        if ( has_audio() && !got_audio )
        {
            int64_t f = x;
            DecodeStatus status = decode_audio( f );
            if ( status != kDecodeNoStream && !_audio_muted  )
                find_audio( x );
            else
                _audio_muted = false;
        }
    }

    if ( _frameStart == _frameEnd && _seek_frame != _frameStart )
        _seek_frame = _frameStart;

    find_image( _seek_frame );

    // Seeking done, turn flag off
    _seek_req = false;

    // Queue thumbnail for update
    image_damage( image_damage() | kDamageThumbnail );
}



void CMedia::debug_audio_stores(const int64_t frame,
                                const char* routine,
                                const bool detail)
{
    SCOPED_LOCK( _audio_mutex );

    audio_cache_t::const_iterator iter = _audio.begin();
    audio_cache_t::const_iterator last = _audio.end();

    std::cerr << this << std::dec << " " << name()
              << " S:" << _frame << " D:" << _adts
              << " A:" << frame << " " << routine << " audio stores #"
              << _audio.size() << ": ";

    if ( iter != last )
    {
        std::cerr << (*iter)->frame() << "-" << (*(last-1))->frame();
    }

    std::cerr << std::endl;

    if (detail )
    {
        for ( ; iter != last; ++iter )
        {
            int64_t f = (*iter)->frame();
            if ( f == frame )  std::cerr << "P";
            if ( f == _adts )   std::cerr << "D";
            if ( f == _frame ) std::cerr << "F";
            std::cerr << f << " ";
        }
        std::cerr << std::endl;
    }
}



bool CMedia::in_subtitle_packets( const int64_t frame )
{
    mrv::PacketQueue::const_iterator iter = _subtitle_packets.begin();
    mrv::PacketQueue::const_iterator last = _subtitle_packets.end();

    AVStream* stream = get_subtitle_stream();
    for ( ; iter != last; ++iter )
    {
        if ( get_frame( stream, (*iter) ) == frame )
            return true;
    }
    return false;
}

bool CMedia::in_video_packets( const int64_t frame )
{
    mrv::PacketQueue::const_iterator iter = _video_packets.begin();
    mrv::PacketQueue::const_iterator last = _video_packets.end();

    AVStream* stream = get_video_stream();
    for ( ; iter != last; ++iter )
    {
        if ( get_frame( stream, (*iter) ) == frame )
            return true;
    }
    return false;
}

bool CMedia::in_audio_packets( const int64_t frame )
{
    mrv::PacketQueue::const_iterator iter = _audio_packets.begin();
    mrv::PacketQueue::const_iterator last = _audio_packets.end();

    AVStream* stream = get_audio_stream();
    for ( ; iter != last; ++iter )
    {
        if ( get_frame( stream, (*iter) ) == frame )
            return true;
    }
    return false;
}

void CMedia::debug_audio_packets(const int64_t frame,
                                 const char* routine,
                                 const bool detail)
{
    if ( !has_audio() ) return;
    if ( name() != "alpha-trailer-2_h480p.mov" ) return;

    mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
    SCOPED_LOCK( apm );

    mrv::PacketQueue::const_iterator iter = _audio_packets.begin();
    mrv::PacketQueue::const_iterator last = _audio_packets.end();
    std::cerr << name()
              << " F:" << frame << " D:" << _adts
              << " A:" << _audio_frame << " " << routine << " audio packets #"
              << _audio_packets.size() << " (" << _audio_packets.bytes() << "): ";

    AVStream* stream = get_audio_stream();

    if ( iter == last )
    {
        std::cerr << std::endl << "***EMPTY***";
    }
    else
    {

        if ( _audio_packets.is_loop_end( (*iter) ) ||
                _audio_packets.is_loop_start( (*iter) ) )
        {
            std::cerr << "L(" << (*iter).dts << ")";
        }
        else
        {
            std::cerr << get_frame( stream, *iter );
        }

        std::cerr << '-';

        if ( _audio_packets.is_loop_end( *(last-1) ) ||
                _audio_packets.is_loop_start( *(last-1) ) )
        {
            std::cerr << "L(" << (*(last-1)).dts << ")";
        }
        else
        {
            std::cerr << get_frame( stream, *(last-1) );
        }

        std::cerr << std::endl;
    }

    if ( detail )
    {

        bool in_preroll = false;
        bool in_seek = false;

        int counter = 0;

        for ( ; iter != last; ++iter )
        {
            if ( _audio_packets.is_flush( *iter ) )
            {
                std::cerr << " *";
                continue;
            }
            else if ( _audio_packets.is_loop_end( *iter ) )
            {
                std::cerr << "Le(" << iter->dts << ")";
                continue;
            }
            else if ( _audio_packets.is_loop_start( *iter ) )
            {
                std::cerr << "Ls(" << iter->dts << ")";
                continue;
            }

            int64_t f = get_frame( stream, (*iter) );

            if ( _audio_packets.is_seek_end( *iter ) )
            {
                if ( in_preroll )
                {
                    if ( counter != 0 )
                    {
                        std::cerr << "[PREROLL END: " << f << "]";
                    }
                    else
                    {
                        std::cerr << "[**BAD** PREROLL: " << f << "]";
                    }
                    counter = 0;
                    in_preroll = false;
                }
                else if ( in_seek )
                {
                    if ( counter != 0 )
                    {
                        std::cerr << "<SEEK END: " << f << ">";
                    }
                    else
                    {
                        std::cerr << "<**BAD** SEEK: " << f << ">";
                    }
                    counter = 0;
                    in_seek = false;
                }
                else
                {
                    std::cerr << "+BAD SEEK END:" << f << "+";
                }
            }
            else if ( _audio_packets.is_seek( *iter ) )
            {
                std::cerr << "<SEEK:" << f << ">";
                in_seek = true;
            }
            else if ( _audio_packets.is_preroll( *iter ) )
            {
                std::cerr << "[PREROLL:" << f << "]";
                in_preroll = true;
            }
            else
            {
                // Audio packets often have many packets for same frame.
                // keep printout simpler
                // if ( f == last_frame ) continue;

                if ( f == frame )  std::cerr << "S";
                if ( f == _adts )  std::cerr << "D";
                if ( f == _frame ) std::cerr << "F";
                std::cerr << f << " ";
                if ( in_preroll || in_seek ) counter++;
            }
        }
    }

    std::cerr << std::endl;

}




} // namespace mrv
