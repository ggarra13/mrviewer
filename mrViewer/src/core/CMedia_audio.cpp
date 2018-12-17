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
 * @file   CMedia_audio.cpp
 * @author gga
 * @date   Sat Aug 25 00:14:54 2007
 * 
 * @brief  
 * 
 * 
 */


#define __STDC_LIMIT_MACROS
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
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}



#include <algorithm>  // for std::min, std::abs
#include <limits>

#include <ImfTimeCodeAttribute.h>
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


namespace {

  const char* kModule = "audio";

}


#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#  define AVCODEC_MAX_AUDIO_FRAME_SIZE 198000
#endif

//#undef DBG
//#define DBG(x) std::cerr << x << std::endl;

//#define DEBUG_AUDIO_PACKETS
// #define DEBUG_AUDIO_PACKETS_DETAIL
//#define DEBUG_AUDIO_STORES
// #define DEBUG_AUDIO_STORES_DETAIL
//#define DEBUG_DECODE
// #define DEBUG_QUEUE
// #define DEBUG_SEEK
// #define DEBUG
// #define DEBUG_AUDIO_SPLIT
//#define DEBUG_SEEK_AUDIO_PACKETS


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
    assert( unsigned(_audio_index) < _audio_info.size() );
    return _audio_info[ _audio_index ].stream_index;
}


// Returns the current audio stream
AVStream* CMedia::get_audio_stream() const
{
   if ( _audio_index < 0 ) return NULL;
   assert( _audio_index < _audio_info.size() );

   return _audio_info[ _audio_index ].context->streams[ audio_stream_index() ];
}

