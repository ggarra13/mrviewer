/**
 * @file   aviImage.cpp
 * @author 
 * @date   Tue Sep 26 17:54:48 2006
 * 
 * @brief  Read and play an avi file with audio.
 *         We rely on using the ffmpeg library.
 * 
 */

#include <cstdio>

#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>

#include <iostream>
#include <algorithm>
#include <limits>
using namespace std;


#if !defined(WIN32) && !defined(WIN64)
#include <arpa/inet.h>
#else
#include <winsock2.h>    // for htonl
#endif



extern "C" {
#include "libavutil/pixdesc.h"
}

#include "aviImage.h"
#include "mrvImageView.h"
#include "mrvPlayback.h"
#include "gui/mrvIO.h"          // @todo: change to some static variable
#include "mrvFrameFunctors.h"
#include "mrvThread.h"
#include "mrvCPU.h"



namespace 
{
  const char* kModule = "avi";
}


#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )
#define IMG_WARNING(x) LOG_WARNING( name() << " - " << x )
#define LOG(x) std::cerr << x << std::endl;

// this plays backwards by seeking, which is slow but memory efficient
//#define USE_SEEK_TO_PLAY_BACKWARDS  

//#define DEBUG_STREAM_INDICES
//#define DEBUG_STREAM_KEYFRAMES
//#define DEBUG_DECODE
//#define DEBUG_SEEK
//#define DEBUG_SEEK_VIDEO_PACKETS
//#define DEBUG_SEEK_AUDIO_PACKETS
//#define DEBUG_SEEK_SUBTITLE_PACKETS
//#define DEBUG_PACKETS
//#define DEBUG_PACKETS_DETAIL
//#define DEBUG_STORES
//#define DEBUG_STORES_DETAIL
//#define DEBUG_SUBTITLE_STORES
//#define DEBUG_SUBTITLE_RECT

// #define SEEK_WITH_BYTES


#define FFMPEG_STREAM_BUG_FIX


//  in ffmpeg, sizes are in bytes...
#define kMAX_QUEUE_SIZE (15 * 1024 * 1024)
#define kMAX_AUDIOQ_SIZE (20 * 16 * 1024)
#define kMAX_SUBTITLEQ_SIZE (5 * 30 * 1024)
#define kMIN_FRAMES 5

namespace {
  const unsigned int  kMaxCacheImages = 2;
}

