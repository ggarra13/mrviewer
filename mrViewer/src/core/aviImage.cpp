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
#  include <arpa/inet.h>
#else
#  include <winsock2.h>    // for htonl
#endif



extern "C" {
#include "libavutil/audioconvert.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"

}

#include "aviImage.h"
#include "mrvImageView.h"
#include "mrvPlayback.h"
#include "gui/mrvIO.h"
#include "mrvFrameFunctors.h"
#include "mrvThread.h"
#include "mrvCPU.h"
#include "mrvColorSpaces.h"
#include "mrViewer.h"
#include "gui/mrvIO.h"


namespace 
{
  const char* kModule = "avi";
}


#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )
#define IMG_WARNING(x) LOG_WARNING( name() << " - " << x )
#define LOG(x) std::cerr << x << std::endl;

#ifndef CodecID
#define CodecID AVCodecID
#endif

// this plays backwards by seeking, which is slow but memory efficient
// #define USE_SEEK_TO_PLAY_BACKWARDS  

//#define DEBUG_STREAM_INDICES
//#define DEBUG_STREAM_KEYFRAMES
//#define DEBUG_DECODE
// #define DEBUG_SEEK
//#define DEBUG_SEEK_VIDEO_PACKETS
// #define DEBUG_SEEK_AUDIO_PACKETS
//#define DEBUG_SEEK_SUBTITLE_PACKETS
//#define DEBUG_VIDEO_PACKETS
//#define DEBUG_VIDEO_STORES
//#define DEBUG_AUDIO_PACKETS
//#define DEBUG_PACKETS
//#define DEBUG_PACKETS_DETAIL
//#define DEBUG_AUDIO_STORES
//#define DEBUG_STORES_DETAIL
//#define DEBUG_SUBTITLE_STORES
//#define DEBUG_SUBTITLE_RECT



//  in ffmpeg, sizes are in bytes...
#define kMAX_QUEUE_SIZE (15 * 1024 * 1024)
#define kMAX_AUDIOQ_SIZE (20 * 16 * 1024)
#define kMAX_SUBTITLEQ_SIZE (5 * 30 * 1024)
#define kMIN_FRAMES 5

namespace {
  const unsigned int  kMaxCacheImages = 70;
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

  if ( magic == 0x000001ba )
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
  else if ( magic == 0x00000144 )
  {
     // RED ONE camera images
     if ( strncmp( (char*)data+4, "RED1", 4 ) != 0 )
	return false;
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
  static int lowres = 0;
  static enum AVDiscard skip_frame= AVDISCARD_DEFAULT;
  static enum AVDiscard skip_idct= AVDISCARD_DEFAULT;
  static enum AVDiscard skip_loop_filter= AVDISCARD_DEFAULT;
  static int idct = FF_IDCT_AUTO;
  static int error_concealment = 3;

  ctx->codec_id        = _video_codec->id;
  ctx->idct_algo         = FF_IDCT_AUTO;
  ctx->workaround_bugs = workaround_bugs;
  ctx->lowres          = lowres;
  ctx->skip_frame= skip_frame;
  ctx->skip_idct = skip_idct;
  ctx->idct_algo = idct;
  ctx->skip_loop_filter= skip_loop_filter;
  ctx->error_concealment= error_concealment;

  if(_video_codec->capabilities & CODEC_CAP_DR1)
     ctx->flags |= CODEC_FLAG_EMU_EDGE;

  double aspect_ratio;
  if ( ctx->sample_aspect_ratio.num == 0 )
    aspect_ratio = 0;
  else
    aspect_ratio = av_q2d( ctx->sample_aspect_ratio ) *
    ctx->width / ctx->height;



  if ( width() > 0 && height() > 0 )
  {
     double image_ratio = (double) width() / (double)height();
     if ( aspect_ratio <= 0.0 ) aspect_ratio = image_ratio;

     if ( image_ratio == aspect_ratio ) _pixel_ratio = 1.0;
     else _pixel_ratio = aspect_ratio / image_ratio;
  }

  AVDictionary* info = NULL;
  av_dict_set(&info, "threads", "2", 0);

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
     {
	avcodec_flush_buffers( stream->codec );
     }
  }
}


/// VCR play (and cache frames if needed) sequence
void aviImage::play( const Playback dir, mrv::ViewerUI* const uiMain,
		     bool fg )
{
   CMedia::play( dir, uiMain, fg );
}

// Seek to the requested frame
bool aviImage::seek_to_position( const boost::int64_t frame )
{


   // double frac = ( (double) (frame - _frameStart) / 
   // 		   (double) (_frameEnd - _frameStart) );
   // boost::int64_t offset = boost::int64_t( _context->duration * frac );
   // if ( _context->start_time != AV_NOPTS_VALUE )
   //    offset += _context->start_time;

  // static const AVRational base = { 1, AV_TIME_BASE };
  // boost::int64_t min_ts = std::numeric_limits< boost::int64_t >::max();

   boost::int64_t offset = boost::int64_t( ((frame-1) * AV_TIME_BASE ) / fps() );

   int flags = 0;
   flags &= ~AVSEEK_FLAG_BYTE;

  // AVStream* stream = NULL;
  // int idx = -1;

  // if ( has_video() )
  // {
  //    idx = video_stream_index();
  //    stream = get_video_stream();

  //    int64_t ts = av_rescale_q(offset, base, stream->time_base);
  //    int i = av_index_search_timestamp( stream, ts, AVSEEK_FLAG_BACKWARD );
  //    if ( i >= 0 ) min_ts = stream->index_entries[i].timestamp;
  // }

  // if ( has_audio() )
  // {
  //    AVStream* st = get_audio_stream();

  //    int64_t ts = av_rescale_q(offset, base, st->time_base);
  //    int i = av_index_search_timestamp( st, ts, AVSEEK_FLAG_BACKWARD );

  //    int64_t new_ts = st->index_entries[i].timestamp;
  //    if ( i >= 0 && new_ts < min_ts ) {
  // 	min_ts = new_ts;
  // 	stream = st;
  // 	idx = audio_stream_index();
  //    }
  // }

  // offset = av_rescale_q(offset, base, stream->time_base);

  // int ret = av_seek_frame( _context, idx, offset, AVSEEK_FLAG_BACKWARD );

  int ret = avformat_seek_file( _context, -1, INT64_MIN, offset, 
   				INT64_MAX, flags );
  
  if (ret < 0)
  {
     IMG_ERROR( _("Could not seek to frame ") << frame );
     return false;
  }

  if ( _acontext )
  {
     int ret = avformat_seek_file( _acontext, -1, INT64_MIN, offset, 
				   INT64_MAX, flags );
  
     if (ret < 0)
     {
	IMG_ERROR( _("Could not seek to frame ") << frame );
	return false;
     }
  }


  bool got_audio = !has_audio();
  bool got_video = !has_video();
  bool got_subtitle = !has_subtitle();

  // if ( !got_video )
  //   {
  //     got_video = in_video_store( frame );
  //   }

  // if ( !got_audio )
  //   {
  //     got_audio = in_audio_store( frame );
  //   }

  // if ( !got_subtitle )
  //   {
  //     got_subtitle = in_subtitle_store( frame );
  //   }


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

  if ( !_seek_req && playback() == kBackwards )
  {
     if ( !got_video )    _video_packets.preroll(vpts);
     if ( !got_audio )    _audio_packets.preroll(apts);
     if ( !got_subtitle ) _subtitle_packets.preroll(spts);
  }
  else
  {
     if ( !got_video )    _video_packets.seek_begin(vpts);
     if ( !got_audio )    _audio_packets.seek_begin(apts);
     if ( !got_subtitle ) _subtitle_packets.seek_begin(spts);
  }

#ifdef DEBUG_SEEK_VIDEO_PACKETS
  LOG_INFO( "BEFORE SEEK:" );
  debug_video_packets(frame);
#endif

#ifdef DEBUG_SEEK_AUDIO_PACKETS
  debug_audio_packets(frame);
#endif


  boost::int64_t dts = queue_packets( frame, true, got_video,
				      got_audio, got_subtitle );


  //
  // When pre-rolling, make sure new dts is not at a distance bigger
  // than our image/audio cache.
  //

  if (! _seek_req )
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
     
     if ( abs(diff) > max_frames )
     {
	dts = _dts + int64_t(max_frames) * _playback;
	if ( dts < first_frame() )
	   dts = first_frame();
	else if ( dts > last_frame() )
	   dts = last_frame();
     }
  }
 

  _dts = dts;
  assert( _dts >= first_frame() && _dts <= last_frame() );

  _expected = dts + 1;
  _seek_req = false;