// Opens the audio codec associated to the current stream
void CMedia::open_audio_codec()
{
  AVStream *stream = get_audio_stream();
  if ( stream == NULL ) 
    {
      _audio_index = -1;
      return;
    }

  AVCodecParameters* ictx = stream->codecpar;
  if ( ictx == NULL )
  {
     IMG_ERROR( _("No codec context for audio stream.") );
      _audio_index = -1;
     return;
  }

  _audio_codec = avcodec_find_decoder( ictx->codec_id );
  if ( _audio_codec == NULL )
  {
      IMG_ERROR( _("No decoder found for audio stream. ID: ") 
                 << ictx->codec_id );
      _audio_index = -1;
     return;
  }
  
  _audio_ctx = avcodec_alloc_context3(_audio_codec);
  int r = avcodec_parameters_to_context(_audio_ctx, ictx);
  if ( r < 0 )
  {
      throw _("avcodec_copy_context failed for audio"); 
  }

  av_codec_set_pkt_timebase(_audio_ctx, get_audio_stream()->time_base);

  AVDictionary* opts = NULL;
  if (!av_dict_get(opts, "threads", NULL, 0))
       av_dict_set(&opts, "threads", "auto", 0);  // not "auto" nor "4"
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

    assert( get_audio_stream() != NULL );
    int64_t apts = frame2pts( get_audio_stream(), frame );
    if ( apts < 0 ) {
	apts = 0;
	//return frame;
    }

    
    AVPacket pkt = {0};

    // Clear the packet
    av_init_packet( &pkt );
    pkt.size = 0;
    pkt.data = NULL;

    unsigned int bytes_per_frame = audio_bytes_per_frame();
    assert( bytes_per_frame != 0 );

    unsigned int audio_bytes = 0;

    bool eof = false;
    unsigned counter = 0;
    unsigned packets_added = 0;

    int64_t last = last_frame() + _audio_offset;
    int64_t first = first_frame() + _audio_offset;
    if ( last < first ) last = first;

    while (!got_audio)
    {
        AVStream* stream = get_audio_stream();
        assert( stream != NULL );

        if (eof) {
            if (!got_audio && _audio_ctx &&
                _audio_ctx->codec->capabilities & AV_CODEC_CAP_DELAY) {
                av_init_packet(&pkt);
                pkt.dts = pkt.pts = apts;
                pkt.data = NULL;
                pkt.size = 0;
                pkt.stream_index = audio_stream_index();
                _audio_packets.push_back( pkt );
            }
         
            eof = false;
        }

        int error = av_read_frame( _acontext, &pkt );

        if ( error < 0 )
        {
            if ( error == AVERROR_EOF )
            {
                counter++;
                if ( counter >= _frame_offset ) 
                {
                    if ( is_seek || playback() == kBackwards )
                    {
                        if ( !got_audio && apts >= 0 )
                        {
                            DBG( "+++ ADD AUDIO SEEK END <<<<<<<<<<<< " );
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
         
            if ( is_seek )
            {
                if ( !got_audio && apts >= 0 ) {
                    DBG( "+++ ADD AUDIO SEEK END <<<<<<<<<<<< " );
                    _audio_packets.seek_end(apts);
                }
            }
         
            // av_packet_unref( &pkt );
         
            break;
        }

        if ( pkt.stream_index == audio_stream_index() )
        {
            int64_t pktframe = get_frame( get_audio_stream(), pkt );
         
            if ( playback() == kBackwards )
            {
                // Only add packet if it comes before seek frame
                if ( pktframe <= frame ) _audio_packets.push_back( pkt );
                if ( pktframe < dts ) dts = pktframe;
            }
            else
            {
                _audio_packets.push_back( pkt );
                if ( pktframe > dts ) dts = pktframe;
            }

            if ( !got_audio )
            {
                if ( pktframe > frame ) got_audio = true;
                else if ( pktframe == frame )
                {
                    audio_bytes += pkt.size;
                    if ( audio_bytes >= bytes_per_frame ) got_audio = true;
                }
                if ( is_seek && got_audio && apts >= 0 )
                {
                    DBG( "+++ ADD AUDIO SEEK END <<<<<<<<<<<< " );
                    _audio_packets.seek_end(apts);
                }
            }


#ifdef DEBUG_DECODE
            fprintf( stderr, "\t[avi] FETCH A f: %05" PRId64 
                     " audio pts: %07" PRId64 
                     " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
                     frame, pkt.pts, pkt.dts, pktframe );
#endif
            continue;
        }

        av_packet_unref( &pkt );
    }

    if ( dts > last_frame() ) dts = last_frame();
    else if ( dts < first_frame() ) dts = first_frame();

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
    if ( playback() == kBackwards && start > 0 ) --start;
    if ( start < _frame_start ) start = _frame_start;

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
        apts = frame2pts( get_audio_stream(), start + _audio_offset );
    }

    if ( !_seek_req && _playback == kBackwards )
    {
        if ( !got_audio )    _audio_packets.preroll(apts);
    }
    else
    {
        if ( !got_audio && apts >= 0 )    _audio_packets.seek_begin(apts);
    }

    bool got_video = true;
    bool got_subtitle = true;

    assert( _audio_packets.is_seek() || _audio_packets.is_preroll() );

    int64_t dts = queue_packets( frame, true, got_video,
                                        got_audio, got_subtitle );



    _expected_audio = frame+1;


    _adts = dts;
    _expected = dts+1;
    _seek_req = false;


#ifdef DEBUG_SEEK
    LOG_INFO( "AFTER SEEK:  D: " << _adts << " E: " << _expected_audio );
    debug_audio_packets(frame);
#endif
    return true;
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
  /* for PCM codecs, compute bitrate directly */
  switch(enc->codec_id) {
  case AV_CODEC_ID_PCM_S32LE:
  case AV_CODEC_ID_PCM_S32BE:
  case AV_CODEC_ID_PCM_U32LE:
  case AV_CODEC_ID_PCM_U32BE:
    bitrate = enc->sample_rate * enc->channels * 32;
    break;
  case AV_CODEC_ID_PCM_S24LE:
  case AV_CODEC_ID_PCM_S24BE:
  case AV_CODEC_ID_PCM_U24LE:
  case AV_CODEC_ID_PCM_U24BE:
  case AV_CODEC_ID_PCM_S24DAUD:
    bitrate = enc->sample_rate * enc->channels * 24;
    break;
  case AV_CODEC_ID_PCM_S16LE:
  case AV_CODEC_ID_PCM_S16BE:
  case AV_CODEC_ID_PCM_U16LE:
  case AV_CODEC_ID_PCM_U16BE:
    bitrate = enc->sample_rate * enc->channels * 16;
    break;
  case AV_CODEC_ID_PCM_S8:
  case AV_CODEC_ID_PCM_U8:
  case AV_CODEC_ID_PCM_ALAW:
  case AV_CODEC_ID_PCM_MULAW:
    bitrate = enc->sample_rate * enc->channels * 8;
    break;
  default:
    bitrate = (unsigned int) enc->bit_rate;
    break;
  }
  return bitrate;
}


unsigned int CMedia::audio_bytes_per_frame()
{
    unsigned int ret = 0;
    if ( !has_audio() ) return ret;

    int channels = _audio_ctx->channels;
    if (_audio_engine->channels() > 0 && channels > 0 ) {
        channels = FFMIN(_audio_engine->channels(), (unsigned)channels);
    }
    if ( channels <= 0 || _audio_format == AudioEngine::kNoAudioFormat)
        return ret;

    SCOPED_LOCK( _audio_mutex );
    int frequency = _audio_ctx->sample_rate;
    AVSampleFormat fmt = AudioEngine::ffmpeg_format( _audio_format );
    unsigned bps = av_get_bytes_per_sample( fmt );

    if ( _orig_fps <= 0.0f ) _orig_fps = _fps.load();
    ret = (unsigned int)( (double) frequency / _orig_fps ) * channels * bps;
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
  if ( separate ) c = _acontext;

  // Iterate through all the streams available
  for( unsigned i = 0; i < c->nb_streams; ++i ) 
    {
      // Get the codec context
      const AVStream* stream = c->streams[ i ];
      assert( stream != NULL );

      //const AVCodecContext* ctx = stream->codec;
      const AVCodecParameters* ctx = stream->codecpar;
      assert( ctx != NULL );

      // Determine the type and obtain the first index of each type
      switch( ctx->codec_type ) 
	{
	   case AVMEDIA_TYPE_SUBTITLE:
	   case AVMEDIA_TYPE_VIDEO:
	      break;
	   case AVMEDIA_TYPE_AUDIO:
	      {
		 audio_info_t s;
		 populate_stream_info( s, msg, c, ctx, i );
		 s.channels   = ctx->channels;
		 s.frequency  = ctx->sample_rate;
		 s.bitrate    = calculate_bitrate( stream, ctx );
	    
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
                                                           ctx->format );
		 if ( fmt ) s.format = fmt; 
		 
		 _audio_info.push_back( s );

		 if ( _audio_index < 0 && s.has_codec )
                     _audio_index = ((int) _audio_info.size()) - 1;
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
    }

  if ( msg.str().size() > 0 )
    {
      mrvALERT( filename() << msg.str() );
    }

  if ( _audio_index < 0 )
    {
       mrvALERT( filename() << _("\n\nNo audio or video stream in file") );
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

      double start = 0;
      if ( c->start_time != MRV_NOPTS_VALUE )
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
      if ( c->duration != MRV_NOPTS_VALUE )
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
   
   while((tag=av_dict_get(m, "", tag, AV_DICT_IGNORE_SUFFIX))) {
      std::string name = prefix;
      name += tag->key;
      if ( name == N_("timecode") || name == N_("Video timecode") ||
           name == N_("Timecode") || name == N_("timeCode") )
      {
          Imf::TimeCode t = str2timecode( tag->value );
          process_timecode( t );
          Imf::TimeCodeAttribute attr( t );
          _attrs.insert( std::make_pair( name, attr.copy() ) );
      }
      else
      {
          Imf::StringAttribute attr( tag->value );
          _attrs.insert( std::make_pair( name, attr.copy() ) );
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
   _audio_channels = 0;
   _audio_format = AudioEngine::kFloatLSB;

   if ( _acontext )
   {
      size_t num = _audio_info.size();
      for ( size_t i = 0; i < num; ++i )
      {
         if ( _audio_info[i].context == _acontext )
         {
            _audio_info.erase( _audio_info.begin() + i, 
                               _audio_info.begin() + i + 1);
            break;
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

  AVDictionary* params = NULL;
  AVInputFormat*   format = NULL;

  int error = avformat_open_input( &_acontext, file,
				   format, NULL );
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
    int max_frames = max_video_frames();

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
          return timercmp( a, b, > );
      }
  };

  typedef std::multimap< timeval, audio_cache_t::iterator,
                         customMore > TimedSeqMap;
  TimedSeqMap tmp;
  {
      audio_cache_t::iterator  it = _audio.begin();
      audio_cache_t::iterator end = _audio.end();
      for ( ; it != end; ++it )
      {
          tmp.insert( std::make_pair( (*it)->ptime(), it ) );
      }
  }



  unsigned count = 0;
  TimedSeqMap::iterator it = tmp.begin();
  typedef std::vector< audio_cache_t::iterator > IteratorList;
  IteratorList iters;
  for ( ; it != tmp.end(); ++it )
  {
      ++count;
      if ( count > max_frames )
      {
	  std::cerr << "count " << count << " "
		    << it->first.tv_sec << "." << it->first.tv_usec
		    << " " << (*(it->second))->frame() << std::endl;
          // Store this iterator to remove it later
          iters.push_back( it->second );
      }
  }

  IteratorList::iterator i = iters.begin();
  IteratorList::iterator e = iters.end();

  // We erase from greater to lower to avoid dangling iterators
  std::sort( i, e, std::greater<audio_cache_t::iterator>() );

  i = iters.begin();
  e = iters.end();
  for ( ; i != e; ++i )
  {
      _audio.erase( *i );
  }

}

//
// Limit the audio store to approx. max frames images on each side.
// We have to check both where frame is as well as where _adts is.
//
void CMedia::limit_audio_store(const int64_t frame)
{

    SCOPED_LOCK( _audio_mutex );

    int max_frames = max_video_frames();
    
    if ( max_audio_frames() > max_frames )
	max_frames = max_audio_frames();
    
    
  int64_t first, last;

    switch( playback() )
    {
	case kForwards:
	    first = frame;
	    last  = frame + max_frames;
	    break;
	case kBackwards:
	    first = frame - max_frames;
	    last  = frame;
	    break;
	default:
	    first = frame - max_frames;
	    last  = frame + max_frames;
	    break;
    }
  


    if ( _adts < first ) first = _adts;
    if ( _adts > last )   last = _adts;
#if 0
  if ( first > last ) 
  {
     int64_t tmp = last;
     last = first;
     first = tmp;
  }
#endif


  audio_cache_t::iterator end = _audio.end();
  _audio.erase( std::remove_if( _audio.begin(), end,
				NotInRangeFunctor( first, last ) ), end );

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

uint64_t get_valid_channel_layout(uint64_t channel_layout, int channels)
{
    if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
        return channel_layout;
    else
        return 0;
}



int CMedia::decode_audio3(AVCodecContext *ctx, int16_t *samples,
			  int* audio_size,
			  AVPacket *avpkt)
{   
    if ( ! _aframe ) 
    {
        if ( ! (_aframe = av_frame_alloc()) )
        {
            return AVERROR(ENOMEM);
        }
    }

    int got_frame = 0;
    int ret = -1;
    bool eof = false;

    int got_audio = 0;
    ret = decode( ctx, _aframe, &got_audio, avpkt, eof );
    if ( !got_audio ) return ret;
	
    av_assert2( _aframe->nb_samples > 0 );
    av_assert2( ctx->channels > 0 );
    int data_size = av_samples_get_buffer_size(NULL, ctx->channels,
					       _aframe->nb_samples,
					       ctx->sample_fmt, 0);
    if (*audio_size < data_size) {
	IMG_ERROR( _("Output buffer size is too small for "
		     "the current frame (")
		   << *audio_size << " < " << data_size << ")" );
	return AVERROR(EINVAL);
    }


    if ( ctx->sample_fmt == AV_SAMPLE_FMT_S16P ||
	 ctx->sample_fmt == AV_SAMPLE_FMT_S16 )
    {
	_audio_format = AudioEngine::kS16LSB;
    }

    AVSampleFormat fmt = AudioEngine::ffmpeg_format( _audio_format );
    if ( _audio_channels == 0 ) 
	_audio_channels = (unsigned short) ctx->channels;
    
    if ( ctx->sample_fmt != fmt  ||
	 unsigned(ctx->channels) != _audio_channels )
    {
	if (!forw_ctx)
	{
	    char buf[256];
	    
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

	    IMG_INFO( _("Create audio conversion from ") << buf 
		      << _(", channels ") << ctx->channels
		      << N_(", ") );
	    IMG_INFO( _("format ") 
		      << av_get_sample_fmt_name( ctx->sample_fmt )
		      << _(", sample rate ") << ctx->sample_rate
		      << _(" to") );

	    uint64_t out_ch_layout = in_ch_layout;
	    unsigned out_channels = ctx->channels;

	    if ( out_channels > _audio_channels && _audio_channels > 0 )
		out_channels = _audio_channels;
	    else
		_audio_channels = (unsigned short) ctx->channels;
	    

	    av_get_channel_layout_string( buf, 256, out_channels, 
					  out_ch_layout );

	    AVSampleFormat  out_sample_fmt = fmt;
	    AVSampleFormat  in_sample_fmt = ctx->sample_fmt;
	    int in_sample_rate = ctx->sample_rate;
	    int out_sample_rate = in_sample_rate;

	    IMG_INFO( buf << _(", channels ") << out_channels 
		      << _(", format " )
		      << av_get_sample_fmt_name( out_sample_fmt )
		      << _(", sample rate ")
		      << out_sample_rate);
	    
	    
	    forw_ctx  = swr_alloc_set_opts(NULL, out_ch_layout,
					   out_sample_fmt,  out_sample_rate,
					   in_ch_layout,  in_sample_fmt, 
					   in_sample_rate,
					   0, NULL);
	    if(!forw_ctx) {
		LOG_ERROR( _("Failed to alloc swresample library") );
		return 0;
	    }
	    
	    if(swr_init(forw_ctx) < 0)
	    {
		char buf[256];
		av_get_channel_layout_string(buf, 256, -1, in_ch_layout);
		LOG_ERROR( _("Failed to init swresample library with ") 
			   << buf << " " 
			   << av_get_sample_fmt_name(in_sample_fmt)
			   << _(" frequency: ") << in_sample_rate );
		return 0;
	    }
	}
	
	av_assert2( ret >= 0 );
	av_assert2( samples != NULL );
	av_assert2( _aframe->nb_samples > 0 );
	av_assert2( _aframe->extended_data != NULL );
	av_assert2( _aframe->extended_data[0] != NULL );
        
	int len2 = swr_convert(forw_ctx, (uint8_t**)&samples, 
			       _aframe->nb_samples, 
			       (const uint8_t **)_aframe->extended_data, 
			       _aframe->nb_samples );
	if ( len2 <= 0 )
	{
	    IMG_ERROR( _("Resampling failed") );
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

  av_assert0( !_audio_packets.is_seek_end( pkt ) );
  av_assert0( !_audio_packets.is_seek( pkt ) );
  av_assert0( !_audio_packets.is_flush( pkt ) );
  av_assert0( !_audio_packets.is_preroll( pkt ) );
  av_assert0( !_audio_packets.is_loop_end( pkt ) );
  av_assert0( !_audio_packets.is_loop_start( pkt ) );

  ptsframe = pts2frame( stream, pkt.dts );
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


#ifdef DEBUG
  if ( _audio_buf_used + pkt.size >= _audio_max )
    {
      IMG_ERROR( _("Too much audio used:") << _audio_buf_used  );
    }
#endif

  if ( _audio_packets.is_jump( pkt ) )
  {
      return kDecodeOK;
  }
  
  AVPacket pkt_temp;
  av_init_packet(&pkt_temp);
  pkt_temp.data = pkt.data;
  pkt_temp.size = pkt.size;


  av_assert0( _audio_buf != NULL );
  av_assert0( pkt.size + _audio_buf_used < _audio_max );

  int audio_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;  //< correct
  av_assert0( pkt_temp.size <= audio_size );

  if ( _audio_buf_used + audio_size > _audio_max )
  {
      aligned16_uint8_t* old = _audio_buf;
      _audio_buf = new aligned16_uint8_t[ _audio_max + audio_size ];
      memcpy( _audio_buf, old, _audio_max );
      delete [] old;
      _audio_max += audio_size;
  }

  {
      // Decode the audio into the buffer
      assert( _audio_buf_used % 16 == 0 );

      int ret = decode_audio3( _audio_ctx, 
                               ( int16_t * )( (char*)_audio_buf + 
                                              _audio_buf_used ), 
                               &audio_size, &pkt_temp );
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
	   pkt_temp.size = 0;
           return kDecodeMissingSamples;
	}

      assert( audio_size > 0 );
      assert( audio_size + _audio_buf_used <= _audio_max );


      _audio_buf_used += audio_size;
      do {

	ret = decode_audio3( _audio_ctx, 
			     ( int16_t * )( (char*)_audio_buf + 
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
    
    unsigned int bytes_per_frame = audio_bytes_per_frame();
    assert( bytes_per_frame != 0 );
    
    if ( last == first_frame() || playback() == kStopped )
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
                      << "audio frame: " << audio_frame << std::endl
                      << "index: " << index << std::endl
                      << "  bpf: " << bytes_per_frame << std::endl
                      << " used: " << _audio_buf_used << std::endl
                      << "  max: " << _audio_max << std::endl;
	}
#endif

        index += store_audio( last,
                              (boost::uint8_t*)_audio_buf + index,
                              bytes_per_frame );


        if ( last >= frame ) got_audio = kDecodeOK;

        assert( bytes_per_frame <= _audio_buf_used );
        _audio_buf_used -= bytes_per_frame;

        ++last;

    }
  


    if (_audio_buf_used > 0  )
    {
        //
        // NOTE: audio buffer must remain 16 bits aligned for ffmpeg.
        memmove( _audio_buf, _audio_buf + index, _audio_buf_used );
    }

    return got_audio;
}


// Return the number of frames cached for jog/shuttle
// or 0 for no cache or numeric_limits<int>max() for full cache
int CMedia::max_audio_frames()
{
    if ( _audio_cache_size > 0 )
        return _audio_cache_size;
    else if ( _audio_cache_size == 0 )
        return int( fps()*2 );
    else
        return std::numeric_limits<int>::max();
}




void CMedia::audio_stream( int idx )
{
  if ( idx < -1 || unsigned(idx) >= number_of_audio_streams() )
      idx = -1;

  if ( _right_eye ) _right_eye->audio_stream(idx);


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
      swr_free( &forw_ctx );
      forw_ctx = NULL;
      _audio_channels = 0;
      _audio_format = AudioEngine::kFloatLSB;
    }

  clear_stores();

  _audio_index = idx;

  if ( _audio_index >= 0 )
    {
      open_audio_codec();
      _audio_start = true;
      seek( _frame );
      _audio_start = false;
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
		     const boost::uint8_t* buf, const unsigned int size )
{

  assert( buf != NULL );
  if ( buf == NULL || size == 0 )
  {
      IMG_ERROR( _("store_audio: Invalid data") );
      return 0;
  }

  // Ge2t the audio codec context
  AVStream* stream = get_audio_stream();
  if ( !stream || !_audio_ctx ) return 0;


  SCOPED_LOCK( _audio_mutex );
  //  assert( audio_frame <= last_frame()  );
  //  assert( audio_frame >= first_frame() );

  int64_t f = audio_frame;
  _audio_last_frame = f;

  // Get the audio info from the codec context
  _audio_channels = (unsigned short)_audio_ctx->channels;
  const unsigned short channels = _audio_channels;
  const int frequency = _audio_ctx->sample_rate;

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

#if 1  // needed
        audio_cache_t::iterator at = std::lower_bound( _audio.begin(),
                                                       end,
                                                       f,
                                                       LessThanFunctor() );
#else
        audio_cache_t::iterator at = std::find_if( _audio.begin(), end,
                                                   EqualFunctor(f) );
#endif
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

    if ( !got_audio && frame != _expected_audio )
    {
        DBG( "FRAME(" << frame << ") != EXPECTED (" << _expected_audio << ")" );
        bool ok = seek_to_position( frame );
        DBG( "FRAME(" << frame << ") NEW EXPECTED (" << _expected_audio << ")" );
        if (ok) return;
    }

    DBG( ">>>>>>>> FRAME " << frame << " IS EXPECTED " << _expected_audio );

  // if ( !got_audio && !_audio.empty() )
  //   {
  //      got_audio = in_audio_store( frame );
  //   }



  bool got_video = true;
  bool got_subtitle = true;

  DBG( "queue packets " << frame << " is_seek " << false
       << " got audio " << got_audio );
  int64_t dts = frame;

  queue_packets( frame, false, got_video, got_audio, 
                 got_subtitle );

  DBG( "queue packets return " << dts );

  int64_t last = last_frame() + _audio_offset;
  int64_t first = first_frame() + _audio_offset;

  if ( dts > last ) dts = last;
  else if ( dts < first ) dts = first;

  _adts = dts;

  _expected_audio = dts + 1;
  DBG( "DTS " << dts << " EXPECTED " << _expected_audio );

}



void CMedia::audio_initialize()
{
  if ( _audio_engine ) return;

  av_log_set_level(-99);

  _audio_engine = mrv::AudioEngine::factory();
  _audio_channels = (unsigned short) _audio_engine->channels();
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
  close_audio();

  DBG("open audio - audio closed" );

  AudioEngine::AudioFormat format = AudioEngine::kFloatLSB;

  // Avoid conversion to float if unneeded
  if ( _audio_ctx->sample_fmt == AV_SAMPLE_FMT_S16P ||
       _audio_ctx->sample_fmt == AV_SAMPLE_FMT_S16 )
  {
     format = AudioEngine::kS16LSB;
  }

  if ( _fps > 100.0 )
  {
      // At this speed, we consume buffers really fast.  Use more buffers
      // This fixes a bug in Windows where sound would not play.
      // On Linux, this does nothing.
      DBG("16 audio buffers" );
      _audio_engine->buffers( 16 );
  }

  AVSampleFormat ft = AudioEngine::ffmpeg_format( format );
  unsigned bps = av_get_bytes_per_sample( ft ) * 8;

  bool ok = false;
  int ch = channels;
  for ( int fmt = format; fmt > 0; fmt -= 2 ) // to skip be/le versions
  {
      ok = _audio_engine->open( ch, nSamplesPerSec,
                                (AudioEngine::AudioFormat)fmt, bps );
      if ( ok ) break;
  }

  _audio_format = _audio_engine->format();
  _audio_channels = _audio_engine->channels();
  _samples_per_sec = nSamplesPerSec;

  return ok;
}


bool CMedia::play_audio( const mrv::audio_type_ptr& result )
{
  
  double speedup = _play_fps / _fps;
  unsigned nSamplesPerSec = unsigned( (double) result->frequency() * speedup );
  if ( !_audio_engine || nSamplesPerSec != _samples_per_sec ||
       result->channels() != _audio_channels ||
       _audio_format == AudioEngine::kNoAudioFormat ||
       AudioEngine::device_index() != AudioEngine::old_device_index())
    {
      SCOPED_LOCK( _audio_mutex );
      if ( ! open_audio( result->channels(), nSamplesPerSec ) )
	{
	  IMG_ERROR( _("Could not open audio driver") );
	  _audio_index = -1;
	  return false;
	}

    }

  if ( ! _audio_engine ) {
      IMG_ERROR( _("Could not initialize audio engine" ) );
      return false;
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
  audio_type_ptr result;


  {
#ifdef DEBUG_AUDIO_PACKETS
      debug_audio_packets(frame, _right_eye ? "RFIND" : "FIND");
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RFIND" : "FIND");
#endif

    SCOPED_LOCK( _audio_mutex );

    _audio_frame = frame;

    if ( frame < first_frame() )
        return true;

#if 1 // needed
    audio_cache_t::iterator end = _audio.end();
    audio_cache_t::iterator i = std::lower_bound( _audio.begin(), end, 
						  frame, LessThanFunctor() );
#else
    audio_cache_t::iterator end = _audio.end();
    audio_cache_t::iterator i = std::find_if( _audio.begin(), end, 
                                              EqualFunctor(frame) );
#endif
    if ( i == end )
      {
	IMG_WARNING( _("Audio frame ") << frame << _(" not found") );
	return false;
      }

    result = *i;

  }

  assert( result->frame() == frame );
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

  //set_clock_at(&audclk, _audio_pts, 0, audio_callback_time / 1000000.0 );
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
  SCOPED_LOCK( _audio_mutex);
  
  if ( _audio_engine ) _audio_engine->close();
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
  _audio_engine->volume( v );
}


CMedia::DecodeStatus 
CMedia::handle_audio_packet_seek( int64_t& frame, 
				  const bool is_seek )
{
#ifdef DEBUG_AUDIO_PACKETS
    debug_audio_packets(frame, _right_eye ? "RDOSEEK" : "DOSEEK");
#endif

#ifdef DEBUG_AUDIO_STORES
    debug_audio_stores(frame, _right_eye ? "RDOSEEK" : "DOSEEK");
#endif

 
  bool skip = false;

  if ( is_seek && _audio_packets.is_seek() )
  {
     _audio_packets.pop_front();  // pop seek begin packet
  }
  else if ( !is_seek && _audio_packets.is_preroll() )
  {
     _audio_packets.pop_front();
  }
  else
      IMG_ERROR( _("Audio packet is unknown, expected seek or preroll") );

  DecodeStatus got_audio = kDecodeMissingFrame;

  assert( _audio_buf_used == 0 );

  if ( !_audio_packets.empty() && !_audio_packets.is_seek_end() )
  {
      const AVPacket& pkt = _audio_packets.front();
      // int64_t pts = pkt.dts + pkt.duration;
      _audio_last_frame = pts2frame( get_audio_stream(), pkt.dts );
  }


  int64_t last = _audio_last_frame;


  while ( !_audio_packets.empty() && !_audio_packets.is_seek_end() )
    {
      const AVPacket& pkt = _audio_packets.front();
      int64_t f = pts2frame( get_audio_stream(), pkt.dts );


      DecodeStatus status;

      if ( !in_audio_store( f ) )
      {
          // if ( (status = decode_audio( frame, pkt )) == kDecodeOK )
          if ( (status = decode_audio( f, pkt )) == kDecodeOK )
              got_audio = kDecodeOK;
      }
      else
      {
          status = decode_audio_packet( last, f, pkt ); //frame
          if ( status != kDecodeOK )
              LOG_WARNING( _( "decode_audio_packet failed for frame " )
                           << frame );
      }

      _audio_packets.pop_front();
    }

  if ( _audio_packets.empty() ) {
      IMG_ERROR( _("Audio packets empty at end of seek") );
      return kDecodeError;
  }


  if ( is_seek )
    {
      assert( !_audio_packets.empty() );
      const AVPacket& pkt = _audio_packets.front();
      frame = pts2frame( get_audio_stream(), pkt.dts ) + _audio_offset;
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

    audio_callback_time = av_gettime_relative();

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



  while ( got_audio != kDecodeOK && !_audio_packets.empty() )
  {
      assert( !_audio_packets.is_seek_end() );
      if ( _audio_packets.is_flush() )
	{
	  flush_audio();
          assert( !_audio_packets.empty() );
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
          assert( !_audio_packets.empty() );
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
          assert( !_audio_packets.empty() );
          _audio_packets.pop_front();
          return kDecodeLoopEnd;
      }
      else if ( _audio_packets.is_seek()  )
	{
            clear_stores();  // audio stores MUST be cleared when seeked
	    //_audio_buf_used = 0;
            got_audio = handle_audio_packet_seek( frame, true );
            continue;
	}
      else if ( _audio_packets.is_preroll() )
	{
	   bool ok = in_audio_store( frame );
	   if ( ok ) {
               SCOPED_LOCK( _audio_mutex );
               assert( !_audio_packets.empty() );
               AVPacket& pkt = _audio_packets.front();
               int64_t pktframe = get_frame( get_audio_stream(), pkt )
                                  - _frame_offset;
	      if ( pktframe >= frame )
	      {
		 _audio_buf_used = 0;
		 got_audio = handle_audio_packet_seek( frame, false );
	      }
	      return kDecodeOK;
	   }

	   _audio_buf_used = 0;
	   got_audio = handle_audio_packet_seek( frame, false );
	   continue;
	}
      else if ( _audio_packets.is_jump() )
      {
	  _audio_packets.pop_front();
	  return kDecodeOK;
      }
      else
      {
          assert( !_audio_packets.empty() );
	  AVPacket& pkt = _audio_packets.front();

	  int64_t pktframe = get_frame( get_audio_stream(), pkt );
#if 0
          // This does not work as decode_audio_packet may decode more
          // than one frame of audio (see Essa.wmv)
	  bool ok = in_audio_store( frame );
	  if ( ok ) 
          {
              got_audio = decode_audio_packet( pktframe, frame, pkt );
              assert( !_audio_packets.empty() );
              _audio_packets.pop_front();
              continue;
          }
#endif
	  got_audio = decode_audio( frame, pkt );
          assert( !_audio_packets.empty() );
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
          clear_packets();

      fetch_audio( x );
  }

  // Seeking done, turn flag off
  if ( stopped() || saving() )
  {

     if ( has_audio() && !got_audio )
     {
         int64_t f = x;
         DecodeStatus status = decode_audio( f );
         if ( status != kDecodeOK && status != kDecodeNoStream )
             IMG_ERROR( _("Decode audio error: ") 
                        << decode_error( status ) 
			<< _(" for frame ") << x );
         if ( status != kDecodeNoStream && !_audio_start  )
             find_audio( x );
         else
             _audio_start = false;
     }
  }

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

  if (detail)
  {
     for ( ; iter != last; ++iter )
     {
	int64_t f = (*iter)->frame();
	if ( f == frame )  std::cerr << "P";
	if ( f == _adts )   std::cerr << "D";
	if ( f == _frame ) std::cerr << "F";
	std::cerr << " " << f;
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

  mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
  SCOPED_LOCK( apm );

  mrv::PacketQueue::const_iterator iter = _audio_packets.begin();
  mrv::PacketQueue::const_iterator last = _audio_packets.end();
  std::cerr << this << std::dec << " " << name()
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

     int64_t last_frame = std::numeric_limits< int64_t >::min();

     for ( ; iter != last; ++iter )
     {
	if ( _audio_packets.is_flush( *iter ) )
	{
	   std::cerr << "* "; continue;
	}
	else if ( _audio_packets.is_loop_end( *iter ) )
	{
	   std::cerr << "Le(" << iter->dts << ")"; continue;
	}
	else if ( _audio_packets.is_loop_start( *iter ) )
	{
	   std::cerr << "Ls(" << iter->dts << ")"; continue;
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
	      std::cerr << "+ERROR:" << f << "+";
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
	   last_frame = f;
	   if ( in_preroll || in_seek ) counter++;
	}
     }
  }

  std::cerr << std::endl;

}




} // namespace mrv