namespace mrv {



aviImage::aviImage() :
  CMedia(),
  _video_index(-1),
  _av_frame( NULL ),
  _video_codec( NULL ),
  _convert_ctx( NULL ),
  _video_images( 0 ),
  _max_images( kMaxCacheImages ),
  _subtitle_codec( NULL )
{
  _gamma = 1.0f;
  _compression = "";

  memset(&_sub, 0, sizeof(AVSubtitle));

}


aviImage::~aviImage()
{

  if ( !stopped() )
    stop();

  image_damage(0);

  _video_packets.clear();

  flush_video();

  if ( _convert_ctx )
     sws_freeContext( _convert_ctx );
  if ( _av_frame )
     av_free( _av_frame );

  if ( _video_index >= 0 )
    close_video_codec();
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

  if ( magic == 0x000001ba )
    {
      // MPEG movie
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
  else if ( strncmp( (char*)data, "OggS", 4 ) == 0 ) 
    {
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
	    (magic & 0xFFE00000) == 0xFFE00000 )
    {
      // MP3
      if ( (magic & 0xF000) == 0xF000 ||
	   (magic & 0xF000) == 0 ) return false;
      return true;
    }
  else
    {
      // Check for Quicktime
      if ( strncmp( (char*)data+4, "ftyp", 4 ) != 0 && 
	   strncmp( (char*)data+4, "moov", 4 ) != 0 &&
	   strncmp( (char*)data+4, "free", 4 ) != 0 && 
	   strncmp( (char*)data+4, "mdat", 4 ) != 0 &&
	   strncmp( (char*)data+4, "wide", 4 ) != 0 )
	return false;

      return true;
    }

  return false;
}

// Returns the current subtitle stream
AVStream* aviImage::get_subtitle_stream() const
{
  return _subtitle_index >= 0 ? _context->streams[ subtitle_stream_index() ] : NULL;
}


// Returns the current video stream
AVStream* aviImage::get_video_stream() const
{
  return _video_index >= 0 ? _context->streams[ video_stream_index() ] : NULL;
}



bool aviImage::has_video() const
{
  return ( _video_index >= 0 && _video_info[ _video_index ].has_codec );
}


// Opens the video codec associated to the current stream
void aviImage::open_video_codec()
{
  AVStream *stream = get_video_stream();
  if ( stream == NULL ) return;

  AVCodecContext *ctx = stream->codec;
  _video_codec = avcodec_find_decoder( ctx->codec_id );

  static int workaround_bugs = 1;
  static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
  static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
  static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;
  static int error_resilience = 0; //FF_ER_CAREFUL;
  static int error_concealment = FF_EC_GUESS_MVS|FF_EC_DEBLOCK;

  ctx->workaround_bugs = workaround_bugs;
  ctx->skip_frame= skip_frame;
  ctx->skip_idct= skip_idct;
  ctx->skip_loop_filter= skip_loop_filter;
  ctx->error_concealment= error_concealment;

  double aspect_ratio;
  if ( ctx->sample_aspect_ratio.num == 0 )
    aspect_ratio = 0;
  else
    aspect_ratio = av_q2d( ctx->sample_aspect_ratio ) *
    ctx->width / ctx->height;



  double image_ratio = (float) width() / (float)height();
  if ( aspect_ratio <= 0.0 ) aspect_ratio = image_ratio;

  if ( image_ratio == aspect_ratio ) _pixel_ratio = 1.0f;
  else _pixel_ratio = aspect_ratio / image_ratio;

  AVDictionary* info = NULL;
  if ( _video_codec == NULL || 
       avcodec_open2( ctx, _video_codec, &info ) < 0 )
    _video_index = -1;

}

void aviImage::close_video_codec()
{
  AVStream *stream = get_video_stream();
  if ( ( stream != NULL ) && ( stream->codec != NULL ) )
    avcodec_close( stream->codec );
}


// Flush video buffers
void aviImage::flush_video()
{
  if ( has_video() )
  {
     AVStream* stream = get_video_stream();
     if ( stream != NULL && stream->codec != NULL )
	avcodec_flush_buffers( stream->codec );
  }
}


/// VCR play (and cache frames if needed) sequence
void aviImage::play( const Playback dir,  mrv::ViewerUI* const uiMain)
{
  CMedia::play( dir, uiMain );
}

// Seek to the requested frame
bool aviImage::seek_to_position( const boost::int64_t ptsframe, 
				 const int flags )
{
  AVStream* stream = NULL;
  int idx = -1;
  boost::int64_t min_ts = std::numeric_limits< boost::int64_t >::max();
  static const AVRational base = { 1, AV_TIME_BASE };

  if ( _context == NULL ) return false;


  // boost::int64_t offset = boost::int64_t(((frame - _frameStart) * AV_TIME_BASE) / fps());

  boost::int64_t offset = boost::int64_t((ptsframe * AV_TIME_BASE) / fps());

  boost::int64_t frame = ptsframe;

#if 0
  //
  // Find minimum byte position between audio and video stream
  //
  if ( has_audio() )
    {
      idx = audio_stream_index();
      stream = get_audio_stream();
      if ( stream == NULL ) return false;
      assert( stream != 0 );

      boost::int64_t ts = av_rescale_q(offset, base, stream->time_base);

      int i = av_index_search_timestamp( stream, ts, AVSEEK_FLAG_BACKWARD );
      if ( i >= 0 ) min_ts = stream->index_entries[i].timestamp;

    }

  if ( has_video() ) {
    AVStream* st = get_video_stream();
    assert( st != 0 );
    if ( st == NULL ) return false;

    boost::int64_t ts  = av_rescale_q(offset, base, st->time_base);

    int i = av_index_search_timestamp( st, ts, AVSEEK_FLAG_BACKWARD );

    if ( !stream || (i >= 0 && st->index_entries[i].timestamp < min_ts) )
      {
	idx = video_stream_index();
	stream = st;
      }
  }
 
  assert( stream != NULL );
  assert( idx != -1 );

  if ( stream == NULL ) return false;

  offset = av_rescale_q(offset, base, stream->time_base);


  bool ok;
  try {

     ok = avformat_seek_file( _context, idx, INT64_MIN, offset, INT64_MAX, 
      			      AVSEEK_FLAG_BACKWARD ) == 0;
     //ok = av_seek_frame( _context, idx, offset, AVSEEK_FLAG_BACKWARD ) == 0;

    if (!ok)
      {
	IMG_ERROR( "Could not seek to frame " << frame );
	return false;
      }
#else
  bool ok;
  try {

     ok = avformat_seek_file( _context, -1, INT64_MIN, offset, INT64_MAX, 
			      AVSEEK_FLAG_BACKWARD ) == 0;
    if (!ok)
      {
	IMG_ERROR( "Could not seek to frame " << frame );
	return false;
      }
#endif

  }
  catch( ... )
    {
      IMG_ERROR("av_seek_frame raised some exception" );
      return false;
    }


  bool got_audio = !has_audio();
  bool got_video = !has_video();
  bool got_subtitle = !has_subtitle();

  if ( !got_video )
    {
      got_video = in_video_store( frame );
    }

  if ( !got_audio )
    {
      got_audio = in_audio_store( frame );
    }

  if ( !got_subtitle )
    {
      got_subtitle = in_subtitle_store( frame );
    }


  boost::int64_t vpts = 0, apts = 0, spts = 0;

  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  mrv::PacketQueue::Mutex& apm = _audio_packets.mutex();
  SCOPED_LOCK( apm );

  mrv::PacketQueue::Mutex& spm = _subtitle_packets.mutex();
  SCOPED_LOCK( spm );


  if ( !got_video ) {
    vpts = frame2pts( get_video_stream(), frame );
  }

  if ( !got_audio ) {
    apts = frame2pts( get_audio_stream(), frame );
  }

  if ( !got_subtitle ) {
    spts = frame2pts( get_subtitle_stream(), frame );
  }


  if ( _seek_req || _playback != kBackwards )
    {
      if ( !got_video )    _video_packets.seek_begin(vpts);
      if ( !got_audio )    _audio_packets.seek_begin(apts);
      if ( !got_subtitle ) _subtitle_packets.seek_begin(spts);
    }
  else
    {

#ifdef USE_SEEK_TO_PLAY_BACKWARDS
      if ( !got_video )    _video_packets.seek_begin(vpts);
      if ( !got_audio )    _audio_packets.seek_begin(apts);
      if ( !got_subtitle ) _subtitle_packets.seek_begin(spts);
#else
      if ( !got_video )    _video_packets.preroll(vpts);
      if ( !got_audio )    _audio_packets.preroll(apts);
      if ( !got_subtitle ) _subtitle_packets.preroll(spts);
#endif
    }


#ifdef DEBUG_SEEK
  LOG_INFO( "BEFORE SEEK:" );
  debug_video_packets(frame);
#endif

#ifdef DEBUG_SEEK_AUDIO_PACKETS
  debug_audio_packets(frame);
#endif

  boost::int64_t dts = frame;

  // Clear the packet
  AVPacket pkt;
  av_init_packet( &pkt );

  unsigned int bytes_per_frame = audio_bytes_per_frame();
  unsigned int audio_bytes = 0;

  try {

    while (!got_video || !got_audio) {


      int error = av_read_frame( _context, &pkt );
      if ( error < 0 )
	{
	   int err = _context->pb ? _context->pb->error : 0;
	   if ( err != 0 )
	   {
	      IMG_ERROR("seek: Could not read frame " << frame << " error: "
			<< strerror(err) );
	   }
	   else
	   {
	      IMG_ERROR("seek: Could not read frame " << frame  );
	   }

	   if ( !got_video    ) _video_packets.seek_end(vpts);
	   if ( !got_audio    ) _audio_packets.seek_end(apts);
	   if ( !got_subtitle ) _subtitle_packets.seek_end(spts);

	   ok = false;
	   break;
	}


      if ( has_video() && pkt.stream_index == video_stream_index() )
	{
	  // For video, DTS is really PTS in FFMPEG (yuck)
	   boost::int64_t pktframe = pts2frame( get_video_stream(), pkt.dts );


	  if ( playback() == kBackwards )
	    {
	      if ( pktframe <= frame )
		_video_packets.push_back( pkt );
#ifndef USE_SEEK_TO_PLAY_BACKWARDS
	      if ( pktframe < dts ) dts = pktframe;
#endif
	    }
	  else
	    {
	      _video_packets.push_back( pkt );
	      if ( pktframe > dts ) dts = pktframe;
	    }

	  if ( !got_video && pktframe >= frame ) 
	    {
	      _video_packets.seek_end(vpts);
	      got_video = true;
	    }

#ifdef DEBUG_SEEK_VIDEO_PACKETS
	  char ftype;
	  if (_av_frame->pict_type == FF_B_TYPE)
	    ftype = 'B';
	  else if (_av_frame->pict_type == FF_I_TYPE)
	    ftype = 'I';
	  else
	    ftype = 'P';

	  fprintf( stderr, "f: %05" PRId64 " video dts: %07" PRId64 
		   " as frame: %05" PRId64 " %c\n",
		   _dts, pkt.dts, pktframe, ftype );
#endif
	  continue;
	}
      else
	if ( has_audio() && pkt.stream_index == audio_stream_index() )
	  {
	     boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );

	     if ( playback() == kBackwards )
	     {
		if ( pktframe <= frame ) _audio_packets.push_back( pkt );
		if ( !has_video() && pktframe < dts ) dts = pktframe;
	     }
	     else
	     {
		if ( pktframe <= last_frame() ) _audio_packets.push_back( pkt );
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
		if ( got_audio ) _audio_packets.seek_end(apts);
	     }
	     

#ifdef DEBUG_SEEK_AUDIO_PACKETS
	    fprintf( stderr, "f: %05" PRId64 " audio pts: %07" PRId64 
		     " dts: %07" PRId64 " as frame: %05" PRId64 " size: %d\n",
		     frame, pkt.pts, pkt.dts, pktframe, pkt.size );
#endif
	    continue;
	  }
      else
	if ( has_subtitle() && pkt.stream_index == subtitle_stream_index() )
	  {
	     boost::int64_t pktframe = get_frame( get_subtitle_stream(), pkt );

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
		_subtitle_packets.seek_end(spts);
	      }

#ifdef DEBUG_SEEK_SUBTITLE_PACKETS
	    fprintf( stderr, 
		     "f: %05ld audio pts: %07ld dts: %07ld as frame: %05ld\n",
		     frame, pkt.pts, pkt.dts, pktframe );
#endif
	    continue;
	  }

      av_free_packet( &pkt );
    }
  }
  catch( ... )
    {
      IMG_ERROR("ffmpeg seek raised an exception");
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
  debug_video_packets(frame);
#endif
#ifdef DEBUG_SEEK_AUDIO_PACKETS
  debug_audio_packets(frame);
#endif


  return ok;
}


mrv::image_type_ptr aviImage::allocate_image( const boost::int64_t& frame,
					      const double pts )
{ 
  return mrv::image_type_ptr( new image_type( frame,
					      width(), 
					      height(), 
					      _num_channels,
					      _pix_fmt,
					      VideoFrame::kByte,
					      _av_frame->repeat_pict,
					      pts ) );

}


void aviImage::store_image( const boost::int64_t frame, 
			    const boost::int64_t pts )
{
  AVStream* stream = get_video_stream();
  unsigned int w = width();
  unsigned int h = height();



  mrv::image_type_ptr image = allocate_image( frame, pts * 
					      av_q2d( 
						     stream->time_base
						      ) 
					      );
  if ( ! image )
    {
      IMG_ERROR( "No memory for video frame" );
      IMG_ERROR( "Audios #" << _audio.size() );
      IMG_ERROR( "Videos #" << _images.size() );
      return;
    }


  AVPicture output;
  boost::uint8_t* ptr = (boost::uint8_t*)image->data().get();

  // Fill the fields of AVPicture output based on _av_dst_pix_fmt

  avpicture_fill( &output, ptr, _av_dst_pix_fmt, w, h );

  static const int sws_flags = 0;
  _convert_ctx = sws_getCachedContext(_convert_ctx,
				      stream->codec->width, 
				      stream->codec->height,
				      stream->codec->pix_fmt,
				      w, h,
  				      _av_dst_pix_fmt, SWS_BICUBIC, 
				      NULL, NULL, NULL);

  if ( _convert_ctx == NULL )
    {
      IMG_ERROR("Could not get image conversion context");
      return;
    }

  sws_scale(_convert_ctx, _av_frame->data, _av_frame->linesize,
	    0, stream->codec->height, output.data, output.linesize);

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

CMedia::DecodeStatus
aviImage::decode_video_packet( boost::int64_t& ptsframe, 
			       const boost::int64_t frame, 
			       const AVPacket& pkt
			       )
{
  AVStream* stream = get_video_stream();

  ptsframe = pts2frame( stream, pkt.dts );

  int got_pict = 0;
  avcodec_decode_video2( stream->codec, _av_frame, &got_pict, 
			 (AVPacket*)&pkt );
  if ( got_pict == 0 ) return kDecodeError;
  return kDecodeOK;
}


// Decode the image
CMedia::DecodeStatus
aviImage::decode_image( const boost::int64_t frame, const AVPacket& pkt )
{
  boost::int64_t ptsframe;

  DecodeStatus status = decode_video_packet( ptsframe, frame, pkt );
  if ( status != kDecodeOK )
    {
       char ftype = av_get_picture_type_char( _av_frame->pict_type );
       if ( ptsframe >= first_frame() && ptsframe <= last_frame() )
	  IMG_WARNING("Could not decode video frame " << ptsframe 
		      << " type " << ftype << " pts: " 
		      << (pkt.pts == MRV_NOPTS_VALUE ?
			-1 : pkt.pts ) << " dts: " << pkt.dts
		      << " data: " << (void*)pkt.data);
    }
  else
  {
      if ( ptsframe < frame )
	{
	  status = kDecodeMissingFrame;
	}
      else
	{
	   store_image( ptsframe, pkt.dts );
	}
    }

  return status;
}

void aviImage::clear_packets()
{

#ifdef DEBUG_PACKETS
  cerr << "+++++++++++++ CLEAR VIDEO PACKETS" << endl;
#endif
  _video_packets.clear();
  _audio_packets.clear();

  _audio_buf_used = 0;
  _video_images = 0;
}


//
// Limit the video store to approx. max frames images on each side.
// We have to check both where frame is as well as where _dts is.
//
void aviImage::limit_video_store(const boost::int64_t frame)
{

  boost::int64_t first, last;

  switch( playback() )
    {
    case kBackwards:
      first = frame - max_video_frames();
      last  = frame;
      if ( _dts < first ) first = _dts;
      break;
    case kForwards:
      first = frame;
      last  = frame + 1; // max_video_frames();
      if ( _dts > last )   last = _dts;
      break;
    default:
      first = frame - max_video_frames()/2;
      last  = frame + max_video_frames()/2;
      if ( first < first_frame() ) first = first_frame();
      if ( last  > last_frame() )   last = last_frame();
      break;
    }

  //   if ( !_images.empty() )
  //     {
  //       cerr << frame << " store " << _images.front()->frame() << "-" 
  // 	   << _images.back()->frame() << " #" << _images.size() << endl;
  //     }

  //   cerr << frame << " limit " << first << "-" 
  //        << last << endl;

  video_cache_t::iterator end = _images.end();
  _images.erase( std::remove_if( _images.begin(), end,
				 NotInRangeFunctor( first, last ) ), end );



}

//
// Limit the subtitle store to approx. max frames images on each side.
// We have to check both where frame is as well as where _dts is.
//
void aviImage::limit_subtitle_store(const boost::int64_t frame)
{

  boost::int64_t first, last;

  switch( playback() )
    {
    case kBackwards:
       first = frame - (boost::int64_t)fps() * 2;
       last  = frame;
       if ( _dts < first ) first = _dts;
      break;
    case kForwards:
      first = frame;
      last  = frame + (boost::int64_t)fps() * 2;
      if ( _dts > last )   last = _dts;
      break;
    default:
       first = frame - (boost::int64_t)fps() * 2;
       last  = frame + (boost::int64_t)fps() * 2;
      if ( first < first_frame() ) first = first_frame();
      if ( last  > last_frame() )   last = last_frame();
      break;
    }

  subtitle_cache_t::iterator end = _subtitles.end();
  _subtitles.erase( std::remove_if( _subtitles.begin(), end,
				    NotInRangeFunctor( first, last ) ), end );



}

// Opens the subtitle codec associated to the current stream
void aviImage::open_subtitle_codec()
{
  AVStream *stream = get_subtitle_stream();
  AVCodecContext *ctx = stream->codec;
  _subtitle_codec = avcodec_find_decoder( ctx->codec_id );

  static int workaround_bugs = 1;
  static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
  static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
  static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;
  static int error_resilience = 0; //FF_ER_CAREFUL;
  static int error_concealment = FF_EC_GUESS_MVS|FF_EC_DEBLOCK;

  ctx->workaround_bugs = workaround_bugs;
  ctx->skip_frame= skip_frame;
  ctx->skip_idct= skip_idct;
  ctx->skip_loop_filter= skip_loop_filter;
  ctx->error_concealment= error_concealment;

  AVDictionary* info = NULL;
  if ( _subtitle_codec == NULL || 
       avcodec_open2( ctx, _subtitle_codec, &info ) < 0 )
    _subtitle_index = -1;
}

void aviImage::close_subtitle_codec()
{
  AVStream *stream = get_subtitle_stream();
  if ( stream && stream->codec )
    avcodec_close( stream->codec );
}

bool aviImage::find_subtitle( const boost::int64_t frame )
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


  image_damage( image_damage() | kDamageSubtitle );

  limit_subtitle_store( frame );

  return false;
}

bool aviImage::find_image( const boost::int64_t frame )
{

#ifdef DEBUG_PACKETS
  debug_video_packets(frame, "find_image");
#endif

#ifdef DEBUG_STORES
  debug_video_stores(frame, "find_image");
#endif

  {

    SCOPED_LOCK( _mutex );

    video_cache_t::iterator end = _images.end();
    video_cache_t::iterator i = std::lower_bound( _images.begin(), end, 
						  frame, LessThanFunctor() );

    if ( i != end && *i )
      {
	_hires = *i;
      }
    else
      {
	// Hmm... no close image was found.  If we have some images in
	// cache, we choose the last one in it.  This avoids problems if
	// the last frame is the one with problem.
	// If not, we fail.
	_frame = frame;
	
	if ( ! _images.empty() )
	  {
	    _hires = _images.back();
	    IMG_WARNING( "find_image: " << frame 
			 << _(" not found, choosing ") << _hires->frame() 
			 << _(" instead") );
	    refresh();
	  }
	else
	  {
	     IMG_ERROR( "find_image: " << frame << _(" not found") );
	  }
	return false;
      }


    boost::int64_t distance = frame - _hires->frame();
    if ( distance > _hires->repeat() )
      {
	boost::int64_t first = (*_images.begin())->frame();
	video_cache_t::iterator end = std::max_element( _images.begin(), 
							_images.end() );
	boost::int64_t last  = (*end)->frame();
	boost::uint64_t diff = last - first + 1;
	IMG_ERROR( _("Video Sync master frame ") << frame 
		     << " != " << _hires->frame()
		   << _(" video frame, cache ") << first << "-" << last
		   << " (" << diff << _(") cache size: ") << _images.size()
		   << " dts: " << _dts );
	// 	debug_video_stores(frame);
	// 	debug_video_packets(frame);
      }

    _video_pts   = _hires->pts();
    _video_clock = av_gettime() / 1000000.0;

    // Limit (clean) the video store as we play it
    limit_video_store( frame );

  }  // release lock


  _frame = frame;
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

  _video_index  = x;
  _num_channels = 0;
  if ( x < 0 ) return;

  int mask = (1 << PIX_FMT_BGR24) | (1 << PIX_FMT_BGR32);

  if ( supports_yuv() )
    {
      mask |= ( (1 << PIX_FMT_YUV444P) | 
		(1 << PIX_FMT_YUV422P) | (1 << PIX_FMT_YUV420P) );

//       mask |= ( (1 << PIX_FMT_YUVA420P) | (1 << PIX_FMT_YUV444P) | 
// 		(1 << PIX_FMT_YUV422P) | (1 << PIX_FMT_YUV420P) );
    }

  AVStream* stream = get_video_stream();

  int has_alpha = ( ( stream->codec->pix_fmt == PIX_FMT_RGB32   ) |
		    ( stream->codec->pix_fmt == PIX_FMT_RGB32_1 ) |
		    ( stream->codec->pix_fmt == PIX_FMT_PAL8    ) | 
		    ( stream->codec->pix_fmt == PIX_FMT_BGR32   ) | 
		    ( stream->codec->pix_fmt == PIX_FMT_BGR32_1 ) );

  _av_dst_pix_fmt = avcodec_find_best_pix_fmt( mask, stream->codec->pix_fmt,
					       has_alpha, NULL );
  if ( _av_dst_pix_fmt < 0 ) _av_dst_pix_fmt = PIX_FMT_BGRA;

  _num_channels = 0;
  _layers.clear();
		
  rgb_layers();
  lumma_layers();

  if ( _av_dst_pix_fmt == PIX_FMT_RGBA ||
       _av_dst_pix_fmt == PIX_FMT_BGRA ||
       _av_dst_pix_fmt == PIX_FMT_YUVA420P) alpha_layers();


  unsigned int w = stream->codec->width;

  switch( _av_dst_pix_fmt )
    {
    case PIX_FMT_BGR24:
      _pix_fmt = VideoFrame::kBGR; break;
    case PIX_FMT_BGR32:
      _pix_fmt = VideoFrame::kBGRA; break;
    case PIX_FMT_RGB24:
      _pix_fmt = VideoFrame::kRGB; break;
    case PIX_FMT_RGB32:
      _pix_fmt = VideoFrame::kRGBA; break;
    case PIX_FMT_YUV444P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_702_YCbCr444; 
      else
	_pix_fmt = VideoFrame::kITU_601_YCbCr444; 
      break;
    case PIX_FMT_YUV422P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_702_YCbCr422;
      else
	_pix_fmt = VideoFrame::kITU_601_YCbCr422;
      break;
    case PIX_FMT_YUV420P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_702_YCbCr420;
      else
	_pix_fmt = VideoFrame::kITU_601_YCbCr420;
      break;
    case PIX_FMT_YUVA420P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_702_YCbCr420A;
      else
	_pix_fmt = VideoFrame::kITU_601_YCbCr420A;
      break;
    default:
       IMG_ERROR( _("Unknown destination video frame format: ") 
		  << av_get_pix_fmt_name( _av_dst_pix_fmt ) );

      _pix_fmt = VideoFrame::kBGRA; break;
    }

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
      const AVCodecContext* ctx = stream->codec;
      if ( ctx == NULL ) continue;

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
		 populate_stream_info( s, msg, ctx, i );
		 s.has_b_frames = (bool)ctx->has_b_frames;
		 s.fps          = calculate_fps( stream );
		 if ( av_get_pix_fmt_name( ctx->pix_fmt ) )
 		    s.pixel_format = av_get_pix_fmt_name( ctx->pix_fmt );
		 _video_info.push_back( s );
		 if ( _video_index < 0 && s.has_codec )
		 {
		    video_stream( 0 );
		    int w = ctx->width;
		    int h = ctx->height;
		    image_size( w, h );
		 }
		 break;
	      }
	   case AVMEDIA_TYPE_AUDIO:
	      {
		 audio_info_t s;
		 populate_stream_info( s, msg, ctx, i );
		 s.channels   = ctx->channels;
		 s.frequency  = ctx->sample_rate;
		 s.bitrate    = calculate_bitrate( ctx );
		 
		 _audio_info.push_back( s );
		 if ( _audio_index < 0 && s.has_codec )
		    _audio_index = 0;
		 break;
	      }
	   case AVMEDIA_TYPE_SUBTITLE:
	      {
		 subtitle_info_t s;
		 populate_stream_info( s, msg, ctx, i );
		 s.bitrate    = calculate_bitrate( ctx );
		 _subtitle_info.push_back( s );
		 if ( _subtitle_index < 0 )
		    _subtitle_index = 0;
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

  if ( _video_index < 0 && _audio_index < 0 )
    {
       mrvALERT( filename() << _("\n\nNo audio or video stream in file") );
       return;
    }

  // Open the video and audio codecs
  if ( has_video() )    open_video_codec();
  if ( has_audio() )    open_audio_codec();
  if ( has_subtitle() ) open_subtitle_codec();


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


  _fps = _play_fps = calculate_fps( stream );


#ifdef DEBUG_STREAM_INDICES
  debug_stream_index( stream );
#elif defined(DEBUG_STREAM_KEYFRAMES)
  debug_stream_keyframes( stream );
#endif


  //
  // Calculate frame start and frame end if possible
  //

  _frameStart = 1;

  if ( _context->start_time != MRV_NOPTS_VALUE )
    {
      _frameStart = boost::int64_t( ( _fps * ( double )_context->start_time / 
				      ( double )AV_TIME_BASE ) ) + 1;
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

      _frameStart = (boost::int64_t)start; 
    }

  _frame_start = _frameStart;

  //
  // BUG FIX for ffmpeg bugs with some codecs/containers.
  //
  // using context->start_time and context->duration should be enough,
  // but with that ffmpeg often reports a frame that cannot be decoded
  // with some codecs like divx.
  //
  int64_t duration;
  if ( _context->duration != MRV_NOPTS_VALUE )
    {
      duration = int64_t( (int64_t(_fps + 0.5f) * ( double )(_context->duration) / 
			   ( double )AV_TIME_BASE ) );
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

      duration = boost::int64_t( length * int64_t(_fps + 0.5f) );
    }

  _frameEnd = _frameStart + duration - 1;
  _frame_end = _frameEnd;

  bool got_audio = !has_audio();
  unsigned bytes_per_frame = audio_bytes_per_frame();

  if ( has_video() || has_audio() )
    {
      // Loop until we get first frame
      AVPacket pkt;
      // Clear the packet
      av_init_packet( &pkt );

      bool got_image = false;
      while( !got_image )
	{

	  int error = av_read_frame( _context, &pkt );
	  if ( error < 0 )
	  {
	     int err = _context->pb ? _context->pb->error : 0;
	     if ( err != 0 )
	     {
		IMG_ERROR("populate: Could not read frame 1 error: "
			  << strerror(err) );
	     }
	     break;
	  }
	  
	  if ( has_video() && pkt.stream_index == video_stream_index() )
	    {
	      boost::int64_t ptsframe;
	      DecodeStatus status = decode_video_packet( ptsframe, 
							 _frameStart, pkt );
	      if ( status == kDecodeOK )
		{
		   store_image( ptsframe, pkt.dts );
		  _frameStart = _frame_start = ptsframe;
		  got_image = true;
		}
	    }
	  else
	     if ( has_audio() && pkt.stream_index == audio_stream_index() )
	     {
		boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );

		boost::int64_t dts = _frameStart;
		if ( playback() == kBackwards )
		{
		   // Only add packet if it comes before seek frame
		   if ( pktframe <= _frameStart )  
		      _audio_packets.push_back( pkt );
		   if ( !has_video() && pktframe < dts ) dts = pktframe;
		}
		else
		{
		   if ( pktframe <= last_frame() ) 
		      _audio_packets.push_back( pkt );
		   if ( !has_video() && pktframe > dts ) dts = pktframe;
		}
		
	
		if ( !got_audio )
		{
		   unsigned audio_bytes = 0;
		   if ( pktframe > _frameStart ) got_audio = true;
		   else if ( pktframe == _frameStart )
		   {
		      audio_bytes += pkt.size;
		      if ( audio_bytes >= bytes_per_frame ) got_audio = true;
		   }
		}
		
		
#ifdef DEBUG_DECODE
		fprintf( stderr, "POP. A f: %05" PRId64 " audio pts: %07" PRId64 
			 " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
			 pktframe, pkt.pts, pkt.dts, pktframe );
#endif
		continue;
	     }
	  av_free_packet( &pkt );
	}

      if ( got_image ) find_image( _frameStart );
    }
  

  _frame = _dts = _frameStart;
  _expected = _frame-1;



  //
  // Format
  //
  if ( _context->iformat )
    _format = _context->iformat->name;
  else
    _format = "Unknown";

  //
  // Miscellaneous information
  //

  AVDictionary* m = _context->metadata;
  if ( has_audio() )
  {
     AVStream* stream = get_audio_stream();
     if ( stream->metadata ) m = stream->metadata;
  }
  else if ( has_video() )
  {
     AVStream* stream = get_video_stream();
     if ( stream->metadata ) m = stream->metadata;
  }
  
  
  dump_metadata( _context->metadata );

  
  for (unsigned i = 0; i < _context->nb_chapters; ++i) 
  {
     AVChapter *ch = _context->chapters[i];
     dump_metadata(ch->metadata);
  }
       
  if ( _context->nb_programs )
  {
     for (unsigned i = 0; i < _context->nb_programs; ++i) 
     {
	AVDictionaryEntry* tag = 
	     av_dict_get(_context->programs[i]->metadata,
			 "name", NULL, 0);
	char buf[256];
	sprintf( buf, "Program %d", i );
	if ( tag ) 
	   _iptc.insert( std::make_pair(buf, tag->value) );
	dump_metadata( _context->programs[i]->metadata );
     }
  }
  

}

bool aviImage::initialize()
{
  if ( _context == NULL )
    {
      AVDictionary *opts = NULL;
      av_dict_set(&opts, "initial_pause", "1", 0);

      AVInputFormat*     format = NULL;
      int error = avformat_open_input( &_context, filename(), 
				       format, &opts );

      if ( error >= 0 )
	{
	   error = avformat_find_stream_info( _context, NULL );
	}

      if ( error >= 0 )
	{

	  // Allocate an av frame
	  _av_frame = avcodec_alloc_frame();

	  populate();

	}
      else
	{
	  _context = NULL;
	  mrvALERT( filename() << _("\n\nCould not open avi file") );
	  return false;
	}
    }

  return true;
}

void aviImage::preroll( const boost::int64_t frame )
{
  _dts = _frame = _audio_frame = frame;

  _images.reserve( _max_images );

}


bool aviImage::fetch(const boost::int64_t frame)
{
#ifdef DEBUG_DECODE
  cerr << "FETCH BEGIN: " << frame << " EXPECTED: " << _expected << endl;
#endif

  bool got_image = !has_video();
  bool got_audio = !has_audio();

  if ( !got_audio )
    {
      got_audio = in_audio_store( frame );
    }


  if ( frame != _expected )
    {
       if ( ! seek_to_position( frame, AVSEEK_FLAG_BACKWARD ) )
	  LOG_ERROR("Could not seek to frame " << frame );

      SCOPED_LOCK( _audio_mutex ); // needed
      _audio_buf_used = 0;
      return true;
    }


#ifdef DEBUG_DECODE
  cerr << "------------------------------------------------------" << endl;
  cerr << "FETCH START: " << frame << " V:" << got_image << " A:" << got_audio << endl;
#endif

  boost::int64_t dts = frame;

  try {

    AVPacket pkt;

    // Clear the packet
    av_init_packet( &pkt );

    unsigned int bytes_per_frame = audio_bytes_per_frame();
    unsigned int audio_bytes = 0;

    // Loop until an error or we have what we need
    while( !got_image || !got_audio )
      {

	int error = av_read_frame( _context, &pkt );
	if ( error < 0 )
	  {
	     int err = _context->pb ? _context->pb->error : 0;
	     if ( err != 0 )
	     {
		IMG_ERROR("fetch: Could not read frame " << frame << " error: "
			  << strerror(err) );
	     }
	    break;
	  }


	if ( has_video() && 
	     pkt.stream_index == video_stream_index() )
	  {
	     boost::int64_t pktframe = pts2frame( get_video_stream(), pkt.dts );

	    if ( playback() == kBackwards )
	      {
		// Only add packet if it comes before seek frame
		if ( pktframe <= frame )
		  _video_packets.push_back( pkt );
		if ( pktframe < dts ) dts = pktframe;
	      }
	    else
	      {
		_video_packets.push_back( pkt );
		if ( pktframe > dts ) dts = pktframe;
	      }

	    if ( pktframe >= frame )
	      got_image = true;

#ifdef DEBUG_DECODE
	    char ftype;
	    if (_av_frame->pict_type == FF_B_TYPE)
	      ftype = 'B';
	    else if (_av_frame->pict_type == FF_I_TYPE)
	      ftype = 'I';
	    else
	      ftype = 'P';
	    fprintf( stderr, "FETCH V f: %05" PRId64 " video pts: %07" PRId64 
		     " dts: %07" PRId64 " %c as frame: %05" PRId64 "\n",
		     frame, pkt.pts, pkt.dts, ftype, pktframe );
#endif
	    continue;
	  }
	else
	  if ( has_audio() && pkt.stream_index == audio_stream_index() )
	    {
	       boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );

	      if ( playback() == kBackwards )
		{
		  // Only add packet if it comes before seek frame
		  if ( pktframe <= frame )  _audio_packets.push_back( pkt );
		  if ( !has_video() && pktframe < dts ) dts = pktframe;
		}
	      else
		{
		  if ( pktframe <= last_frame() ) 
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
		}


#ifdef DEBUG_DECODE
	      fprintf( stderr, "FETCH A f: %05" PRId64 " audio pts: %07" PRId64 
		       " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
		       frame, pkt.pts, pkt.dts, pktframe );
#endif
	      continue;
	    }
	  else
	    if ( has_subtitle() && pkt.stream_index == subtitle_stream_index() )
	      {
		 boost::int64_t pktframe = get_frame( get_subtitle_stream(), pkt );
		 if ( playback() == kBackwards )
		 {
		    if ( pktframe <= frame )
		       _subtitle_packets.push_back( pkt );
		 }
		 else
		 {
		    _subtitle_packets.push_back( pkt );
		 }
		continue;
	      }
	
	av_free_packet( &pkt );
      }
    
  }
  catch( ... )
    {
      IMG_ERROR("ffmpeg raised an exception");
    }



