/**
 * @file   CMedia_audio.cpp
 * @author gga
 * @date   Sat Aug 25 00:14:54 2007
 * 
 * @brief  
 * 
 * 
 */


#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>

#include "mrvPacketQueue.h"


#if defined(WIN32) || defined(WIN64)
#  include <direct.h>
#  define getcwd _getcwd
#else
#  include <unistd.h>
#endif

#include <cstdio>     // for snprintf
#include <cassert>    // for assert


extern "C" {
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}



#include <iostream>
#include <algorithm>  // for std::min, std::abs
#include <limits>


#include "CMedia.h"
#include "mrvIO.h"
#include "mrvException.h"
#include "mrvPlayback.h"
#include "mrvFrameFunctors.h"
#include "mrvThread.h"
#include "mrvAudioEngine.h"
#include "mrvI8N.h"
#include "mrvOS.h"


namespace {

  const char* kModule = "audio";

}

#define IMG_WARNING(x) LOG_WARNING( name() << " - " << x ) 
#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )
#define IMG_INFO(x) LOG_INFO( name() << " - " << x )

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

//#define DEBUG_AUDIO_PACKETS
//#define DEBUG_AUDIO_STORES
// #define DEBUG_AUDIO_STORES_DETAIL
// #define DEBUG_DECODE
//#define DEBUG_SEEK
// #define DEBUG
// #define DEBUG_AUDIO_SPLIT
//#define DEBUG_SEEK_AUDIO_PACKETS


// #define FFMPEG_STREAM_BUG_FIX

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

#if defined(_WIN32) || defined(_WIN64)
AudioEngine::AudioFormat kSampleFormat = mrv::AudioEngine::kFloatLSB;
#else
AudioEngine::AudioFormat kSampleFormat = mrv::AudioEngine::kFloatLSB;
#endif



/** 
 * Clear (audio) packets
 * 
 */
void CMedia::clear_packets()
{
  _video_packets.clear();
  _subtitle_packets.clear();
  _audio_packets.clear();
  _audio_buf_used = 0;
}




/** 
 * Returns the FFMPEG stream index of current audio channel.
 * 
 * @return 
 */
int CMedia::audio_stream_index() const
{
  assert( _audio_index >= 0 );
  assert( unsigned(_audio_index) < _audio_info.size() );
  return _audio_info[ _audio_index ].stream_index;
}


// Returns the current audio stream
AVStream* CMedia::get_audio_stream() const
{
   if ( _audio_index < 0 ) return NULL;

   if ( _acontext )
      return _acontext->streams[ audio_stream_index() ];
   else if ( _context )
      return _context->streams[ audio_stream_index() ];
   else
      return NULL;
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

  AVCodecContext* ctx = stream->codec;
  if ( ctx == NULL )
  {
     IMG_ERROR( _("No codec context for audio stream.") );
      _audio_index = -1;
     return;
  }

  _audio_codec = avcodec_find_decoder( ctx->codec_id );
  if ( _audio_codec == NULL )
  {
     IMG_ERROR( _("No decoder found for audio stream. ID: ") 
		<< ctx->codec_id );
      _audio_index = -1;
     return;
  }
  
  AVDictionary* opts = NULL;

  if ( avcodec_open2( ctx, _audio_codec, NULL ) < 0 )
    {
       IMG_ERROR( _("Could not open audio codec.") );
      _audio_index = -1;
    }
  else
    {
      if ( !_audio_buf ) {
	_audio_max = AVCODEC_MAX_AUDIO_FRAME_SIZE * 10;
	_audio_buf = new aligned16_uint8_t[ _audio_max ];
	assert( (((unsigned long)_audio_buf) % 16) == 0 );
	memset( _audio_buf, 0, _audio_max );
      }
    }
}