#ifdef DEBUG_SEEK
  LOG_INFO( "AFTER SEEK:  D: " << _dts << " E: " << _expected );
#endif

#ifdef DEBUG_VIDEO_STORES
  debug_video_stores(frame);
#endif

#ifdef DEBUG_AUDIO_STORES
  debug_audio_stores(frame);
#endif

#ifdef DEBUG_SEEK_VIDEO_PACKETS
  debug_video_packets(frame);
#endif

#ifdef DEBUG_SEEK_AUDIO_PACKETS
  debug_audio_packets(frame);
#endif


  return true;
}


mrv::image_type_ptr aviImage::allocate_image( const boost::int64_t& frame,
					      const boost::int64_t& pts
                                             )
{ 
  return mrv::image_type_ptr( new image_type( frame,
					      width(), 
					      height(), 
					      _num_channels,
					      _pix_fmt,
					      _ptype,
					      _av_frame->repeat_pict,
					      pts ) );

}


void aviImage::store_image( const boost::int64_t frame, 
			    const boost::int64_t pts )
{
  AVStream* stream = get_video_stream();
  unsigned int w = width();
  unsigned int h = height();


  mrv::image_type_ptr 
  image = allocate_image( frame, boost::int64_t( pts * av_q2d( 
							      stream->time_base
                                                 ) )
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
			       const AVPacket& p
			       )
{
   AVPacket pkt = p;

  AVStream* stream = get_video_stream();


  int got_pict = 0;


  while( pkt.size > 0 || pkt.data == NULL )
  {
     int err = avcodec_decode_video2( stream->codec, _av_frame, &got_pict, 
				      &pkt );

     if ( got_pict ) {
	ptsframe = av_frame_get_best_effort_timestamp( _av_frame );

	if ( ptsframe == MRV_NOPTS_VALUE )
	   ptsframe = frame;
	else
	   ptsframe = pts2frame( stream, ptsframe );

	store_image( ptsframe, pkt.dts );

	return kDecodeOK;
     }

     if ( err < 0 ) {
	return kDecodeError;
     }
     
     if ( err == 0 ) return kDecodeMissingFrame;


     pkt.size -= err;
     pkt.data += err;
  }

  if ( got_pict ) return kDecodeOK;

  return kDecodeMissingFrame;
}



// Decode the image
CMedia::DecodeStatus
aviImage::decode_image( const boost::int64_t frame, AVPacket& pkt )
{
  boost::int64_t ptsframe = frame;

  DecodeStatus status = decode_video_packet( ptsframe, frame, pkt );
  if ( status == kDecodeError )
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
  }

  return status;
}

void aviImage::clear_packets()
{

#ifdef DEBUG_AUDIO_PACKETS
   cerr << "+++++++++++++ CLEAR VIDEO/AUDIO PACKETS " << _frame 
	<< " expected: " << _expected << endl;
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
      last  = frame + max_video_frames();
      if ( _dts > last )   last = _dts;
      break;
    default:
      first = frame - max_video_frames()/2;
      last  = frame + max_video_frames()/2;
      break;
    }

  if ( _images.empty() ) return;




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
  static int error_concealment = 3;

  ctx->idct_algo         = FF_IDCT_AUTO;
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

#ifdef DEBUG_VIDEO_PACKETS
  debug_video_packets(frame, "find_image");
#endif

#ifdef DEBUG_VIDEO_STORES
  debug_video_stores(frame, "find_image");