  _dts = dts;
  _expected = dts + 1;

#ifdef DEBUG_DECODE
  LOG_INFO( "------------------------------------------------------" );
  LOG_INFO( "FETCH DONE: " << _dts << "   expected: " << _expected );

  debug_video_packets(frame, "FETCH");
  debug_video_stores(frame, "FETCH");

  LOG_INFO( "------------------------------------------------------" );
#endif

  return true;
}


bool aviImage::frame( const boost::int64_t f )
{
  //  if ( f == _dts ) return false;

   if ( ( _video_packets.bytes() +  _audio_packets.bytes() + 
   	  _subtitle_packets.bytes() > kMAX_QUEUE_SIZE ) // ||
   	// (_audio_packets.bytes() > kMAX_AUDIOQ_SIZE)  ||
   	// (_video_packets.size() > kMIN_FRAMES) ||
   	// (_subtitle_packets.size() > kMIN_FRAMES) 
	)
    {
     return false;
    }

  if ( f < _frameStart )    _dts = _frameStart;
  else if ( f > _frameEnd ) _dts = _frameEnd;
  else                      _dts = f;

  fetch(_dts);


#ifdef DEBUG_DECODE
  LOG_INFO( "------- FRAME DONE _dts: " << _dts << " _frame: " 
	    << _frame << " _expected: "  << _expected );
#endif

  return true;
}