// Seek to the requested frame
bool CMedia::seek_to_position( const boost::int64_t frame )
{
   if ( _acontext == NULL ) return true;

  boost::int64_t offset = boost::int64_t( (frame * AV_TIME_BASE ) / fps() );

  int flags = 0;
  flags &= ~AVSEEK_FLAG_BYTE;

  int ret = avformat_seek_file( _acontext, -1, INT64_MIN, offset, 
   				INT64_MAX, flags );
  if (ret < 0)
  {
     IMG_ERROR( "Could not seek to frame " << frame );
     return false;
  }

  bool got_audio   = !has_audio();
  

  boost::int64_t apts = 0;

  mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
  SCOPED_LOCK( apm );

  if ( !got_audio ) {
    apts = frame2pts( get_audio_stream(), frame );
  }

  if ( _seek_req || _playback != kBackwards )
    {
      if ( !got_audio )    _audio_packets.seek_begin(apts);
    }
  else
    {
      if ( !got_audio )    _audio_packets.preroll(apts);
    }


  boost::int64_t dts = frame;


  unsigned int bytes_per_frame = audio_bytes_per_frame();
  unsigned int audio_bytes = 0;



  // Clear the packet
  bool ok = false;
  AVPacket pkt;
  av_init_packet( &pkt );

  while (!got_audio) {

     int error = av_read_frame( _acontext, &pkt );
     if ( error < 0 )
     {
	int err = _acontext->pb ? _acontext->pb->error : 0;
	if ( err != 0 )
	{
	   IMG_ERROR("seek: Could not read frame " << frame << " error: "
		     << strerror(err) );
	}

	if ( !got_audio ) _audio_packets.seek_end(apts);
	
	ok = false;
	break;
     }

     if ( has_audio() && pkt.stream_index == audio_stream_index() )
     {
	boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );
	
	if ( playback() == kBackwards )
	{
	   if ( pktframe <= frame ) _audio_packets.push_back( pkt );
	   if ( pktframe < dts ) dts = pktframe;
	}
	else
	{
	   if ( pktframe <= last_frame() ) _audio_packets.push_back( pkt );
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
	   if ( got_audio ) _audio_packets.seek_end(apts);
	}
	 

#ifdef DEBUG_SEEK_AUDIO_PACKETS
	fprintf( stderr, "f: %05" PRId64 " audio pts: %07" PRId64 
		 " dts: %07" PRId64 " as frame: %05" PRId64 " size: %d\n",
		 frame, pkt.pts, pkt.dts, pktframe, pkt.size );
#endif
	continue;
     }

     av_free_packet( &pkt );
  }


  //
  // When pre-rolling, make sure new dts is not at a distance bigger
  // than our image/audio cache.
  //
  if ( !_seek_req )
    {
      int64_t diff = (dts - _dts) * _playback;

      unsigned max_frames = 1;
      if ( has_video() )
	{
	  max_frames = max_video_frames();
	}
      else if ( has_audio() )
	{
	  max_frames = max_audio_frames();
	}

      if ( diff > max_frames )
	{
	  dts = _dts + int64_t(max_frames) * _playback;
	}
    }

  _dts = dts;
  _expected = dts + 1;
  _seek_req = false;


#ifdef DEBUG_SEEK
  LOG_INFO( "AFTER SEEK:  D: " << _dts << " E: " << _expected );
  debug_audio_packets(frame);
#endif
  return ok;
}

/** 
 * Closed the audio codec if one is open.
 * 
 */
void CMedia::close_audio_codec()
{
  AVStream *stream = get_audio_stream();
  if ( stream && stream->codec )
    avcodec_close( stream->codec );
}


/** 
 * Given an audio codec context, calculate the approximate bitrate.
 * 
 * @param enc   codec context
 * 
 * @return bitrate
 */
unsigned int CMedia::calculate_bitrate( const AVCodecContext* enc )
{
  unsigned int bitrate;
  /* for PCM codecs, compute bitrate directly */
  switch(enc->codec_id) {
  case CODEC_ID_PCM_S32LE:
  case CODEC_ID_PCM_S32BE:
  case CODEC_ID_PCM_U32LE:
  case CODEC_ID_PCM_U32BE:
    bitrate = enc->sample_rate * enc->channels * 32;
    break;
  case CODEC_ID_PCM_S24LE:
  case CODEC_ID_PCM_S24BE:
  case CODEC_ID_PCM_U24LE:
  case CODEC_ID_PCM_U24BE:
  case CODEC_ID_PCM_S24DAUD:
    bitrate = enc->sample_rate * enc->channels * 24;
    break;
  case CODEC_ID_PCM_S16LE:
  case CODEC_ID_PCM_S16BE:
  case CODEC_ID_PCM_U16LE:
  case CODEC_ID_PCM_U16BE:
    bitrate = enc->sample_rate * enc->channels * 16;
    break;
  case CODEC_ID_PCM_S8:
  case CODEC_ID_PCM_U8:
  case CODEC_ID_PCM_ALAW:
  case CODEC_ID_PCM_MULAW:
    bitrate = enc->sample_rate * enc->channels * 8;
    break;
  default:
    bitrate = enc->bit_rate;
    break;
  }
  return bitrate;
}


unsigned int CMedia::audio_bytes_per_frame()
{
   unsigned int ret = 0;
   if ( has_audio() )
    {
      AVStream* stream = get_audio_stream();
      AVCodecContext* ctx = stream->codec;
      
      unsigned int channels = ctx->channels;
      if (_audio_engine->channels() > 0 ) {
      	 channels = FFMIN(_audio_engine->channels(), channels);
      }

      int frequency = ctx->sample_rate;
      AVSampleFormat fmt = AudioEngine::ffmpeg_format( _audio_format );
      unsigned bps = av_get_bytes_per_sample( fmt );

      ret = (unsigned int)( (double) frequency / _fps ) * channels * bps;
    }
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

   _audio_info.clear();

  std::ostringstream msg;

  AVFormatContext* c = _context;
  if ( separate ) c = _acontext;

  // Iterate through all the streams available
  for( unsigned i = 0; i < c->nb_streams; ++i ) 
    {
      // Get the codec context
      const AVStream* stream = c->streams[ i ];
      assert( stream != NULL );

      const AVCodecContext* ctx = stream->codec;
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
		 s.bitrate    = calculate_bitrate( ctx );
	    
		 if ( stream->metadata )
		 {
		    AVDictionaryEntry* lang = av_dict_get(stream->metadata, 
							  "language", NULL, 0);
		    if ( lang && lang->value )
		       s.language = lang->value;
		 }
		 else
		 {
		    s.language = "en";
		 }

		 const char* fmt = av_get_sample_fmt_name( ctx->sample_fmt );
		 if ( fmt ) s.format = fmt; 
		 
		 _audio_info.push_back( s );
		 if ( _audio_index < 0 && s.has_codec )
		    _audio_index = 0;
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



  // _fps = _play_fps = calculate_fps( stream );


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

  _frameStart = boost::int64_t( _fps * start ) + 1;

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

      duration = boost::int64_t( length * _fps );
    }


  _frame = _dts = _expected = _frameStart;


  //
  // Miscellaneous information
  //
  AVDictionary* m = NULL;

  stream = get_audio_stream();
  if ( stream->metadata ) m = stream->metadata;

  dump_metadata( c->metadata );

}