#endif

  _frame = frame;

  {

    SCOPED_LOCK( _mutex );

    video_cache_t::iterator end = _images.end();
    video_cache_t::iterator i;

    if ( playback() == kBackwards )
    {
       i = std::upper_bound( _images.begin(), end, 
			     frame, LessThanFunctor() );
    }
    else
    {
       i = std::lower_bound( _images.begin(), end, 
			     frame, LessThanFunctor() );
    }

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
	
	if ( ! _images.empty() )
	  {
	    _hires = _images.back();

	    if ( _hires->frame() != frame && 
		 abs(frame - _hires->frame() ) < 10 )
	       IMG_WARNING( N_("find_image: frame ") << frame 
			    << _(" not found, choosing ") << _hires->frame() 
			    << _(" instead") );
	    refresh();
	  }
	else
	  {
	     IMG_ERROR( "find_image: frame " << frame << _(" not found") );
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

  static PixelFormat fmt[] = { PIX_FMT_BGR24, PIX_FMT_BGR32, PIX_FMT_NONE };
  PixelFormat* fmts = fmt;


  if ( supports_yuv() )
    {
       static PixelFormat fmts2[] = { PIX_FMT_BGR24, PIX_FMT_BGR32,
				      PIX_FMT_YUV444P,
				      PIX_FMT_YUV422P,
				      PIX_FMT_YUV420P,
				      PIX_FMT_NONE };
       fmts = fmts2;

//       mask |= ( (1 << PIX_FMT_YUVA420P) | (1 << PIX_FMT_YUV444P) | 
// 		(1 << PIX_FMT_YUV422P) | (1 << PIX_FMT_YUV420P) );
    }

  AVStream* stream = get_video_stream();
  AVCodecContext* codec = stream->codec;

  int has_alpha = ( ( codec->pix_fmt == PIX_FMT_RGBA    ) |
		    ( codec->pix_fmt == PIX_FMT_ABGR    ) |
		    ( codec->pix_fmt == PIX_FMT_ARGB    ) |
		    ( codec->pix_fmt == PIX_FMT_RGB32   ) |
		    ( codec->pix_fmt == PIX_FMT_RGB32_1 ) |
		    ( codec->pix_fmt == PIX_FMT_PAL8    ) | 
		    ( codec->pix_fmt == PIX_FMT_BGR32   ) | 
		    ( codec->pix_fmt == PIX_FMT_BGR32_1 ) );

  _av_dst_pix_fmt = avcodec_find_best_pix_fmt_of_list( fmts, 
						       codec->pix_fmt,
						       has_alpha, NULL );


  _num_channels = 0;
  _layers.clear();
		
  rgb_layers();
  lumma_layers();

  if ( _av_dst_pix_fmt == PIX_FMT_RGBA ||
       _av_dst_pix_fmt == PIX_FMT_BGRA ||
       _av_dst_pix_fmt == PIX_FMT_YUVA420P) alpha_layers();

  if (codec->lowres) {
     codec->flags |= CODEC_FLAG_EMU_EDGE;
  }

  _ptype = VideoFrame::kByte;
  unsigned int w = codec->width;

  switch( _av_dst_pix_fmt )
    {
       case AV_PIX_FMT_RGBA64BE:
          _ptype = VideoFrame::kShort; break;
          _pix_fmt = VideoFrame::kRGBA; break;
       case AV_PIX_FMT_RGBA64LE:
          _ptype = VideoFrame::kShort; break;
          _pix_fmt = VideoFrame::kRGBA; break;
       case AV_PIX_FMT_BGRA64BE:
       case AV_PIX_FMT_BGRA64LE:
          _pix_fmt = VideoFrame::kBGRA; break;
       case AV_PIX_FMT_BGR24:
          _pix_fmt = VideoFrame::kBGR; break;
       case AV_PIX_FMT_BGRA:
          _pix_fmt = VideoFrame::kBGRA; break;
       case AV_PIX_FMT_RGB24:
      _pix_fmt = VideoFrame::kRGB; break;
    case AV_PIX_FMT_RGBA:
      _pix_fmt = VideoFrame::kRGBA; break;
    case AV_PIX_FMT_YUV444P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_709_YCbCr444; 
      else
	_pix_fmt = VideoFrame::kITU_601_YCbCr444; 
      break;
    case AV_PIX_FMT_YUV422P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_709_YCbCr422;
      else
	_pix_fmt = VideoFrame::kITU_601_YCbCr422;
      break;
    case AV_PIX_FMT_YUV420P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_709_YCbCr420;
      else
	_pix_fmt = VideoFrame::kITU_601_YCbCr420;
      break;
    case AV_PIX_FMT_YUVA420P:
      if ( w > 768 )
	_pix_fmt = VideoFrame::kITU_709_YCbCr420A;
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
		 populate_stream_info( s, msg, _context, ctx, i );
		 s.has_b_frames = ( ctx->has_b_frames != 0 );
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
		 populate_stream_info( s, msg, _context, ctx, i );
		 
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
		    s.language = "default";
		 }

		 const char* fmt = av_get_sample_fmt_name( ctx->sample_fmt );
		 if ( fmt ) s.format = fmt;

		 _audio_info.push_back( s );
		 if ( _audio_index < 0 && s.has_codec )
		    _audio_index = 0;
		 break;
	      }
	   case AVMEDIA_TYPE_SUBTITLE:
	      {
		 
		 subtitle_info_t s;
		 populate_stream_info( s, msg, _context, ctx, i );
		 s.bitrate    = calculate_bitrate( ctx );
		 AVDictionaryEntry* lang = av_dict_get(stream->metadata, 
						       "language", NULL, 0);
		 if ( lang && lang->value )
		    s.language = lang->value;
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
  

  _frame_start = _frame = _frameStart;

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
      duration = int64_t( (_fps * ( double )(_context->duration) / 
			   ( double )AV_TIME_BASE ) + 0.5f );
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

      duration = boost::int64_t( length * _fps + 0.5f );
    }

  _frameEnd = _frameStart + duration - 1;
  _frame_end = _frameEnd;

  _frame_offset = 0;

  
  boost::int64_t dts = _frameStart;
 
  unsigned audio_bytes = 0;
  unsigned bytes_per_frame = audio_bytes_per_frame();

  if ( has_video() || has_audio() )
    {

      // Loop until we get first frame
      AVPacket pkt;
      // Clear the packet
      av_init_packet( &pkt );

      bool eof = false;
      short counter = 0;
      bool got_audio = ! has_audio();
      bool got_video = ! has_video();
      while( !got_video || !got_audio )
	{
	  int error = av_read_frame( _context, &pkt );
	  if ( error < 0 )
	  {
	     int err = _context->pb ? _context->pb->error : 0;
	     if ( err != 0 )
	     {
		IMG_ERROR("populate: Could not read frame 1 error: "
			  << strerror(err) );
                break;
	     }
	  }
	  
	  if ( has_video() && pkt.stream_index == video_stream_index() )
	  {
	       
	     DecodeStatus status = decode_image( _frameStart, pkt ); 
	     if ( status == kDecodeOK )
	     {
		got_video = true;
                store_image( _frameStart, pkt.dts );
	     }
	     else
	     {
		if ( !got_video )
		   _frame_offset += 1;
	     }
	  }
	  else
	     if ( has_audio() && pkt.stream_index == audio_stream_index() )
	     {
		
		boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );
		if ( playback() == kBackwards )
		{
		   // Only add packet if it comes before seek frame
		   if ( pktframe >= first_frame() )  
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
		   
		   if ( pktframe > _frameStart ) got_audio = true;
		   else if ( pktframe == _frameStart )
		   {
		      audio_bytes += pkt.size;
		      if ( audio_bytes >= bytes_per_frame ) got_audio = true;
		   }
		}
		
		
#ifdef DEBUG_DECODE
		fprintf( stderr, "\t[avi]POP. A f: %05" PRId64 " audio pts: %07" PRId64 
			 " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
			 pktframe, pkt.pts, pkt.dts, pktframe );
#endif
		continue;
	     }

	  av_free_packet( &pkt );
	}

      
      if ( has_picture() && !_acontext )
	 find_image( _frameStart );
    }
  

  _dts = dts;
  _frame = _audio_frame = _frameStart;
  _expected = _frame;

  // if ( _frame_offset > 3 ) _frame_offset = 0;


  
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
 
   
  if ( has_audio() )
  {
     AVStream* stream = get_audio_stream();
     if ( stream->metadata ) dump_metadata( stream->metadata, "Audio " );
  }
  
  if ( has_video() )
  {
     AVStream* stream = get_video_stream();
     if ( stream->metadata ) dump_metadata( stream->metadata, "Video " );
  }

}