CMedia::DecodeStatus 
aviImage::handle_video_packet_seek( boost::int64_t& frame, const bool is_seek )
{
#ifdef DEBUG_PACKETS
  debug_video_packets(frame, "BEFORE PREROLL");
#endif

#ifdef DEBUG_STORES
  debug_video_stores(frame, "BEFORE PREROLL");
#endif

  Mutex& mutex = _video_packets.mutex();
  SCOPED_LOCK( mutex );

  _video_packets.pop_front();  // pop seek begin packet

  DecodeStatus got_image = kDecodeMissingFrame;

  while ( !_video_packets.is_seek() && !_video_packets.empty() )
    {
      const AVPacket& pkt = _video_packets.front();

      boost::int64_t pktframe = pts2frame( get_video_stream(), pkt.dts );


      if ( !is_seek && _playback == kBackwards && 
	   pktframe >= frame - max_video_frames() )
	{
	  DecodeStatus status = decode_image( frame, pkt );
	  if      ( status == kDecodeOK ) got_image = status;
	  else if ( status == kDecodeMissingFrame )
	     store_image( pktframe, pkt.dts );
	}
      else
	{
	  if ( pktframe >= frame )
	    {
	      got_image = decode_image( frame, pkt );
	    }
	  else
	    {
	      decode_video_packet( pktframe, frame, pkt );
	    }
	}

      _video_packets.pop_front();
    }

  if ( _video_packets.empty() ) return kDecodeError;

  {
    const AVPacket& pkt = _video_packets.front();
    frame = pts2frame( get_video_stream(), pkt.pts );
  }

  _video_packets.pop_front();  // pop seek end packet

      
#ifdef DEBUG_PACKETS
  debug_video_packets(frame, "AFTER PREROLL");
#endif

#ifdef DEBUG_STORES
  debug_video_stores(frame, "AFTER PREROLL");
#endif
  return got_image;
}