void CMedia::dump_metadata( AVDictionary *m )
{
   if(!m) return;

   AVDictionaryEntry* tag = NULL;
   
   while((tag=av_dict_get(m, "", tag, AV_DICT_IGNORE_SUFFIX))) {
      _iptc.insert( std::make_pair( tag->key, tag->value ) ); 
   }
}


/** 
 * Attach an audio file to this image.
 * 
 * @param file  audio file to attach. 
 */
void CMedia::audio_file( const char* file )
{

   SCOPED_LOCK( _audio_mutex );

   audio_stream( -1 );
   _audio_file.clear();
   close_audio();
   close_audio_codec();
   _audio_packets.clear();
   swr_free( &forw_ctx );
   forw_ctx = NULL;
   _audio_channels = 0;
   _audio_format = AudioEngine::kFloatLSB;
   if ( _acontext )
   {
      avformat_close_input( &_acontext );
      _acontext = NULL;
   }

   if ( file == NULL )
   {
      file = filename();
   }

  AVDictionary* params = NULL;
  AVInputFormat*   format = NULL;

  int error = avformat_open_input( &_acontext, file,
				   format, NULL );
  if ( error < 0 || _acontext == NULL )
  {
     mrvALERT( file << _(": Could not open filename.") );
     return;
  }

  error = avformat_find_stream_info( _acontext, NULL );

  if ( error < 0 )
  {
     mrvALERT( file << _(": Could not find stream info.") );
     return;
  }

  _expected = 0;
  _audio_file = file;


  populate_audio();
}



//
// Limit the audio store to approx. max frames images on each side.
// We have to check both where frame is as well as where _dts is.
//
void CMedia::limit_audio_store(const boost::int64_t frame)
{
  boost::int64_t first, last;

  switch( playback() )
    {
    case kBackwards:
      first = frame - max_audio_frames();
      last  = frame;
      if ( _dts < first ) first = _dts;
      break;
    case kForwards:
      first = frame;
      last  = frame + max_audio_frames();
      if ( _dts > last )   last = _dts;
      break;
    default:
      first = frame - max_audio_frames();
      last  = frame + max_audio_frames();
      if ( _dts > last )   last = _dts;
      if ( first < first_frame() ) first = first_frame();
      if ( last  > last_frame() )   last = last_frame();
      break;
    }
  
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
#ifdef DEBUG_DECODE
   std::cerr << ">>>>>>>>>>> clear audio stores" << std::endl;
#endif

  SCOPED_LOCK( _audio_mutex );

  _audio.clear();
  _audio_buf_used = 0;
}

static inline
int64_t get_valid_channel_layout(int64_t channel_layout, int channels)
{
    if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
        return channel_layout;
    else
        return 0;
}

template< typename T >
struct Swizzle
{
     T* ptr;
     unsigned last;
     
     inline Swizzle(void* data, unsigned data_size ) :
     ptr( (T*) data ),
     last( data_size / 8*sizeof(T) )
     {
     }

     inline void do_it() 
     {
	unsigned i;
	T tmp;
	for (i = 0; i < last; i++, ptr += 6) {	
	   tmp = ptr[2]; ptr[2] = ptr[4]; ptr[4] = tmp;
	   tmp = ptr[3]; ptr[3] = ptr[5]; ptr[5] = tmp;
	}
     }
};