void aviImage::probe_size( unsigned p ) 
{ 
   _context->probesize = p; 
   _context->max_analyze_duration = p;
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

	}
      else
	{
	  _context = NULL;
	  mrvALERT( filename() << _("\n\nCould not open file") );
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


boost::int64_t aviImage::queue_packets( const boost::int64_t frame,
					const bool is_seek,
					bool& got_video,
					bool& got_audio,
					bool& got_subtitle )
{

   boost::int64_t dts = frame;

  boost::int64_t vpts, apts, spts;

  if ( !got_video ) {
    vpts = frame2pts( get_video_stream(), frame );
  }

  if ( !got_audio ) {
    apts = frame2pts( get_audio_stream(), frame );
  }

  if ( !got_subtitle ) {
    spts = frame2pts( get_subtitle_stream(), frame );
  }


  AVPacket pkt;

  // Clear the packet
  av_init_packet( &pkt );

  unsigned int bytes_per_frame = audio_bytes_per_frame();
  unsigned int audio_bytes = 0;

  int eof = false;
  unsigned counter = 0;
  unsigned packets_added = 0;

  // Loop until an error or we have what we need
  while( !got_video || (!got_audio && !_acontext) )
  {

     if (eof) {
	if (!got_video && video_stream_index() >= 0) {
	   av_init_packet(&pkt);
	   pkt.dts = pkt.pts = vpts;
	   pkt.data = NULL;
	   pkt.size = 0;
	   pkt.stream_index = video_stream_index();
	   packets_added++;
	   _video_packets.push_back( pkt );
	}

	AVStream* stream = get_audio_stream();
	if (!got_audio && audio_stream_index() >= 0 &&
	    stream->codec->codec->capabilities & CODEC_CAP_DELAY) {
	   av_init_packet(&pkt);
	   pkt.dts = pkt.pts = apts;
	   pkt.data = NULL;
	   pkt.size = 0;
	   pkt.stream_index = audio_stream_index();
	   _audio_packets.push_back( pkt );
	}

	eof = false;
     }

     int error = av_read_frame( _context, &pkt );

     if ( error < 0 )
     {
	if ( error == AVERROR_EOF )
	{
	   eof = true;
	   counter++;
	   if ( counter > _frame_offset ) {
	      if ( is_seek || playback() == kBackwards )
	      {
		 if ( !got_video    ) _video_packets.seek_end(vpts);
		 if ( !got_audio    ) _audio_packets.seek_end(apts);
		 if ( !got_subtitle ) _subtitle_packets.seek_end(spts);
	      }
	      break;
	   }
	   continue;
	}
	int err = _context->pb ? _context->pb->error : 0;
	if ( err != 0 )
	{
	   IMG_ERROR("fetch: Could not read frame " << frame << " error: "
		     << strerror(err) );
	}

	if ( is_seek )
	{
	   if ( !got_video    ) {
	      if ( packets_added > 0 )
		 _video_packets.seek_end(vpts);
	      else
	      {
		 _video_packets.pop_front(); // seek begin
		 _video_packets.pop_front(); // flush
	      }
	   }
	   if ( !got_audio    ) _audio_packets.seek_end(apts);
	   if ( !got_subtitle ) _subtitle_packets.seek_end(spts);
	}


	av_free_packet( &pkt );

	break;
     }


     if ( has_video() && pkt.stream_index == video_stream_index() )
     {
	boost::int64_t pktframe = pts2frame( get_video_stream(), pkt.dts )
	- _frame_offset;

	if ( playback() == kBackwards )
	{
	   packets_added++;
	   if ( pktframe <= frame )
	      _video_packets.push_back( pkt );
	   if ( pktframe < dts ) dts = pktframe;
	}
	else
	{
	   packets_added++;
	   _video_packets.push_back( pkt );
	   if ( pktframe >= dts ) dts = pktframe;
	}

	if ( !got_video && pktframe >= frame )
	{
	   got_video = true;
	   if ( is_seek ) {
	      if ( packets_added == 0 )
	      {
		 _video_packets.pop_front(); // seek begin
		 _video_packets.pop_front(); // flush
	      }
	      else
	      {
		 _video_packets.seek_end(vpts);
	      }
	   }
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
	boost::int64_t pktframe = get_frame( get_subtitle_stream(), pkt );
	if ( playback() == kBackwards )
	{
	      if ( pktframe <= frame )
		 _subtitle_packets.push_back( pkt );		 }
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
	
	if ( has_audio() && !_acontext &&
	     pkt.stream_index == audio_stream_index() )
	{

	   boost::int64_t pktframe = get_frame( get_audio_stream(), pkt ) -
	   _frame_offset;
	   

	   if ( playback() == kBackwards )
	   {
	      // Only add packet if it comes before seek frame
	      if ( pktframe <= frame )
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
	      if ( pktframe > frame ) got_audio = true;
	      else if ( pktframe == frame )
	      {
		 audio_bytes += pkt.size;
		 if ( audio_bytes >= bytes_per_frame ) got_audio = true;
	      }
	      if ( is_seek && got_audio ) _audio_packets.seek_end(apts);
	   }
	  
	   
#ifdef DEBUG_DECODE
	   fprintf( stderr, "\t[avi] FETCH A f: %05" PRId64 
		    " audio pts: %07" PRId64 
		    " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
		    frame, pkt.pts, pkt.dts, pktframe );
#endif
	   continue;
	}
     }
     
     av_free_packet( &pkt );


  } // (!got_video || !got_audio)
    
	
  if ( _acontext )
  {

     while (!got_audio)
     {
	AVStream* stream = get_audio_stream();
	if (stream->codec->codec->capabilities & CODEC_CAP_DELAY) {
	   av_init_packet(&pkt);
	   pkt.dts = pkt.pts = apts;
	   pkt.data = NULL;
	   pkt.size = 0;
	   pkt.stream_index = audio_stream_index();
	   _audio_packets.push_back( pkt );
	}

	int error = av_read_frame( _acontext, &pkt );
	   
	if ( error < 0 )
	{
	   if ( error == AVERROR_EOF )
	   {
	      counter++;
	      if ( counter >= _frame_offset ) {
		 
		 if ( is_seek )
		 {
		    if ( !got_audio ) _audio_packets.seek_end(apts);
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
	      if ( !got_audio ) _audio_packets.seek_end(apts);
	   }
	}
     
	if ( pkt.stream_index == audio_stream_index() )
	{
	   boost::int64_t pktframe = get_frame( get_audio_stream(), pkt );

	   if ( playback() == kBackwards )
	   {
	      // Only add packet if it comes before seek frame
	      if ( pktframe <= frame ) _audio_packets.push_back( pkt );
	   }
	   else
	   {
	      if ( pktframe <= last_frame() )
		 _audio_packets.push_back( pkt );
	   }
	
	   if ( !got_audio )
	   {
	      if ( pktframe > frame ) got_audio = true;
	      else if ( pktframe == frame )
	      {
	      	 audio_bytes += pkt.size;
	      	 if ( audio_bytes >= bytes_per_frame ) got_audio = true;
	      }
	      if ( is_seek && got_audio ) _audio_packets.seek_end(apts);
	   }
	   
	   
#ifdef DEBUG_DECODE
	   fprintf( stderr, "\t[avi] FETCH A f: %05" PRId64 
		    " audio pts: %07" PRId64 
		    " dts: %07" PRId64 " as frame: %05" PRId64 "\n",
		    frame, pkt.pts, pkt.dts, pktframe );
#endif
	   continue;
	}

	av_free_packet( &pkt );
     }
  }

  //debug_video_packets( dts, "queue_packets");
  
  if ( dts > last_frame() ) dts = last_frame();
  else if ( dts < first_frame() ) dts = first_frame();

  return dts;
}

bool aviImage::fetch(const boost::int64_t frame)
{
#ifdef DEBUG_DECODE
   cerr << "FETCH BEGIN: " << frame << " EXPECTED: " << _expected << endl;
#endif

   bool got_video = !has_video();
   bool got_audio = !has_audio();
   bool got_subtitle = !has_subtitle();

  // if ( !got_audio )
  // {
  //    got_audio = in_audio_store( frame );
  // }

  // if ( !got_video )
  // {
  //    got_video = in_video_store( frame );
  // }

  // if ( !got_subtitle )
  // {
  //    got_subtitle = in_subtitle_store( frame );
  // }

  if ( (!got_video || !got_audio || !got_subtitle) && frame != _expected )
    {
       bool ok = seek_to_position( frame );
       if ( !ok )
	  IMG_ERROR("seek_to_position: Could not seek to frame " 
		    << frame );
       return ok;
    }


#ifdef DEBUG_DECODE
  cerr << "------------------------------------------------------" << endl;
  cerr << "FETCH START: " << frame << " gotV:" << got_video << " gotA:" << got_audio << endl;
#endif

#ifdef DEBUG_VIDEO_PACKETS
  debug_video_packets(frame, "Fetch");
#endif
#ifdef DEBUG_VIDEO_STORES
  debug_video_stores(frame, "Fetch");
#endif

#ifdef DEBUG_AUDIO_PACKETS
  debug_audio_packets(frame, "Fetch");
#endif
#ifdef DEBUG_AUDIO_STORES
  debug_audio_stores(frame, "Fetch");
#endif



  boost::int64_t dts = queue_packets( frame, false, got_video, 
				      got_audio, got_subtitle);



  _dts = dts;
  assert( _dts >= first_frame() && _dts <= last_frame() );

  _expected = dts + 1;

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
  debug_audio_packets(frame, "FETCH DONE");
#endif

#ifdef DEBUG_AUDIO_STORES
  debug_audio_stores(frame, "FETCH DONE");
#endif


  return true;
}


bool aviImage::frame( const boost::int64_t f )
{

   if ( ( playback() != kStopped &&
	  ( has_video() && _video_packets.size() > kMIN_FRAMES ) &&
	  ( has_audio() && _audio_packets.size() > kMIN_FRAMES ) &&
	  ( _video_packets.bytes() +  _audio_packets.bytes() + 
	    _subtitle_packets.bytes() > kMAX_QUEUE_SIZE  
	    ) ) )
    {
       // std::cerr << "false return: " << std::endl;
       // std::cerr << "vp: " << _video_packets.size() << std::endl;
       // std::cerr << "ap: " << _audio_packets.size() << std::endl;
       // std::cerr << "sum: " <<
       // ( _video_packets.bytes() +  _audio_packets.bytes() + 
       // 	 _subtitle_packets.bytes() ) << " > " <<  kMAX_QUEUE_SIZE
       // 		 << std::endl;

     return false;
    }

  if ( f < _frameStart )    _dts = _frameStart;
  else if ( f > _frameEnd ) _dts = _frameEnd;
  else                      _dts = f;



  bool ok = fetch(_dts);

#ifdef DEBUG_DECODE
  LOG_INFO( "------- FRAME DONE _dts: " << _dts << " _frame: " 
	    << _frame << " _expected: "  << _expected );
#endif

  return ok;
}

CMedia::DecodeStatus 
aviImage::handle_video_packet_seek( boost::int64_t& frame, const bool is_seek )
{
#ifdef DEBUG_VIDEO_PACKETS
  debug_video_packets(frame, "BEFORE HSEEK");
#endif

#ifdef DEBUG_VIDEO_STORES
  debug_video_stores(frame, "BEFORE HSEEK");
#endif

  Mutex& mutex = _video_packets.mutex();
  SCOPED_LOCK( mutex );

  if ( is_seek && _video_packets.is_seek() )
     _video_packets.pop_front();  // pop seek begin packet
  else if ( !is_seek && _video_packets.is_preroll() )
     _video_packets.pop_front();
  else
     IMG_ERROR( "handle_video_packet_seek error - no seek/preroll packet" );

  DecodeStatus got_video = kDecodeMissingFrame;
  unsigned count = 0;

  while ( !_video_packets.empty() && !_video_packets.is_seek_end() )
    {
      const AVPacket& pkt = _video_packets.front();
      count += 1;

      boost::int64_t pktframe = pts2frame( get_video_stream(), pkt.dts ) -
      _frame_offset;

      if ( !is_seek && playback() == kBackwards )
	{
	   if (pktframe > frame )
	   {
	       decode_video_packet( pktframe, frame, pkt );
	   }
	   else
	   {
	      DecodeStatus status = decode_image( pktframe, (AVPacket&)pkt );
	      if ( status == kDecodeOK ) 
	      {
		 got_video = status;
		 store_image( pktframe, pkt.dts );
	      }
	   }
	}
      else
	{
	  if ( pktframe >= frame )
	    {
	       got_video = decode_image( pktframe, (AVPacket&)pkt );
	       if ( got_video != kDecodeOK ) 
	       {
		  IMG_ERROR( "Could not decode video frame " << pktframe );
	       }
	       else
	       {
		  store_image( pktframe, pkt.dts );
	       }
	    }
	  else
	    {
	       // decode_image( pktframe, pkt );
	       decode_video_packet( pktframe, frame, pkt );	  
	    }
	}

      _video_packets.pop_front();
    }

  if ( _video_packets.empty() ) return kDecodeError;

  if ( count != 0 && is_seek )
  {
    const AVPacket& pkt = _video_packets.front();
    frame = pts2frame( get_video_stream(), pkt.dts );
  }

  if ( _video_packets.is_seek_end() )
     _video_packets.pop_front();  // pop seek end packet

  if ( count == 0 ) {
     IMG_WARNING("Empty preroll" );
     return kDecodeError;
  }
      
#ifdef DEBUG_VIDEO_PACKETS
  debug_video_packets(frame, "AFTER HSEEK");
#endif

#ifdef DEBUG_VIDEO_STORES
  debug_video_stores(frame, "AFTER HSEEK");
#endif
  return got_video;
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
	  boost::int64_t pktframe = pts2frame( get_video_stream(), pkt.dts ) -
	  _frame_offset;
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

#ifdef DEBUG_VIDEO_PACKETS
  debug_video_packets(frame, "decode_video");
#endif

  mrv::PacketQueue::Mutex& vpm = _video_packets.mutex();
  SCOPED_LOCK( vpm );

  if ( _video_packets.empty() )
  {
     bool ok = in_video_store( frame );
     if ( ok ) return kDecodeOK;
     return kDecodeMissingFrame;
  }

  DecodeStatus got_video = kDecodeMissingFrame;

  while ( got_video != kDecodeOK && !_video_packets.empty() )
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
	   bool ok = in_video_store( frame );
	   if ( ok ) 
	   {
	      video_cache_t::const_iterator iter = _images.begin();
	      if ( (*iter)->frame() >= frame )
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

	   if ( ok && frame > first_frame() )
	   {
	      return kDecodeOK;
	   }

	   if ( frame <= first_frame() )
	   {
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
	  _video_packets.pop_front();
	  return kDecodeLoopEnd;
	}
      else
	{
	  AVPacket& pkt = _video_packets.front();

	  bool ok = in_video_store( frame );
	  if ( ok )
	    {
	       boost::int64_t pktframe;
	       if ( pkt.dts != MRV_NOPTS_VALUE )
		  pktframe = pts2frame( get_video_stream(), pkt.dts );
	       else
		  pktframe = frame;

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
	  if ( _images.size() >= max_video_frames() )
	  {
	     limit_video_store(frame);
	     return kDecodeBufferFull;
	  }


	  got_video = decode_image( frame, pkt );
	  _video_packets.pop_front();
	}

    }


#ifdef DEBUG_VIDEO_STORES
  debug_video_stores(frame, "decode_video");
#endif

  return got_video;
}




void aviImage::debug_subtitle_stores(const boost::int64_t frame, 
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
	boost::int64_t f = (*iter)->frame();
	if ( f == frame )  std::cerr << "S";
	if ( f == _dts )   std::cerr << "D";
	if ( f == _frame ) std::cerr << "F";
	std::cerr << f << " ";
     }
     std::cerr << endl;
  }
}

void aviImage::debug_video_stores(const boost::int64_t frame, 
				  const char* routine,
				  const bool detail )
{

  SCOPED_LOCK( _mutex );

  video_cache_t::const_iterator iter = _images.begin();
  video_cache_t::const_iterator last = _images.end();
  
  std::cerr << name() << " S:" << _frame << " D:" << _dts << " V:" << frame 
	    << " " << routine << " video stores  #" 
	    << _images.size() << ": ";


  if ( iter != last )
     std::cerr << (*iter)->frame() << "-" 
	       << (*(last-1))->frame() 
	       << std::endl;

  if ( detail )
  {
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
}


void aviImage::debug_subtitle_packets(const boost::int64_t frame, 
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
	   std::cerr << "* "; continue;
	}
	else if ( _subtitle_packets.is_loop_start( *iter ) ||
		  _subtitle_packets.is_loop_end( *iter ) )
	{
	   std::cerr << "L "; continue;
	}
	
	assert( (*iter).dts != MRV_NOPTS_VALUE );
	boost::int64_t f = pts2frame( get_subtitle_stream(), (*iter).dts );
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

  _dts = _seek_frame;

  bool got_video = !has_video();
  bool got_audio = !has_audio();

  if ( !got_audio || !got_video )
  {
     clear_packets();
     fetch( _seek_frame );
  }


  // Seeking done, turn flag off
  _seek_req = false;

  if ( stopped() )
    {

       DecodeStatus status;
       if ( has_audio() && !got_audio )
       {
	  status = decode_audio( _seek_frame );
	  if ( status != kDecodeOK )
	     IMG_ERROR( "Decode audio error: " << status 
			<< " for frame " << _seek_frame );
	  find_audio( _seek_frame );
       }
       
       if ( has_video() && !got_video )
       {
	  status = decode_video( _seek_frame );

	  if ( !find_image( _seek_frame ) && status != kDecodeOK )
	     IMG_ERROR( "Decode video error seek frame " 
			<< _seek_frame 
			<< " status: " << status );
       }
       
       if ( has_subtitle() )
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

  const unsigned* pal = (const unsigned*)rect.pict.data[1];

  for ( int x = dstx; x < dstx + dstw; ++x )
  {
     for ( int y = dsty; y < dsty + dsth; ++y )
     {
  	boost::uint8_t* d = root + 4 * (x + y * imgw); 
	assert( d != NULL );

	boost::uint8_t* const s = rect.pict.data[0] + (x-dstx) + 
                                  (y-dsty) * dstw;

	unsigned t = pal[*s];
	a = (t >> 24) & 0xff;
	yuv.r = float( (t >> 16) & 0xff );
	yuv.g = float( (t >> 8) & 0xff );
	yuv.b = float( t & 0xff );

	rgb = mrv::color::yuv::to_rgb( yuv );


	if ( rgb.r > 0xff ) rgb.r = 0xff;
	if ( rgb.r < 0x00 ) rgb.r = 0x00;

	if ( rgb.g > 0xff ) rgb.g = 0xff;
	if ( rgb.g < 0x00 ) rgb.g = 0x00;

	if ( rgb.b > 0xff ) rgb.b = 0xff;
	if ( rgb.b < 0x00 ) rgb.b = 0x00;


        // float w = (float)rgb.a / 255.0f;
        // rgb.r = (float)rgb.r * w;
        // rgb.g = (float)rgb.g * w;
        // rgb.b = (float)rgb.b * w;



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

void aviImage::store_subtitle( const boost::int64_t& frame,
			       const boost::int64_t& repeat )
{
  // if ( _sub.format != 0 )
  //   {
  //     IMG_ERROR("Subtitle format " << _sub.format << " not yet handled");
  //     subtitle_stream(-1);
  //     return;
  //   }

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

  boost::int64_t endframe;
  if ( pkt.pts != MRV_NOPTS_VALUE )
    { 
       ptsframe = pts2frame( stream, boost::int64_t( pkt.pts + 
						     _sub.start_display_time /
						     1000.0 ) );
       endframe = pts2frame( stream, boost::int64_t( pkt.pts + 
						     _sub.end_display_time /
						     1000.0 ) );
       repeat = endframe - ptsframe + 1;
    }
  else
    {
      ptsframe = pts2frame( stream, boost::int64_t( pkt.dts + 
						    _sub.start_display_time /
						    1000.0 ) );
      endframe = pts2frame( stream, boost::int64_t( pkt.dts + 
						    _sub.end_display_time / 
						    1000.0 ) );
      repeat = endframe - ptsframe + 1;
      IMG_ERROR("Could not determine pts for subtitle frame, "
		"using " << ptsframe );
    }

  if ( repeat <= 1 )
  {
     repeat = boost::int64_t( fps() * 4 );
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

  while ( !_subtitle_packets.is_seek_end() && !_subtitle_packets.empty() )
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

  if ( _subtitle_packets.is_seek_end() )
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
   if ( _subtitle_index < 0 ) return kDecodeOK;

  mrv::PacketQueue::Mutex& vpm = _subtitle_packets.mutex();
  SCOPED_LOCK( vpm );

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
		   boost::int64_t ptsframe, repeat;
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
  if ( has_video() || is_sequence() )
    {
      _video_packets.loop_at_start( frame );
    }

  if ( has_audio() )
    {
      _audio_packets.loop_at_start( frame );
    }

  if ( has_subtitle()  )
    {
      _subtitle_packets.loop_at_start( frame );
    }
}



void aviImage::loop_at_end( const boost::int64_t frame )
{

   if ( has_picture() )
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



static AVFrame *picture = NULL;
static int frame_count = 0, video_outbuf_size;

static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
    uint8_t *picture_buf;
    int size;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = (uint8_t*)av_malloc(size);
    if (!picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf,
                   pix_fmt, width, height);
    return picture;
}
static bool open_video(AVFormatContext *oc, AVStream *st,
		       const CMedia* img )
{
    AVCodecContext* c = st->codec;

    /* find the video encoder */
    AVCodec* codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
       LOG_ERROR( _("Video codec not found") );
       return false;
    }

    /* open the codec */
    if (avcodec_open2(c, codec, NULL) < 0) {
       LOG_ERROR( _("Could not open video codec") );
       return false;
    }

    
    /* Allocate the encoded raw picture. */
    picture = alloc_picture(c->pix_fmt, img->width(), img->height());
    if (!picture) {
       LOG_ERROR( _("Could not allocate picture") );
       return false;
    }

    return true;
}

static void close_video(AVFormatContext *oc, AVStream *st)
{

    avcodec_close(st->codec);
    av_free(picture->data[0]);
    av_free(picture);
}

/* prepare a dummy image */
static void fill_yuv_image(AVFrame *pict, const CMedia* img,
			   const mrv::ViewerUI* const ui )
{
   CMedia* m = (CMedia*) img;

   image_type_ptr hires = img->hires();

   unsigned w = img->width();
   unsigned h = img->height();

   for ( unsigned y = 0; y < h; ++y )
   {
      for ( unsigned x = 0; x < w; ++x )
      {
	 ImagePixel p = hires->pixel( x, y );

	 float gamma = 1.0f/img->gamma();
	 if ( gamma != 1.0f )
	 {
	    p.r = powf( p.r, gamma );
	    p.g = powf( p.g, gamma );
	    p.b = powf( p.b, gamma );

	    if (p.r < 0.0f) p.r = 0.0f;
	    if (p.g < 0.0f) p.g = 0.0f;
	    if (p.b < 0.0f) p.b = 0.0f;
	    if (p.r > 1.0f) p.r = 1.0f;
	    if (p.g > 1.0f) p.g = 1.0f;
	    if (p.b > 1.0f) p.b = 1.0f;
	 }

	 ImagePixel yuv = color::rgb::to_ITU601( p );

	 pict->data[0][y * pict->linesize[0]   + x   ] = uint8_t(yuv.r);

	 unsigned x2 = x / 2;
	 unsigned y2 = y / 2;

	 pict->data[1][y2 * pict->linesize[1] + x2 ] = uint8_t(yuv.g);
	 pict->data[2][y2 * pict->linesize[2] + x2 ] = uint8_t(yuv.b);
      }
   }
       
   pict->extended_data = pict->data;
       
}

static bool write_video_frame(AVFormatContext* oc, AVStream* st,
			      const CMedia* img, 
			      const mrv::ViewerUI* const ui )
{
    int ret;
    AVCodecContext* c = st->codec;

    if (oc->oformat->flags & AVFMT_RAWPICTURE) {
        /* Raw video case - the API will change slightly in the near
         * future for that. */
        AVPacket pkt;
        av_init_packet(&pkt);

        pkt.flags        |= AV_PKT_FLAG_KEY;
        pkt.stream_index  = st->index;
        pkt.data          = picture->data[0];
        pkt.size          = sizeof(AVPicture);

        int ret = av_interleaved_write_frame(oc, &pkt);
	if ( ret != 0 )
	   LOG_ERROR( _("Error while writing raw frame") );
    }
    else
    {
#if 1
       if (frame_count >= img->duration() ) {
        /* No more frames to compress. The codec has a latency of a few
         * frames if using B-frames, so we get the last frames by
         * passing the same picture again. */
	  LOG_INFO("NOTHING SENT");
       } 
       else 
#endif
       {
	  fill_yuv_image( picture, img, ui );
       }

       AVPacket pkt = { 0 };
    av_init_packet(&pkt);

    int got_packet = 0;

    /* encode the image */
    ret = avcodec_encode_video2(c, &pkt, picture, &got_packet);
    if (ret < 0) {
       LOG_ERROR( _("Error while encoding video frame") );
       return false;
    }

    /* If size is zero, it means the image was buffered. */
    if ( got_packet )
    {
       if (!ret && got_packet && pkt.size) {
	  pkt.stream_index = st->index;
       
	  /* Write the compressed frame to the media file. */
	  ret = av_interleaved_write_frame(oc, &pkt);
       }
       else
       {
	  ret = 0;
       }
    }

    if (ret != 0) {
       LOG_ERROR( _("Error while writing video frame") );
       return false;
    }
    
    frame_count++;
    }

    return true;
}
static AVStream* add_video_stream(AVFormatContext *oc,
				  AVCodec** codec,
				  CodecID codec_id,
				  const CMedia* img )
{

   if ( img->width() == 0 ) return NULL;

   /* find the video encoder */
   *codec = avcodec_find_encoder(codec_id);
   if (!(*codec)) {
       LOG_ERROR( _( "Video codec not found") );
       return NULL;
    }

    AVStream* st = avformat_new_stream(oc, *codec);
    if (!st) {
       LOG_ERROR( _("Could not alloc stream") );
       return NULL;
    }

    AVCodecContext* c = st->codec;

    avcodec_get_context_defaults3(c, *codec);

    c->codec_id = codec_id;

 
    /* resolution must be a multiple of two */
    c->width = (img->width() / 2) * 2;
    c->height = (img->height() / 2) * 2;

    /* put sample parameters */
    c->bit_rate = c->width * c->height * 3;
    // c->rc_max_rate = ??;
    // c->field_order = -1;
    c->bit_rate_tolerance = 400000;
    //c->compression_level = FF_COMPRESSION_DEFAULT;
    /* time base: this is the fundamental unit of time (in seconds) in terms
       of which frame timestamps are represented. for fixed-fps content,
       timebase should be 1/framerate and timestamp increments should be
       identically 1. */
    c->time_base.den = int(1000 * img->play_fps());
    c->time_base.num = 1000;
    // c->ticks_per_frame = 1;
    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    //c->max_b_frames = 0;
    //c->me_method = 5;
    // if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
    //     /* just for testing, we also add B frames */
    //    c->max_b_frames = 2;
    //    // c->b_quant_factor = 1;
    //    // c->b_quant_offset = 0.0f;
    // }
    // c->mpeg_quant = 0;
    // c->i_quant_factor = 1.0f;
    // c->i_quant_offset = 0.0f;
    // c->p_masking = 0.0f;
    // c->dark_masking = 0.0f;
    // c->me_cmp = 7;
    // c->me_sub_cmp = 7;
    // c->ildct_cmp = FF_CMP_SSE;
    // c->last_predictor_count = 2;
    // c->pre_me = 7;
    // c->me_pre_cmp = 7;
    // c->pre_dia_size = 8;
    // c->me_subpel_quality = 2;
    // c->me_range = 0;
    // c->intra_quant_bias = FF_DEFAULT_QUANT_BIAS;
    // c->inter_quant_bias = FF_DEFAULT_QUANT_BIAS;
    // c->mb_decision = FF_MB_DECISION_RD;
    // c->me_threshold = 8;
    // c->mb_threshold = 8;
    // c->intra_dc_precision = 1;
    // c->keyint_min = 4;

    int qscale = 2;
    c->flags |= CODEC_FLAG_QSCALE;
    c->global_quality = FF_QP2LAMBDA * qscale;

    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

static AVFormatContext *oc = NULL;
static AVOutputFormat* fmt = NULL;
static AVStream* audio_st = NULL, *video_st = NULL;

/**************************************************************/
/* audio output */

AVSampleFormat aformat;
static uint8_t **src_samples_data = NULL;
static int       src_samples_linesize;
static int       src_nb_samples;
static unsigned  src_samples_size;

static int max_dst_nb_samples;
uint8_t **dst_samples_data = NULL;
int       dst_samples_linesize;
int       dst_samples_size;

struct SwrContext *swr_ctx = NULL;

AVCodec* audio_cdc, *video_codec;



/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

static inline
int64_t get_valid_channel_layout(int64_t channel_layout, int channels)
{
    if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
        return channel_layout;
    else
        return 0;
}


/*
 * add an audio output stream
 */
static AVStream *add_audio_stream(AVFormatContext* oc,
				  AVCodec** codec,
				  enum CodecID codec_id,
				  const CMedia* img )
{
   int ret = 0;

    /* find the audio encoder */
   codec_id = AV_CODEC_ID_PCM_S16LE;
   *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
       LOG_ERROR( _("Audio codec not found") );
       return NULL;
    }

 
    AVStream* st = avformat_new_stream(oc, *codec);
    if (!st) {
       LOG_ERROR( _("Could not alloc stream") );
       return NULL;
    }

    st->id = 1;

    AVCodecContext* c = st->codec;
    c->codec_id = codec_id;
    // c->strict_std_compliance= FF_COMPLIANCE_EXPERIMENTAL;

    /* put sample parameters */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    //c->sample_fmt = AV_SAMPLE_FMT_FLTP;

    if (!check_sample_fmt(*codec, c->sample_fmt)) {
       LOG_ERROR( _("Encoder does not support ") <<
		 av_get_sample_fmt_name(c->sample_fmt));
       return NULL;
    }

    c->bit_rate = 64000;
    c->sample_rate = img->audio_frequency();
    assert( c->sample_rate > 0 );

    c->channels = img->audio_channels();

    assert( c->channels > 0 );

    c->channel_layout = get_valid_channel_layout( AV_CH_LAYOUT_MONO,
    						  c->channels );

    if ( c->channel_layout == 0 )
       c->channel_layout = get_valid_channel_layout( AV_CH_LAYOUT_STEREO,
						     c->channels );
    if ( c->channel_layout == 0 )
       c->channel_layout = get_valid_channel_layout( AV_CH_LAYOUT_5POINT1,
						     c->channels );
    if ( c->channel_layout == 0 )
       c->channel_layout = get_valid_channel_layout( AV_CH_LAYOUT_7POINT1,
						     c->channels );


    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

static bool open_audio_static(AVFormatContext *oc, AVCodec* codec,
			      AVStream* st, const CMedia* img )

{
    AVCodecContext* c = st->codec;

   /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
       LOG_ERROR( _("Could not open audio codec" ) );
       return false;
    }
    
    if (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
    {
       src_nb_samples = 10000;
    }
    else
    {
        src_nb_samples = c->frame_size;
    }


    aformat = AudioEngine::ffmpeg_format( img->audio_format() );

    src_samples_size = ( src_nb_samples * c->channels * 
			 av_get_bytes_per_sample( aformat ) );

    if ( aformat != c->sample_fmt )
    {

        /* set options */
       swr_ctx  = swr_alloc();
       if ( swr_ctx == NULL )
       {
	  LOG_ERROR( "Could not alloc swr_ctx" );
	  return false;
       }


        av_opt_set_int       (swr_ctx, "in_channel_count",   c->channels, 0);
        av_opt_set_int       (swr_ctx, "in_sample_rate",     c->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt",      aformat, 0);
        av_opt_set_int       (swr_ctx, "out_channel_count",  c->channels,   0);
        av_opt_set_int       (swr_ctx, "out_sample_rate",    c->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt",     c->sample_fmt, 0);
	

	LOG_INFO( "Audio conversion of channels " << c->channels << ", freq "
		  << c->sample_rate << " " << av_get_sample_fmt_name( aformat ) 
		  << " to " << std::endl
		  << "                 channels " << c->channels 
		  << " freq " << c->sample_rate << " "
		  << av_get_sample_fmt_name( c->sample_fmt ) );
		   
	
        /* initialize the resampling context */
        if ((swr_init(swr_ctx)) < 0) {
           LOG_ERROR( _("Failed to initialize the resampling context") );
	   return false;
        }
    }

    int ret = av_samples_alloc_array_and_samples(&src_samples_data, 
						 &src_samples_linesize,
						 c->channels,
						 src_nb_samples, 
						 aformat, 0);
    if (ret < 0) {
       LOG_ERROR( _("Could not allocate source samples") );
        return false;
    }

    assert( src_samples_data[0] != NULL );
    assert( src_samples_linesize > 0 );

    int max_dst_nb_samples = src_nb_samples;
    assert( src_nb_samples > 0 );

    ret = av_samples_alloc_array_and_samples(&dst_samples_data, 
					     &dst_samples_linesize,
					     c->channels,
                                             max_dst_nb_samples,
					     c->sample_fmt, 0);
    if ( ret < 0 )
    {
       LOG_ERROR( _("Could not allocate destination samples") );
       return false;
    }
    
    assert( dst_samples_data[0] != NULL );
    assert( dst_samples_linesize != 0 );
    assert( max_dst_nb_samples > 0 );
    dst_samples_size = av_samples_get_buffer_size(NULL, c->channels, 
						  max_dst_nb_samples,
                                                  c->sample_fmt, 0);
    assert( dst_samples_size > 0 );
    assert( max_dst_nb_samples > 0 );

    return true;
}


/* prepare a 16 bit dummy audio frame of 'frame_size' samples and
   'nb_channels' channels */
void CMedia::get_audio_frame(uint8_t*& buf, int& frame_size,
			     const AVCodecContext* c ) const
{
    audio_cache_t::const_iterator end = _audio.end();
    audio_cache_t::const_iterator i = std::lower_bound( _audio.begin(), end, 
							_frame,
							LessThanFunctor() );
    if ( i == end ) {
       IMG_ERROR( _("Could not get audio frame ") << _frame );
       frame_size = 0;
       return;
    }

    audio_type_ptr result = *i;

    frame_size = result->size();

    src_nb_samples = frame_size;
    src_nb_samples /= c->channels;
    src_nb_samples /= av_get_bytes_per_sample( aformat );

    if ( unsigned(frame_size) > src_samples_size )
    {
       //assert( frame_size <= src_nb_samples );
       av_free(src_samples_data[0]);
       int ret = av_samples_alloc(src_samples_data, &src_samples_linesize,
				  c->channels, src_nb_samples,
				  aformat, 0);

       if ( ret < 0 )
       {
	  LOG_ERROR( "Could not allocate src buffer" );
	  return;
       }

    }


    memcpy( src_samples_data[0], result->data(), frame_size );

}



static void write_audio_frame(AVFormatContext *oc, AVStream *st,
			      const CMedia* img)
{
   AVPacket pkt = {0};
   AVFrame* frame = av_frame_alloc();
   av_frame_unref(frame);
   int got_packet, ret, dst_nb_samples;
   
   
   {
      av_init_packet(&pkt);
      pkt.size = 0;
      pkt.data = NULL;


      int size = 0;
       
      AVCodecContext* c = st->codec;
      int bytes_per_sample = av_get_bytes_per_sample(c->sample_fmt);

      assert( src_samples_data[0] != NULL );

      img->get_audio_frame(src_samples_data[0], size, c );
      if ( size == 0 ) {
	 LOG_ERROR( _("Could not get audio frame for encoding") );
	 // memset( src_samples_data[0], 0, src_nb_samples
	 // 	 * c->channels * bytes_per_sample);
      }

      if (swr_ctx) {
	 /* Compute destination number of samples */
	 dst_nb_samples = src_nb_samples;
	 if (dst_nb_samples > max_dst_nb_samples) {
   	    av_free(dst_samples_data[0]);
            ret = av_samples_alloc(dst_samples_data, &dst_samples_linesize, c->channels,
                                   dst_nb_samples, c->sample_fmt, 0);
            if (ret < 0)
   	       LOG_ERROR( _("Could not allocate dst samples") );

            max_dst_nb_samples = dst_nb_samples;
            dst_samples_size = av_samples_get_buffer_size(NULL, c->channels, 
   							  dst_nb_samples,
                                                          c->sample_fmt, 0);
	 }
	 
	 /* convert to destination format */
	 assert( dst_nb_samples == src_nb_samples );
	 ret = swr_convert(swr_ctx,
			   dst_samples_data, dst_nb_samples,
			   (const uint8_t **)src_samples_data, src_nb_samples);
	 if (ret < 0) {
            LOG_ERROR( _("Error while converting") );
	    return;
	 }

      } else {
	 dst_samples_data[0] = src_samples_data[0];
	 dst_nb_samples = src_nb_samples;
      }

      c->frame_size = dst_nb_samples;
      // c->frame_size /= c->channels;
      // c->frame_size /= av_get_bytes_per_sample( c->sample_fmt );


      frame->nb_samples     = c->frame_size;
      // frame->format         = c->sample_fmt;
      // frame->channels       = c->channels;
      // frame->channel_layout = c->channel_layout;

      assert( dst_samples_data[0] != NULL );
      assert( dst_samples_size != 0 );
      assert( dst_samples_size >= dst_nb_samples );
      assert( dst_nb_samples > 0 );
      assert( c->channels > 0 );
      assert( c->sample_fmt != 0 );
      

      int err = avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt,
					 dst_samples_data[0], 
					 dst_samples_size, 0 );

      if (err < 0)
      {
	 LOG_ERROR( _("Could not fill audio frame. Error: ") << 
		    strerror(err) );
	 return;
      }
       
       
      err = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
      if (err < 0 || !got_packet )
      {
	 LOG_ERROR( _("Could not encode audio frame") );
	 return;
      }
       
      pkt.stream_index = st->index;
      if (got_packet) {
	 if (pkt.pts != AV_NOPTS_VALUE)
	    pkt.pts = av_rescale_q(pkt.pts, c->time_base, st->time_base);
	 if (pkt.dts != AV_NOPTS_VALUE)
	    pkt.dts = av_rescale_q(pkt.dts, c->time_base, st->time_base);
	 if (pkt.duration > 0)
	    pkt.duration = (int) av_rescale_q(pkt.duration, c->time_base, 
					      st->time_base);
      }
       
      /* write the compressed frame in the media file */
      if (av_interleaved_write_frame(oc, &pkt) != 0) {
	 LOG_ERROR( _("Error while writing audio frame") );
      }
   }
    
    
   av_frame_free( &frame );

}

static void close_audio_static(AVFormatContext *oc, AVStream *st)
{
   avcodec_close(st->codec);

   // av_free(audio_outbuf);
}

bool aviImage::open_movie( const char* filename, const CMedia* img )
{

   frame_count = 0;

   av_register_all();

   if ( oc != NULL ) return false;

   std::string ext = "avi";
   std::string file = filename;
   size_t pos = file.rfind( '.' );
   if ( pos != std::string::npos )
   {
      ext = file.substr( pos, file.size() );
   }

   // Avoutputformat* outformat = av_guess_format(NULL, filename, NULL);
   // if ( outformat == NULL )
   // {
   //    LOG_ERROR("Could not guess output format from filenmae" );
   // }


   int err = avformat_alloc_output_context2(&oc, NULL, NULL, filename);
   if (!oc || err < 0) {
      LOG_INFO( _("Could not deduce output format from file extension: using MPEG.") );
      
      err = avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);
      if ( err < 0 )
      {
	 LOG_ERROR( _("Could not open mpeg movie") );
	 return false;
      }
   }

   fmt = oc->oformat;
   
   fmt->audio_codec = AV_CODEC_ID_AC3;

   video_st = NULL;
   audio_st = NULL;
   if (img->has_picture() && fmt->video_codec != CODEC_ID_NONE) {
      video_st = add_video_stream(oc, &video_codec, fmt->video_codec, img);
   }
       
   if (img->has_audio() && fmt->audio_codec != CODEC_ID_NONE) {
      audio_st = add_audio_stream(oc, &audio_cdc, fmt->audio_codec,
				  img );
   }
   
   
   /* Now that all the parameters are set, we can open the audio and
    * video codecs and allocate the necessary encode buffers. */
   if (video_st)
      if ( ! open_video(oc, video_st, img) )
	 return false;
   
   if (audio_st)
      if ( ! open_audio_static(oc, audio_cdc, audio_st, img) )
      {
	 audio_st = NULL;
	 if ( !video_st ) return false;
      }
   
   if (!(fmt->flags & AVFMT_NOFILE)) {
      if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
	 LOG_ERROR( _("Could not open '") << filename << "'" );
	 return false;
      }
   }
   
   picture->pts = 0; // AV_NOPTS_VALUE;
   
   /* Write the stream header, if any. */
   avformat_write_header(oc, NULL);


   return true;

}


bool aviImage::save_movie_frame( const CMedia* img,
				 const mrv::ViewerUI* ui )
{

   double audio_time, video_time;
   
   
   audio_time = audio_st ? audio_st->pts.val * av_q2d(audio_st->time_base) : 0.0;
   video_time = video_st ? video_st->pts.val * av_q2d(video_st->time_base) : 0.0;

   if (!audio_st && !video_st)
      return false;

   /* write interleaved audio and video frames */
   if (!video_st || (video_st && audio_st && audio_time <= video_time)) {


      while( audio_time <= video_time )
      {
	 write_audio_frame(oc, audio_st, img);
	 audio_time = audio_st->pts.val * av_q2d(audio_st->time_base);
      }
   }
   
   write_video_frame(oc, video_st, img, ui);
   picture->pts += av_rescale_q(1, video_st->codec->time_base,
				video_st->time_base);

   return true;
}


bool aviImage::close_movie()
{
   /* Write the trailer, if any. The trailer must be written before you
    * close the CodecContexts open when you wrote the header; otherwise
    * av_write_trailer() may try to use memory that was freed on
    * av_codec_close(). */
   av_write_trailer(oc);
    
    /* Close each codec. */
    if (video_st)
        close_video(oc, video_st);
    if (audio_st)
       close_audio_static(oc, audio_st);

    /* Free the streams. */
    for (unsigned i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
    if (!(fmt->flags & AVFMT_NOFILE))
       /* Close the output file. */
       avio_close(oc->pb);

    /* free the stream */
    av_free(oc);
    oc = NULL;

    return true;
}

} // namespace mrv