int64_t aviImage::wait_image()
{
  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  for(;;)
    {
      if ( stopped() ) break;

      if ( ! _video_packets.empty() )
	{
	  const AVPacket& pkt = _video_packets.front();
	  
	  boost::int64_t pktframe = pts2frame( get_video_stream(), pkt.dts );
	  return pktframe;
	}

      CONDITION_WAIT( _video_packets.cond(), vpm );
    }
  return _frame;
}

bool aviImage::in_video_store( const boost::int64_t frame )
{
  SCOPED_LOCK( _mutex );

  // Check if audio is already in audio store
  // If so, no need to decode it again.  We just pop the packet and return.
  video_cache_t::iterator end = _images.end();
  video_cache_t::iterator i = std::find_if( _images.begin(), end,
					    EqualFunctor(frame) );
  if ( i != end ) return true;
  return false;
}

CMedia::DecodeStatus aviImage::decode_video( boost::int64_t& frame )
{

  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  if ( _video_packets.empty() )
    {
      bool ok = in_video_store( frame );
      if ( ok ) return kDecodeOK;
      return kDecodeMissingFrame;
    }

  DecodeStatus got_image = kDecodeMissingFrame;

  while ( got_image != kDecodeOK && !_video_packets.empty() )
    {
      if ( _video_packets.is_flush() )
	{
	  flush_video();
	  _video_packets.pop_front();
	}
      else if ( _video_packets.is_seek() )
	{
	  return handle_video_packet_seek( frame, true );
	}
      else if ( _video_packets.is_preroll() )
	{
	  AVPacket& pkt = _video_packets.front();
	  bool ok = in_video_store( frame );
	  if ( ok && pts2frame( get_video_stream(), pkt.pts ) != frame )
	    return kDecodeOK;

	  return handle_video_packet_seek( frame, false );
	}
      else if ( _video_packets.is_loop_start() )
	{
	  AVPacket& pkt = _video_packets.front();
	  // with loops, packet dts is really frame
	  if ( frame <= pkt.pts )
	    {
	      flush_video();
	      _video_packets.pop_front();
	      return kDecodeLoopStart;
	    }

	  bool ok = in_video_store( frame );	   
	  if ( ok ) return kDecodeOK;
	  return kDecodeError;
	}
      else if ( _video_packets.is_loop_end() )
	{
	  AVPacket& pkt = _video_packets.front();
	  // with loops, packet dts is really frame
	  if ( frame >= pkt.pts )
	    {
	      flush_video();
	      _video_packets.pop_front();
	      return kDecodeLoopEnd;
	    }

	  bool ok = in_video_store( frame );	   
	  if ( ok ) return kDecodeOK;
	  return kDecodeError;
	}
      else
	{
	  AVPacket& pkt = _video_packets.front();

	  bool ok = in_video_store( frame );
	  if ( ok )
	    {
	       boost::int64_t pktframe = pts2frame( get_video_stream(), 
						    pkt.dts );
	      if ( pktframe == frame )
		{
		  boost::int64_t ptsframe;
		  decode_video_packet( ptsframe, frame, pkt );
		  _video_packets.pop_front();
		}
	      return kDecodeOK;
	    }

	  // Limit storage of frames to only fps.  For example, 30 frames
	  // for a fps of 30.
	  if ( _images.size() >= fps() )
	     return kDecodeOK;

	  got_image = decode_image( frame, pkt );

	  _video_packets.pop_front();
	}

    }


#ifdef DEBUG_STORES
  debug_video_stores(frame, "decode_video");
#endif

  return got_image;
}