int CMedia::decode_audio3(AVCodecContext *ctx, int16_t *samples,
			  int *frame_size_ptr,
			  AVPacket *avpkt)
{   
   AVFrame frame = { { 0 } };
   int ret, got_frame = 0;
   

    ret = avcodec_decode_audio4(ctx, &frame, &got_frame, avpkt);

    if (ret >= 0 && got_frame) {
       int data_size = av_samples_get_buffer_size(NULL, ctx->channels,
						  frame.nb_samples,
						  ctx->sample_fmt, 0);
        if (*frame_size_ptr < data_size) {
	   IMG_ERROR( "decode_audio3 - Output buffer size is too small for "
		      "the current frame (" 
		      << *frame_size_ptr << " < " << data_size << ")" );
	   return AVERROR(EINVAL);
        }


	if ( ctx->sample_fmt == AV_SAMPLE_FMT_S16P ||
	     ctx->sample_fmt == AV_SAMPLE_FMT_S16 )
	{
	   _audio_format = AudioEngine::kS16LSB;
	}

	AVSampleFormat fmt = AudioEngine::ffmpeg_format( _audio_format );

	if ( ( ctx->sample_fmt != fmt  ||
	       unsigned(ctx->channels) > _audio_channels ) && 
	       _audio_channels > 0 )
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

	      IMG_INFO("Create audio conversion from " << buf 
		       << ", channels " << ctx->channels << ", " );
	      IMG_INFO( "format " 
		       << av_get_sample_fmt_name( ctx->sample_fmt ) 
			<< ", sample rate " << ctx->sample_rate << " to" );

	      uint64_t out_ch_layout = in_ch_layout;
	      unsigned out_channels = ctx->channels;

	      if ( out_channels > _audio_channels && _audio_channels > 0 )
	      	 out_channels = _audio_channels;
	      else
		 _audio_channels = ctx->channels;


	      av_get_channel_layout_string( buf, 256, out_channels, 
					    out_ch_layout );

	      AVSampleFormat  out_sample_fmt = fmt;
	      AVSampleFormat  in_sample_fmt = ctx->sample_fmt;
	      int in_sample_rate = ctx->sample_rate;
	      int out_sample_rate = in_sample_rate;

	      IMG_INFO( buf << ", channels " << out_channels << ", format " 
			<< av_get_sample_fmt_name( out_sample_fmt )
			<< ", sample rate " 
			<< out_sample_rate);
	      

	      forw_ctx  = swr_alloc_set_opts(NULL, out_ch_layout,
					     out_sample_fmt,  out_sample_rate,
					     in_ch_layout,  in_sample_fmt, 
					     in_sample_rate,
					     0, NULL);
	      if(!forw_ctx) {
		 LOG_ERROR("Failed to alloc swresample library");
		 return 0;
	      }
	      if(swr_init(forw_ctx) < 0)
	      {
		 char buf[256];
		 av_get_channel_layout_string(buf, 256, -1, in_ch_layout);
		 LOG_ERROR( "Failed to init swresample library with " 
			    << buf << " " 
			    << av_get_sample_fmt_name(in_sample_fmt)
			    << " frequency: " << in_sample_rate );
		 return 0;
	      }
	   }
	   


	   int len2 = swr_convert(forw_ctx, (uint8_t**)&samples, 
				  _audio_max, 
				  (const uint8_t **)frame.extended_data, 
				  frame.nb_samples );
	   if ( len2 < 0 )
	   {
	      IMG_ERROR( "resampling failed" );
	      return 0;
	   }


	   data_size = ( len2 * _audio_channels *
	   		 av_get_bytes_per_sample( fmt ) );

	   if ( _audio_channels >= 6 )
	   {
	      if ( fmt == AV_SAMPLE_FMT_FLT )
	      {
		 Swizzle<float> t( samples, data_size );
		 t.do_it();
	      }
	      else if ( fmt == AV_SAMPLE_FMT_S32 )
	      {
		 Swizzle<int32_t> t( samples, data_size );
		 t.do_it();
	      }
	      else if ( fmt == AV_SAMPLE_FMT_S16 )
	      {
		 Swizzle<int16_t> t( samples, data_size );
		 t.do_it();
	      }
	   }

	}
	else
	{
	   if ( _audio_channels > 0 )
	   {
 	      memcpy(samples, frame.extended_data[0], data_size);
	   }
	}

        *frame_size_ptr = data_size;
    } else {
        *frame_size_ptr = 0;
    }
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
CMedia::decode_audio_packet( boost::int64_t& ptsframe,
			     const boost::int64_t frame, 
			     const AVPacket& pkt )
{

  AVStream* stream = get_audio_stream();
  if ( !stream ) return kDecodeNoStream;

  // Get the audio codec context
  AVCodecContext* ctx = stream->codec;


  assert( !_audio_packets.is_seek( pkt ) );
  assert( !_audio_packets.is_flush( pkt ) );
  assert( !_audio_packets.is_preroll( pkt ) );
  assert( !_audio_packets.is_loop_end( pkt ) );
  assert( !_audio_packets.is_loop_start( pkt ) );

  ptsframe = get_frame( stream, pkt );

  // Make sure audio frames are continous during playback to 
  // accomodate weird sample rates not evenly divisable by frame rate
  if ( playback() != kStopped && _audio_buf_used != 0 && (!_audio.empty()) )
    {
       ptsframe = _audio_last_frame + 1;
      // assert( ptsframe <= last_frame() );
    }


#ifdef DEBUG
  if ( _audio_buf_used + pkt.size >= _audio_max )
    {
      IMG_ERROR( _("Too much audio used:") << _audio_buf_used  );
    }
#endif

  AVPacket pkt_temp;
  av_init_packet(&pkt_temp);
  pkt_temp.data = pkt.data;
  pkt_temp.size = pkt.size;



  assert( _audio_buf != NULL );
  assert( pkt.size + _audio_buf_used < _audio_max );

  int audio_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;  //< correct
  assert( pkt_temp.size <= audio_size );

  while ( pkt_temp.size > 0 || pkt_temp.data == NULL )
    {
       // Decode the audio into the buffer
       assert( _audio_buf_used + pkt_temp.size <= _audio_max );
       assert( pkt_temp.data != NULL );
       assert( _audio_buf_used % 16 == 0 );
       assert( audio_size > 0 );
       int ret = decode_audio3( ctx, 
				( int16_t * )( (char*)_audio_buf + 
					       _audio_buf_used ), 
				&audio_size, &pkt_temp );

      // If no samples are returned, then break now
      if ( ret <= 0 )
	{
	   pkt_temp.size = 0;
	   IMG_ERROR( _("Audio missed for frame: ") << ptsframe
		      << _(" ret: ") << ret
		      << _(" audio max: ")  << _audio_max 
		      << _(" audio used: ") << _audio_buf_used 
		       );
	   
	  return kDecodeMissingSamples;
	}


      assert( ret <= pkt_temp.size );
      assert( ret > 0 );
      assert( audio_size + _audio_buf_used <= _audio_max );

      // Decrement the length by the number of bytes parsed
      pkt_temp.data += ret;
      pkt_temp.size -= ret;

      if ( audio_size <= 0 ) break;

      _audio_buf_used += audio_size;
    }

  if ( pkt_temp.size == 0 ) return kDecodeOK;

  IMG_ERROR( _("decode_audio - missed decoding some samples") );

  return kDecodeMissingSamples;
}


/** 
 * Decode an audio packet and split it into frames
 * 
 * @param audio_frame   audio frame decoded
 * @param frame         frame we expect 
 * @param pkt           audio packet
 * 
 * @return status whether frame was decoded correctly or not.
 */
CMedia::DecodeStatus 
CMedia::decode_audio( boost::int64_t& audio_frame,
		      const boost::int64_t frame, const AVPacket& pkt )
{

  audio_frame = frame;

  CMedia::DecodeStatus got_audio = decode_audio_packet( audio_frame, 
							frame, pkt );
  if ( got_audio != kDecodeOK ) {
     return got_audio;
  }

  got_audio = kDecodeMissingFrame;


  unsigned int index = 0;
  unsigned int idx = 0;

  SCOPED_LOCK( _audio_mutex );

  boost::int64_t last = audio_frame;

  unsigned int bytes_per_frame = audio_bytes_per_frame();
  assert( bytes_per_frame != 0 );

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

      idx = index;
      index += store_audio( last, 
			    (boost::uint8_t*)_audio_buf + index, 
			    bytes_per_frame );


      if ( last >= frame )  got_audio = kDecodeOK;




      assert( bytes_per_frame <= _audio_buf_used );
      _audio_buf_used -= bytes_per_frame;




      ++last;
    }
  
#ifdef DEBUG
  if ( got_audio != kDecodeOK )
    {
      IMG_WARNING( _("Did not fill audio frame ") << audio_frame 
		   << _(" last ") << last
		   << _(" from ") << frame << _(" used: ") << _audio_buf_used
		   << _(" need ") 
		   << audio_bytes_per_frame() );
    }
#endif


  if (_audio_buf_used > 0  )
    {

       //
       // NOTE: audio buffer must remain 16 bits aligned for ffmpeg.

       memmove( _audio_buf, _audio_buf + index, _audio_buf_used );
     
       // _audio_buf_used = _audio_max - index;
       // assert( _audio_buf_used % 16 == 0 );
    }

  return got_audio;
}


// Return the number of frames cached for jog/shuttle
unsigned int CMedia::max_audio_frames()
{
   if ( _audio_cache_size > 0 )
      return _audio_cache_size;
   else
      return unsigned( fps() );
}