void aviImage::debug_subtitle_stores(const boost::int64_t frame, 
				     const char* routine)
{

  SCOPED_LOCK( _subtitle_mutex );

  subtitle_cache_t::const_iterator iter = _subtitles.begin();
  subtitle_cache_t::const_iterator last = _subtitles.end();
  
  std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame 
       << " " << routine << " subtitle stores  #" 
       << _subtitles.size() << ": "
       << std::endl;
  for ( ; iter != last; ++iter )
    {
      boost::int64_t f = (*iter)->frame();
      if ( f == frame )  std::cerr << "S";
      if ( f == _dts )   std::cerr << "D";
      if ( f == _frame ) std::cerr << "F";
      std::cerr << f << " ";
    }
  std::cerr << endl;
}

void aviImage::debug_video_stores(const boost::int64_t frame, 
				  const char* routine)
{

  SCOPED_LOCK( _mutex );

  video_cache_t::const_iterator iter = _images.begin();
  video_cache_t::const_iterator last = _images.end();
  
  std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame 
	    << " " << routine << " video stores  #" 
	    << _images.size() << ": " << (*iter)->frame() << "-" 
	    << (*(last-1))->frame() 
	    << std::endl;

#ifdef DEBUG_VIDEO_STORES_DETAIL
  for ( ; iter != last; ++iter )
    {
      boost::int64_t f = (*iter)->frame();
      if ( f == frame )  std::cerr << "S";
      if ( f == _dts )   std::cerr << "D";
      if ( f == _frame ) std::cerr << "F";
      std::cerr << f << " ";
    }
  std::cerr << endl;
#endif
}



void aviImage::debug_subtitle_packets(const boost::int64_t frame, 
				      const char* routine)
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

  bool in_preroll = false;
  bool in_seek = false;
  for ( ; iter != last; ++iter )
    {
      if ( _subtitle_packets.is_flush( *iter ) )
	{
	  std::cerr << "* "; continue;
	}
      else if ( _subtitle_packets.is_loop_start( *iter ) ||
		_subtitle_packets.is_loop_end( *iter ) )
	{
	  std::cerr << "L "; continue;
	}

      assert( (*iter).dts != MRV_NOPTS_VALUE );
      boost::int64_t f = pts2frame( get_subtitle_stream(), (*iter).dts );
      if ( _subtitle_packets.is_seek( *iter ) )
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


void aviImage::do_seek()
{
  if ( _dts == _seek_frame ) return;

  _dts = _seek_frame;


  bool got_image = !has_video();
  bool got_audio = !has_audio();

  if ( !got_image )
    {
      got_image = in_video_store( _dts );
    }

  if ( !got_audio )
    {
      got_audio = in_audio_store( _dts );
    }

  if ( !got_image || !got_audio )
    {
      if ( _seek_frame != _expected )
      	{
      	   clear_packets();
      	  _expected = _dts - 1;
      	}

      fetch(_seek_frame);
    }


  if ( stopped() )
    {
      if ( has_audio() )
	{
	  decode_audio( _seek_frame );
	}
      
      if ( has_video() )
	{
	  decode_video( _seek_frame );
	  find_image( _seek_frame );
	}

      if ( has_subtitle() )
	{
	  decode_subtitle( _seek_frame );
	  find_subtitle( _seek_frame );
	}


#ifdef DEBUG_STORES
      debug_video_stores(_seek_frame, "doseek" );
      debug_audio_stores(_seek_frame, "doseek" );
#endif

    }

  // Seeking done, turn flag off
  _seek_req = false;
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

  unsigned r,g,b,a;

  const unsigned* pal = (const unsigned*)rect.pict.data[1];

  for ( int x = dstx; x < dstx + dstw; ++x )
  {
     for ( int y = dsty; y < dsty + dsth; ++y )
     {
  	boost::uint8_t* d = root + 4 * (x + y * imgw); 
	assert( d != NULL );

	boost::uint8_t* const s = 
	rect.pict.data[0] + 
	(x-dstx) + (y-dsty) * dstw;

	unsigned v = pal[*s];
	a = (v >> 24) & 0xff;
	r = (v >> 16) & 0xff;
	g = (v >> 8) & 0xff;
	b = v & 0xff;

	*d++ = r;
	*d++ = g;
	*d++ = b;
	*d++ = a;
     }
  }
}


// Flush subtitle buffers
void aviImage::flush_subtitle()
{
  if ( has_subtitle() )
    avcodec_flush_buffers( get_subtitle_stream()->codec );
}

void aviImage::subtitle_stream( int idx )
{
  if ( idx != -1 && unsigned(idx) >= number_of_subtitle_streams() )
    throw "Subtitle stream invalid";

  if ( idx == _subtitle_index ) return;

  mrv::PacketQueue::Mutex& apm = _subtitle_packets.mutex();
  SCOPED_LOCK( apm );

  flush_subtitle();
  close_subtitle_codec();
  _subtitle_packets.clear();

  _subtitle_index = idx;

  if ( _subtitle_index >= 0 )
    {
      open_subtitle_codec();
      seek( _frame );
    }
}

void aviImage::store_subtitle( const boost::int64_t frame,
			       const boost::int64_t repeat )
{
  if ( _sub.format != 0 )
    {
      IMG_ERROR("Subtitle format " << _sub.format << " not yet handled");
      subtitle_stream(-1);
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
	      // subtitle_text_to_image( *rect );
	      std::cerr << rect->text << std::endl;
	      break;
	   case SUBTITLE_ASS:
	      subtitle_rect_to_image( *rect );
	      break;
	}
     }
  }

  av_free(_sub.rects);

  memset(&_sub, 0, sizeof(AVSubtitle));

}