void CMedia::audio_stream( int idx )
{
  if ( idx != -1 && unsigned(idx) >= number_of_audio_streams() )
    throw "Audio stream invalid";

  if ( idx == _audio_index ) return;

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
    }

  clear_stores();

  _audio_index = idx;

  if ( _audio_index >= 0 )
    {
      open_audio_codec();
      seek( _frame );
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
CMedia::store_audio( const boost::int64_t audio_frame, 
		     const boost::uint8_t* buf, const unsigned int size )
{
#ifdef DEBUG_DECODE
   std::cerr << "store audio " << audio_frame << std::endl;
#endif

  assert( buf != NULL );

  SCOPED_LOCK( _audio_mutex );

  // Ge2t the audio codec context
  AVStream* stream = get_audio_stream();
  if ( !stream ) return 0;

  //  assert( audio_frame <= last_frame()  );
  //  assert( audio_frame >= first_frame() );


  _audio_last_frame = audio_frame;

  AVCodecContext* ctx = stream->codec;

  // Get the audio info from the codec context
  if ( ctx->channels == 1 ) _audio_channels = 1;
  int channels = _audio_channels;
  if ( channels == 0 ) channels = ctx->channels;

  int frequency = ctx->sample_rate;

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

  if ( _audio.empty() || _audio.back()->frame() < audio_frame )
    {
      _audio.push_back( aud );
    }
  else
    {
      audio_cache_t::iterator at = std::lower_bound( _audio.begin(), 
						     _audio.end(),
						     audio_frame, 
						     LessThanFunctor() );

      // Avoid storing duplicate frames, replace old frame with this one
      if ( at != _audio.end() )
	{
	  if ( (*at)->frame() == audio_frame )
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
 * @param frame   frame to find audio for
 */
void CMedia::fetch_audio( const boost::int64_t frame )
{
  // Seek to the correct position if necessary
  if ( frame != _expected )
    {
      seek_to_position( frame );
      return;
    }

  bool got_audio = !has_audio();

  if ( !got_audio && !_audio.empty() )
    {
       got_audio = in_audio_store( frame );
    }

  if ( got_audio ) return;

 

  // Clear the packet
  AVPacket pkt;
  av_init_packet( &pkt );

  unsigned int bytes_per_frame = audio_bytes_per_frame();
  unsigned int audio_bytes = 0;

  unsigned counter = 0;
  int eof = false;
  boost::int64_t dts = _dts;

  // Loop until an error or we have what we need
  while( !got_audio )
    {
       if (eof) {
	  
	  AVStream* stream = get_audio_stream();
	  if (!got_audio && audio_stream_index() >= 0 &&
	      stream->codec->codec->capabilities & CODEC_CAP_DELAY) {
	     av_init_packet(&pkt);
	     pkt.data = NULL;
	     pkt.size = 0;
	     pkt.stream_index = audio_stream_index();
	     _audio_packets.push_back( pkt );
	  }

	  got_audio = true;
	  eof = false;
	  continue;
       }
 
      int error = av_read_frame( _acontext, &pkt );
      if ( error < 0 )
      {
	if ( error == AVERROR_EOF )
	{
	   av_free_packet( &pkt );
	   counter++;
	   if ( counter >= _frame_offset )
	      break;
	   IMG_ERROR( _("Could not read audio for frame ") << frame);
	   eof = true;
	   continue;
	}
      }

      if ( has_audio() && pkt.stream_index == audio_stream_index() )
  	{
  	  assert( pkt.pts != MRV_NOPTS_VALUE );

  	  boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );

	  if ( playback() == kBackwards )
	  {
	     // Only add packet if it comes before seek frame
	     if ( pktframe <= frame )  
		_audio_packets.push_back( pkt );
	     if ( pktframe < dts ) dts = pktframe;
	  }
	  else
	  {
	     if ( pktframe <= last_frame() ) 
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
	  }

	  continue;
  	}

      av_free_packet( &pkt );
    }
}



void CMedia::audio_initialize()
{
  if ( _audio_engine ) return;
  _audio_engine = mrv::AudioEngine::factory();
  _audio_channels = _audio_engine->channels();
}



int64_t CMedia::wait_audio()
{
  mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
  SCOPED_LOCK( apm );

  for(;;)
    {
      if ( stopped() ) break;

      if ( ! _audio_packets.empty() )
	{
	  const AVPacket& pkt = _audio_packets.front();
	  return get_frame( get_audio_stream(), pkt );
	}


      CONDITION_WAIT( _audio_packets.cond(), apm );
    }
  return _frame;
}



bool CMedia::open_audio( const short channels, 
			 const unsigned nSamplesPerSec )
{
  close_audio();

  _samples_per_sec = nSamplesPerSec;

  AudioEngine::AudioFormat format = kSampleFormat;

  // Avoid conversion to float if unneeded
  AVCodecContext* ctx = get_audio_stream()->codec;
  if ( ctx->sample_fmt == AV_SAMPLE_FMT_S16P ||
       ctx->sample_fmt == AV_SAMPLE_FMT_S16 )
  {
     format = AudioEngine::kS16LSB;
  }


  AVSampleFormat fmt = AudioEngine::ffmpeg_format( format );
  unsigned bps = av_get_bytes_per_sample( fmt ) * 8;

  bool ok = false;

  for ( short fmt = format; fmt > 0; fmt -= 2 )
  {
     for (int ch = channels; ch > 0; ch -= 2 )
     {
	ok = _audio_engine->open( ch, nSamplesPerSec,
				  (AudioEngine::AudioFormat)fmt, bps );
	if ( ok ) break;
     }
     if ( ok ) break;
  }

  _audio_format = _audio_engine->format();

  if ( channels == 1 )
     _audio_channels = 1;
  else
     _audio_channels = _audio_engine->channels();
  

  return ok;
}


bool CMedia::play_audio( const mrv::audio_type_ptr& result )
{
  double speedup = _play_fps / _fps;
  unsigned nSamplesPerSec = unsigned( result->frequency() * speedup );
  if ( nSamplesPerSec != _samples_per_sec ||
       result->channels() != _audio_channels )
    {
      SCOPED_LOCK( _audio_mutex );
      if ( ! open_audio( result->channels(), nSamplesPerSec ) )
	{
	  IMG_ERROR( _("Could not open audio driver") );
	  _audio_index = -1;
	  return false;
	}
    }

  if ( ! _audio_engine ) return false;
  
  if ( ! _audio_engine->play( (char*)result->data(), result->size() ) )
  {
     IMG_ERROR( _("Playback of audio frame failed") );
     close_audio();
     return false;
  }
  
  return true;
}

bool CMedia::find_audio( const boost::int64_t frame )
{
  audio_type_ptr result;

  if ( !has_video() && !is_sequence() ) 
    {
      _frame = frame;
      refresh();
    }
  

  {

#ifdef DEBUG_PACKETS
    debug_audio_packets(frame, "FIND");
#endif

#ifdef DEBUG_STORES
    debug_audio_stores(frame, "FIND");
#endif

    SCOPED_LOCK( _audio_mutex );

    _audio_frame = frame;
    audio_cache_t::iterator end = _audio.end();
    audio_cache_t::iterator i = std::lower_bound( _audio.begin(), end, 
						  frame, LessThanFunctor() );
    
    if ( i == end )
      {
	IMG_WARNING( _("Audio frame ") << frame << _(" not found") );
	limit_audio_store( frame );
	return false;
      }


    result = *i;


    limit_audio_store( frame );
  }
  
  bool ok = play_audio( result );
  _audio_pts   = int64_t( _audio_frame / _fps );
  _audio_clock = av_gettime() / 1000000.0;
  return ok;
}



void CMedia::flush_audio()
{
  if ( has_audio() )
    avcodec_flush_buffers( get_audio_stream()->codec );
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
  if ( !_audio_engine ) return;
  _audio_engine->volume( v );
}


CMedia::DecodeStatus 
CMedia::handle_audio_packet_seek( boost::int64_t& frame, 
				  const bool is_seek )
{
#ifdef DEBUG_PACKETS
  debug_audio_packets(frame, "DOSEEK");
#endif

#ifdef DEBUG_STORES
  debug_audio_stores(frame, "DOSEEK");
#endif

  Mutex& m = _audio_packets.mutex();
  SCOPED_LOCK( m );

  assert( _audio_packets.is_seek() || _audio_packets.is_preroll() );

  _audio_packets.pop_front();  // pop seek/preroll begin packet

  DecodeStatus got_audio = kDecodeMissingFrame;

  assert( _audio_buf_used == 0 );

  {
    AVPacket& pkt = _audio_packets.front();
    _audio_last_frame = get_frame( get_audio_stream(), pkt );
  }

  boost::int64_t last = _audio_last_frame;

 
  while ( !_audio_packets.empty() && !_audio_packets.is_seek() )
    {
      AVPacket& pkt = _audio_packets.front();

      if ( decode_audio( last, frame, pkt ) == kDecodeOK )
	got_audio = kDecodeOK;
      
      _audio_packets.pop_front();
    }

  if ( _audio_packets.empty() ) return kDecodeError;

  if ( is_seek )
    {
      AVPacket& pkt = _audio_packets.front();
      frame = get_frame( get_audio_stream(), pkt );
    }

  _audio_packets.pop_front();  // pop seek/preroll end packet

  if ( _audio_packets.empty() ) return got_audio;

#ifdef DEBUG_PACKETS
  debug_audio_packets(frame, "DOSEEK END");
#endif

#ifdef DEBUG_STORES
  debug_audio_stores(frame, "DOSEEK END");
#endif

  return kDecodeOK;
}

bool CMedia::in_audio_store( const boost::int64_t frame )
{
  SCOPED_LOCK( _audio_mutex );
  // Check if audio is already in audio store
  // If so, no need to decode it again.  We just pop the packet and return.
  audio_cache_t::iterator end = _audio.end();
  audio_cache_t::iterator i = std::find_if( _audio.begin(), end,
					    EqualFunctor(frame) );
  if ( i != end ) return true;
  return false;
}

CMedia::DecodeStatus CMedia::decode_audio( boost::int64_t& frame )
{ 
#ifdef DEBUG_AUDIO_PACKETS
  debug_audio_packets(frame, "DECODE");
#endif

#ifdef DEBUG_AUDIO_STORES
  debug_audio_stores(frame, "DECODE");
#endif

  DecodeStatus got_audio = kDecodeMissingFrame;

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
      if ( _audio_packets.is_flush() )
	{
	  flush_audio();
	  _audio_packets.pop_front();
	}
      else if ( _audio_packets.is_loop_start() )
	{
	  AVPacket& pkt = _audio_packets.front();
	  // with loops, packet dts is really frame
	  if ( frame <= pkt.dts )
	    {
	       flush_audio();
	      _audio_packets.pop_front();
	      return kDecodeLoopStart;
	    }

	  bool ok = in_audio_store( frame );	   
	  if ( ok ) return kDecodeOK;
	  return kDecodeError;
	}
      else if ( _audio_packets.is_loop_end() )
	{
	  AVPacket& pkt = _audio_packets.front();
	  // with loops, packet dts is really frame
	  if ( frame >= pkt.dts )
	    {
	       flush_audio();
	       _audio_packets.pop_front();
	       return kDecodeLoopEnd;
	    }

	  bool ok = in_audio_store( frame );	   
	  if ( ok ) return kDecodeOK;
	  return kDecodeError;
	}
      else if ( _audio_packets.is_seek()  )
	{
	  clear_stores();  // audio stores MUST be cleared when seeked
	  got_audio = handle_audio_packet_seek( frame, true );
	  continue;
	}
      else if ( _audio_packets.is_preroll() )
	{
	  AVPacket& pkt = _audio_packets.front();
	  bool ok = in_audio_store( frame );
	  boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );
	  
	  if ( ok && pktframe != frame )
	  {
	     got_audio = kDecodeOK;
	     continue;
	  }

	  // when prerolling, we always start at the beginning of audio
	  // buffer.
	  _audio_buf_used = 0;
	  got_audio = handle_audio_packet_seek( pktframe, false );
	  return got_audio;
	}
      else
	{
	  AVPacket& pkt = _audio_packets.front();
	  bool ok = in_audio_store( frame );
	  boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );
	  if ( ok ) 
	    {
	      if ( pktframe == frame )
		{
		  int64_t temp;
		  decode_audio_packet( temp, frame, pkt );
		  _audio_packets.pop_front();
		  return kDecodeOK;
		}
	    }

	  boost::int64_t last_frame = pktframe;
	  got_audio = decode_audio( last_frame, frame, pkt );
	  if ( got_audio == kDecodeError ) {
	     return got_audio;
	  }
	  _audio_packets.pop_front();
	}

    }

#ifdef DEBUG_PACKETS
  debug_audio_packets(frame, "DECODE END");
#endif

#ifdef DEBUG_STORES
  debug_audio_stores(frame, "DECODE END");
#endif


  return got_audio;
}