CMedia::DecodeStatus
aviImage::decode_subtitle_packet( boost::int64_t& ptsframe, 
				  boost::int64_t& repeat,
				  const boost::int64_t frame, 
				  const AVPacket& pkt
				  )
{
  AVStream* stream = get_subtitle_stream();

  if ( pkt.pts != MRV_NOPTS_VALUE )
    { 
       ptsframe = pts2frame( stream, pkt.pts + 
			     _sub.start_display_time / 1000.0);
       boost::int64_t endframe = pts2frame( stream, pkt.pts + 
					    _sub.end_display_time / 1000.0);
       repeat = endframe - ptsframe + 1;
    }
  else
    {
      ptsframe = pts2frame( stream, pkt.dts + 
			     _sub.start_display_time / 1000.0);
      boost::int64_t endframe = pts2frame( stream, pkt.dts + 
					   _sub.end_display_time / 1000.0);
      repeat = endframe - ptsframe + 1;
      IMG_ERROR("Could not determine pts for subtitle frame, "
		"using " << ptsframe );
    }

  if ( repeat <= 1 )
  {
     repeat = fps() * 3;
  }

  int got_sub = 0;
  avcodec_decode_subtitle2( stream->codec, &_sub, &got_sub, 
			    (AVPacket*)&pkt );
  if ( got_sub == 0 ) return kDecodeError;

  // AVSubtitle has a start display time in ms. relative to pts
  // ptsframe = ptsframe + boost::int64_t( _sub.start_display_time * fps() / 
  // 					1000 );

  return kDecodeOK;
}

// Decode the subtitle
CMedia::DecodeStatus
aviImage::decode_subtitle( const boost::int64_t frame, const AVPacket& pkt )
{
   boost::int64_t ptsframe, repeat;

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
aviImage::handle_subtitle_packet_seek( boost::int64_t& frame, 
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

  _subtitle_packets.pop_front();  // pop seek begin packet

  DecodeStatus got_subtitle = kDecodeMissingFrame;

  while ( !_subtitle_packets.is_seek() && !_subtitle_packets.empty() )
    {
      const AVPacket& pkt = _subtitle_packets.front();
      
      boost::int64_t repeat;
      boost::int64_t pktframe = get_frame( get_subtitle_stream(), pkt );


      if ( !is_seek && _playback == kBackwards && 
	   pktframe >= frame )
	{
	   boost::int64_t ptsframe, repeat;
	   DecodeStatus status = decode_subtitle_packet( ptsframe, repeat, 
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
	      got_subtitle = decode_subtitle( frame, pkt );
	    }
	  else
	    {
	       decode_subtitle_packet( pktframe, repeat, frame, pkt );
	    }
	}

      _subtitle_packets.pop_front();
    }

  if ( _subtitle_packets.empty() ) return kDecodeError;

  {
    const AVPacket& pkt = _subtitle_packets.front();
    frame = get_frame( get_subtitle_stream(), pkt );
  }

  _subtitle_packets.pop_front();  // pop seek end packet

      
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
      if ( stopped() ) break;

      if ( ! _subtitle_packets.empty() )
	{
	  const AVPacket& pkt = _subtitle_packets.front();
	  return pts2frame( get_subtitle_stream(), pkt.pts );
	}

      CONDITION_WAIT( _subtitle_packets.cond(), spm );
    }
  return _frame;
}

CMedia::DecodeStatus aviImage::decode_subtitle( boost::int64_t& frame )
{

  mrv::PacketQueue::Mutex& vpm = _subtitle_packets.mutex();
  SCOPED_LOCK( vpm );

  if ( _subtitle_packets.empty() )
    {
      bool ok = in_subtitle_store( frame );
      if ( ok ) return kDecodeOK;
      return kDecodeMissingFrame;
    }

  DecodeStatus got_image = kDecodeMissingFrame;

  while ( got_image != kDecodeOK && !_subtitle_packets.empty() )
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
		   boost::int64_t ptsframe, repeat;
		   decode_subtitle_packet( ptsframe, repeat, frame, pkt );
		   _subtitle_packets.pop_front();
		}
	      return kDecodeOK;
	    }

	  got_image = decode_subtitle( frame, pkt );

	  _subtitle_packets.pop_front();
	}

    }


#ifdef DEBUG_SUBTITLE_STORES
  debug_subtitle_stores(frame, "decode_subtitle");
#endif

  return got_image;
}


bool aviImage::in_subtitle_store( const boost::int64_t frame )
{
   SCOPED_LOCK( _subtitle_mutex );

   // Check if audio is already in audio store
   // If so, no need to decode it again.  We just pop the packet and return.
   subtitle_cache_t::iterator end = _subtitles.end();
   subtitle_cache_t::iterator i = std::find_if( _subtitles.begin(), end,
						EqualFunctor(frame) );
   if ( i != end ) return true;
   return false;
}


void aviImage::loop_at_start( const boost::int64_t frame )
{
  if ( number_of_video_streams() > 0 )
    {
      _video_packets.loop_at_start( frame );
    }

  if ( number_of_audio_streams() > 0 )
    {
      _audio_packets.loop_at_start( frame );
    }

  if ( number_of_subtitle_streams() > 0  )
    {
      _subtitle_packets.loop_at_start( frame );
    }
}



void aviImage::loop_at_end( const boost::int64_t frame )
{
  if ( has_video() || is_sequence() )
    {
      _video_packets.loop_at_end( frame );
    }

  if ( has_audio() )
    {
      _audio_packets.loop_at_end( frame );
    }

  if ( has_subtitle() )
    {
      _subtitle_packets.loop_at_end( frame );
    }
}



} // namespace mrv