void CMedia::do_seek()
{
  _dts = _seek_frame;

  bool got_audio = !has_audio();

  if ( !got_audio )
    {
       got_audio = in_audio_store( _seek_frame );
    }

  if ( !got_audio )
  {
     clear_packets();
    
     seek_to_position( _seek_frame );
     
  }


  if ( stopped() )
  {
  
     if ( has_audio() )
     {
	decode_audio( _seek_frame );
	find_audio( _seek_frame );
     }
  }

  // Seeking done, turn flag off
  _seek_req = false;

  find_image( _seek_frame );

  // Queue thumbnail for update
  image_damage( image_damage() | kDamageThumbnail );
}



void CMedia::debug_audio_stores(const boost::int64_t frame, 
				   const char* routine)
{
  SCOPED_LOCK( _audio_mutex );

  audio_cache_t::const_iterator iter = _audio.begin();
  audio_cache_t::const_iterator last = _audio.end();

  std::cerr << name() << " S:" << _frame << " D:" << _dts 
	    << " A:" << frame << " " << routine << " audio stores #"
	    << _audio.size() << ": ";

  if ( iter != last )
  {
     std::cerr << (*iter)->frame() << "-" << (*(last-1))->frame();
  }

  std::cerr << std::endl;

#ifdef DEBUG_AUDIO_STORES_DETAIL
  for ( ; iter != last; ++iter )
    {
      boost::int64_t f = (*iter)->frame();
      if ( f == frame )  std::cerr << "P";
      if ( f == _dts )   std::cerr << "D";
      if ( f == _frame ) std::cerr << "F";
      std::cerr << f << " ";
    }
  std::cerr << std::endl;
#endif
}


void CMedia::debug_audio_packets(const boost::int64_t frame,
				    const char* routine)
{
  if ( !has_audio() ) return;

  mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
  SCOPED_LOCK( apm );

  mrv::PacketQueue::const_iterator iter = _audio_packets.begin();
  mrv::PacketQueue::const_iterator last = _audio_packets.end();
  std::cerr << name() << " S:" << _frame << " D:" << _dts 
	    << " A:" << frame << " " << routine << " audio packets #"
	    << _audio_packets.size() << " (" << _audio_packets.bytes() << "): "
	    << std::endl;


  bool in_preroll = false;
  bool in_seek = false;

  boost::int64_t last_frame = std::numeric_limits< boost::int64_t >::min();

  for ( ; iter != last; ++iter )
    {
      if ( _audio_packets.is_flush( *iter ) )
	{
	  std::cerr << "* "; continue;
	}
      else if ( _audio_packets.is_loop_end( *iter ) ||
		_audio_packets.is_loop_start( *iter ) )
	{
	   boost::int64_t f = get_frame( get_audio_stream(), (*iter) );
	   std::cerr << "L(" << f << ")"; continue;
	}

      boost::int64_t f = get_frame( get_audio_stream(), (*iter) );

      if ( _audio_packets.is_seek( *iter ) )
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
	      std::cerr << "<SEEK:" << f << ">";
	      in_seek = true;
	    }
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
	  if ( f == last_frame ) continue;

	  if ( f == frame )  std::cerr << "S";
	  if ( f == _dts )   std::cerr << "D";
	  if ( f == _frame ) std::cerr << "F";
	  std::cerr << f << " ";
	  last_frame = f;
	}
    }

  std::cerr << std::endl;
}




} // namespace mrv
