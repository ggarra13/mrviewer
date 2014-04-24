

#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>

#include <cstdio>

#ifdef _WIN32
#define snprintf _snprintf
#endif

extern "C" {
#include <libavutil/audioconvert.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavutil/pixdesc.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

#include "core/aviImage.h"
#include "gui/mrvIO.h"
#include "core/mrvFrameFunctors.h"
#include "core/mrvColorSpaces.h"

#define mrv_err2str(buf, errnum) \
    av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, errnum)

namespace {
const char* kModule = "save";
}

#define IMG_ERROR(x) LOG_ERROR( name() << " - " << x )
#define IMG_WARNING(x) LOG_WARNING( name() << " - " << x )
#define LOG(x) std::cerr << x << std::endl;

namespace mrv {


int video_is_eof = 0;
int audio_is_eof = 0;
static AVFrame *picture = NULL;
static AVPicture src_picture, dst_picture;
static int frame_count = 0, video_outbuf_size;

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    char buf[AV_TS_MAX_STRING_SIZE];
    char buf2[AV_TS_MAX_STRING_SIZE];

    printf("pts:%s pts_time:%s ",
           av_ts_make_string(buf, pkt->pts), 
           av_ts_make_time_string(buf2, pkt->pts, time_base) );

    printf( "dts:%s dts_time:%s ",
            av_ts_make_string(buf, pkt->dts), 
            av_ts_make_time_string(buf2, pkt->dts, time_base) );

    printf( "duration:%s duration_time:%s stream_index:%d\n",
            av_ts_make_string(buf, pkt->duration), 
            av_ts_make_time_string(buf2, pkt->duration, time_base),
            pkt->stream_index);
}

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
   pkt->pts = av_rescale_q(pkt->pts, *time_base, st->time_base );
   pkt->dts = av_rescale_q(pkt->dts, *time_base, st->time_base );
   pkt->duration = av_rescale_q(pkt->duration, *time_base, st->time_base);
   pkt->stream_index = st->index;
   
   /* Write the compressed frame to the media file. */
   // log_packet(fmt_ctx, pkt);
   return av_interleaved_write_frame(fmt_ctx, pkt);
}

/* Add an output stream. */
static AVStream *add_stream(AVFormatContext *oc, AVCodec **codec,
                            enum AVCodecID codec_id, const CMedia* const img )
{
    AVCodecContext *c;
    AVStream *st;

    /* find the encoder */
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
       LOG_ERROR( "Could not find encoder for '" << 
                  avcodec_get_name(codec_id) << "'" );
       return NULL;
    }


    LOG_INFO( "Open encoder " << avcodec_get_name(codec_id) );

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        fprintf(stderr, "Could not allocate stream\n");
        return NULL;
    }
    st->id = oc->nb_streams-1;
    c = st->codec;
    c->codec_id = codec_id;

    switch ((*codec)->type) {
       case AVMEDIA_TYPE_AUDIO:
          c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
          c->sample_fmt  = (*codec)->sample_fmts ?
                           (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
          c->bit_rate    = 64000;
          c->sample_rate = img->audio_frequency();
          c->channels    = img->audio_channels();
          if ( c->channels == 2 ) c->channel_layout = AV_CH_LAYOUT_STEREO;
          c->time_base.num = 1;
          c->time_base.den = c->sample_rate;
          if (( codec_id == AV_CODEC_ID_MP3 ) || (codec_id == AV_CODEC_ID_AC3))
             c->block_align = 0;
          break;

       case AVMEDIA_TYPE_VIDEO:

          c->bit_rate = img->width() * img->height() * 3;
          /* Resolution must be a multiple of two. */
          c->width    = ( img->width() + 1 ) / 2 * 2;
          c->height   = ( img->height() + 1 ) / 2 * 2;
          /* timebase: This is the fundamental unit of time (in seconds) in terms
           * of which frame timestamps are represented. For fixed-fps content,
           * timebase should be 1/framerate and timestamp increments should be
           * identical to 1. */
          //c->time_base.den = img->fps();
          //c->time_base.num = 1;
          c->time_base.den = 1000 * (double) img->fps();
          c->time_base.num = 1000;
          c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
          c->pix_fmt       = AV_PIX_FMT_YUV420P;
          c->global_quality = 1;
          c->field_order = AV_FIELD_PROGRESSIVE;
          c->compression_level = FF_COMPRESSION_DEFAULT;
          c->me_method     = 5;
          c->prediction_method = FF_PRED_MEDIAN;
          if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
             /* just for testing, we also add B frames */
             c->ticks_per_frame = 2;
             c->max_b_frames = 2;
          }
          // if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
          //    /* Needed to avoid using macroblocks in which some coeffs overflow.
          //     * This does not happen with normal video, it just happens here as
          //     * the motion of the chroma plane does not match the luma plane. */
          //    c->mb_decision = 2;
          // }
          break;
          
    default:
        break;
    }

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;

    return st;
}

/**************************************************************/
/* audio output */

AVSampleFormat aformat;
AVFrame* audio_frame;
static uint8_t **src_samples_data = NULL;
static int       src_samples_linesize;
static int       src_nb_samples;
static unsigned  src_samples_size;

static int max_dst_nb_samples;
uint8_t **dst_samples_data = NULL;
int       dst_samples_linesize;
int       dst_samples_size;
int       samples_count;

struct SwrContext *swr_ctx = NULL;

static bool open_audio_static(AVFormatContext *oc, AVCodec* codec,
			      AVStream* st, const CMedia* img )

{
    AVCodecContext* c = st->codec;

   av_log_set_level(av_log_get_level()+10);

    DBG( __LINE__ );

    /* allocate and init a re-usable frame */
    audio_frame = av_frame_alloc();
    if (!audio_frame) {
        LOG_ERROR( "Could not allocate audio frame");
        return false;
    }


    DBG( __LINE__ );
   /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
       LOG_ERROR( _("Could not open audio codec" ) );
       return false;
    }
    DBG( __LINE__ );
    
    if (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
    {
       src_nb_samples = 10000;
    }
    else
    {
        src_nb_samples = c->frame_size;
    }


    aformat = AudioEngine::ffmpeg_format( img->audio_format() );

    if ( aformat != c->sample_fmt )
    {

        /* set options */
       swr_ctx  = swr_alloc();
       if ( swr_ctx == NULL )
       {
	  LOG_ERROR( "Could not alloc swr_ctx" );
	  return false;
       }


        av_opt_set_int       (swr_ctx, "in_channel_layout", 
                              av_get_default_channel_layout(c->channels), 0);
        av_opt_set_int       (swr_ctx, "in_channel_count",   c->channels, 0);
        av_opt_set_int       (swr_ctx, "in_sample_rate",     c->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt",      aformat, 0);
        av_opt_set_int       (swr_ctx, "out_channel_layout", c->channel_layout,
                              0);
        av_opt_set_int       (swr_ctx, "out_channel_count",  c->channels,   0);
        av_opt_set_int       (swr_ctx, "out_sample_rate",    c->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt",     c->sample_fmt, 0);
	

	LOG_INFO( "Audio conversion of channels " << c->channels << ", freq "
		  << c->sample_rate << " " << av_get_sample_fmt_name( aformat ) 
		  << " to " << std::endl
		  << "              channels " << c->channels 
		  << " freq " << c->sample_rate << " "
		  << av_get_sample_fmt_name( c->sample_fmt ) << "." );
		   
	
        // assert( src_samples_data[0] != NULL );
        // assert( src_samples_linesize > 0 );

        max_dst_nb_samples = src_nb_samples;

        assert( src_nb_samples > 0 );

    DBG( __LINE__ );
        int ret = av_samples_alloc_array_and_samples(&dst_samples_data, 
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
    DBG( __LINE__ );
        /* initialize the resampling context */
        if ((swr_init(swr_ctx)) < 0) {
           LOG_ERROR( _("Failed to initialize the resampling context") );
	   return false;
        }

    }
    else
    {
    DBG( __LINE__ );
       dst_samples_data = src_samples_data;
       max_dst_nb_samples = src_nb_samples;
    DBG( __LINE__ );
    }

    assert( max_dst_nb_samples > 0 );
    DBG( __LINE__ );
    dst_samples_size = av_samples_get_buffer_size(NULL, c->channels, 
						  max_dst_nb_samples,
                                                  c->sample_fmt, 0);
    DBG( __LINE__ );
                                                  // c->sample_fmt, 0);
    assert( dst_samples_size > 0 );
    assert( max_dst_nb_samples > 0 );

    return true;
}


static bool write_audio_frame(AVFormatContext *oc, AVStream *st, 
			      const CMedia* img)
{
   AVPacket pkt = {0};
   int got_packet, ret, dst_nb_samples;
   
   char buf[AV_ERROR_MAX_STRING_SIZE];

   av_init_packet(&pkt);

   AVCodecContext* c = st->codec;
 

   const audio_type_ptr& audio = img->get_audio_frame();
   if ( src_nb_samples == 0 ) {
      return false;
   }

   if (swr_ctx) {
      /* compute destination number of samples */
      /*
      dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, c->sample_rate) + src_nb_samples,
                                      c->sample_rate, c->sample_rate, AV_ROUND_UP);
      */

      dst_nb_samples = src_nb_samples;

      if (dst_nb_samples > max_dst_nb_samples) {
         av_free(dst_samples_data[0]);
         ret = av_samples_alloc(dst_samples_data, &dst_samples_linesize,
                                c->channels, dst_nb_samples, 
                                c->sample_fmt, 0);
         if (ret < 0)
         {
            LOG_ERROR( "Cannot allocate dst samples" );
            return false;
         }

         max_dst_nb_samples = dst_nb_samples;
         dst_samples_size = av_samples_get_buffer_size(NULL, 
                                                       c->channels,
                                                       dst_nb_samples,
                                                       c->sample_fmt, 
                                                       0);
         assert( dst_samples_size / c->channels / av_get_bytes_per_sample(c->sample_fmt ) == dst_nb_samples );
      }

            
      /* convert to destination format */
      const uint8_t* data = audio->data();
      src_samples_data = (uint8_t**)&data;
      ret = swr_convert(swr_ctx,
                        dst_samples_data, dst_nb_samples,
                        (const uint8_t **)src_samples_data, 
                        src_nb_samples);
      if (ret < 0) {
         LOG_ERROR( _("Error while converting audio: ") 
                    << mrv_err2str(buf, ret) );
         return false;
      }
            
   } else {
      dst_nb_samples = src_nb_samples;
   }
         
   audio_frame->nb_samples = dst_nb_samples;
   AVRational ratio = { 1, c->sample_rate };
   audio_frame->pts = av_rescale_q( samples_count, ratio,
                                    c->time_base );
   c->frame_size = dst_nb_samples;

   assert( dst_samples_data[0] != NULL );
   assert( dst_samples_size != 0 );
   assert( dst_samples_size >= dst_nb_samples );
   assert( dst_nb_samples > 0 );
   assert( c->channels > 0 );
   assert( c->sample_fmt != 0 );

   ret = avcodec_fill_audio_frame(audio_frame, c->channels, 
                                  c->sample_fmt,
                                  dst_samples_data[0], 
                                  dst_samples_size, 0 );
   samples_count += dst_nb_samples;

   if (ret < 0)
   {
      LOG_ERROR( _("Could not fill audio frame. Error: ") << 
                 mrv_err2str(buf, ret) );
      return false;
   }

   ret = avcodec_encode_audio2(c, &pkt, audio_frame, &got_packet);
   if (ret < 0)
   {
      LOG_ERROR( _("Could not encode audio frame: ") << 
                 mrv_err2str(buf, ret) );
      return false;
   }

   if (!got_packet) {
      LOG_ERROR( "Did not get audio packet" );
      return false;
   }
      
   ret = write_frame(oc, &c->time_base, st, &pkt);
   if (ret < 0) {
      LOG_ERROR( "Error while writing audio frame: " << 
                 mrv_err2str(buf, ret) );
      return false;
   }
    

   return true;
   
}




static void close_audio_static(AVFormatContext *oc, AVStream *st)
{
   avcodec_close(st->codec);
    if (dst_samples_data != src_samples_data) {
        av_free(dst_samples_data[0]);
        av_free(dst_samples_data);
    }
    av_frame_free(&audio_frame);
}

static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
    uint8_t *frame_buf;
    int size, ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->width = width;
    picture->height = height;

    /* Allocate the encoded raw picture. */
    ret = avpicture_alloc(&dst_picture, pix_fmt, width, height);
    if (ret < 0) {
       LOG_ERROR( "Could not allocate picture: " << ret );
       exit(1);
    }

    /* copy data and linesize picture pointers to frame */
    *((AVPicture *)picture) = dst_picture;

    return picture;
}

static bool open_video(AVFormatContext *oc, AVCodec* codec, AVStream *st,
		       const CMedia* img )
{
    AVCodecContext* c = st->codec;

    /* open the codec */
    if (avcodec_open2(c, codec, NULL) < 0) {
       LOG_ERROR( _("Could not open video codec") );
       return false;
    }

    
    /* Allocate the encoded raw frame. */
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
    av_free(src_picture.data[0]);
    av_free(dst_picture.data[0]);
    av_frame_free(&picture);
}

/* prepare a yuv image */
static void fill_yuv_image(AVFrame *pict, const CMedia* img )
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
			      const CMedia* img )
{
   int ret;
   AVCodecContext* c = st->codec;

   
#if 1
   if (frame_count >= img->duration() ) {
      /* No more frames to compress. The codec has a latency of a few
       * frames if using B-frames, so we get the last frames by
       * passing the same frame again. */
   } 
   else 
#endif
   {
      fill_yuv_image( picture, img );
   } 

   if (oc->oformat->flags & AVFMT_RAWPICTURE ) {
      /* Raw video case - directly store the frame in the packet */
      AVPacket pkt;
      av_init_packet(&pkt);

      pkt.flags        |= AV_PKT_FLAG_KEY;
      pkt.stream_index  = st->index;
        pkt.data          = dst_picture.data[0];
        pkt.size          = sizeof(AVPicture);

      ret = av_interleaved_write_frame(oc, &pkt);
   } else {

      AVPacket pkt = { 0 };
      av_init_packet(&pkt);

      int got_packet = 0;

      /* encode the image */
      picture->pts = frame_count;
      ret = avcodec_encode_video2(c, &pkt, picture, &got_packet);
      if (ret < 0) {
         LOG_ERROR( _("Error while encoding video frame: ") << ret );
         return false;
      }
       
      /* If size is zero, it means the image was buffered. */
      if ( got_packet )
      {
         ret = write_frame( oc, &c->time_base, st, &pkt );
      }
      else
      {
         ret = 0;
      }

      if (ret < 0) {
         LOG_ERROR( _("Error while writing video frame: ") << ret );
         return false;
      }

      frame_count++;
   }
   

   return true;
}


static AVFormatContext *oc = NULL;
static AVOutputFormat* fmt = NULL;
static AVStream* audio_st = NULL, *video_st = NULL;

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



/* prepare a 16 bit dummy audio frame of 'frame_size' samples and
   'nb_channels' channels */
audio_type_ptr CMedia::get_audio_frame() const
{
    audio_cache_t::const_iterator end = _audio.end();
    audio_cache_t::const_iterator i = std::lower_bound( _audio.begin(), end, 
							_frame,
							LessThanFunctor() );
    if ( i == end ) {
       IMG_ERROR( _("Could not get audio frame ") << _frame );
       src_nb_samples = 0;
       return *i;
    }

    src_nb_samples = (*i)->size();
    src_nb_samples /= audio_channels();
    src_nb_samples /= av_get_bytes_per_sample( aformat );

    return *i;
}



bool aviImage::open_movie( const char* filename, const CMedia* img )
{
   assert( filename != NULL );
   assert( img != NULL );

   samples_count = 0;
   frame_count = 0;

   avcodec_register_all();
   av_register_all();

   oc = NULL;

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

   oc->flags |= AVFMT_FLAG_NOBUFFER|AVFMT_FLAG_FLUSH_PACKETS;
   oc->max_interleave_delta = 1;
   oc->debug = FF_FDEBUG_TS;

   fmt = oc->oformat;
   fmt->video_codec = AV_CODEC_ID_MPEG4;

   assert( fmt != NULL );


   video_st = NULL;
   audio_st = NULL;
   if (img->has_picture() && fmt->video_codec != AV_CODEC_ID_NONE) {
      video_st = add_stream(oc, &video_codec, fmt->video_codec, img);
   }

   if (img->has_audio() && fmt->audio_codec != AV_CODEC_ID_NONE) {
      audio_st = add_stream(oc, &audio_cdc, fmt->audio_codec,
                            img );
   }

   if ( video_st == NULL && audio_st == NULL )
   {
      LOG_ERROR( "No audio nor video stream created" );
      return false;
   }


   /* Now that all the parameters are set, we can open the audio and
    * video codecs and allocate the necessary encode buffers. */
   if (video_st)
      if ( ! open_video(oc, video_codec, video_st, img) )
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
   
   /* Write the stream header, if any. */
   err = avformat_write_header(oc, NULL);
   if ( err < 0 )
   {
      LOG_ERROR( _("Error occurred when opening output file: ") << err );
      return false;
   }

   return true;

}


bool aviImage::save_movie_frame( const CMedia* img )
{

   double audio_time, video_time;

   if (!audio_st && !video_st)
      return false;

   // double STREAM_DURATION = (double) img->duration() / (double) img->fps();

    /* Compute current audio and video time. */
    audio_time = audio_st ? audio_st->pts.val * av_q2d(audio_st->time_base) : INFINITY;
    video_time = video_st ? video_st->pts.val * av_q2d(video_st->time_base) : INFINITY;

    // if (!flush &&
    //     (!audio_st || audio_time >= STREAM_DURATION) &&
    //     (!video_st || video_time >= STREAM_DURATION)) {
    //    LOG_WARNING( ">>>>>>>>>>>>  FLUSH = ! " );
    //    flush = 1;
    // }

    

    /* write interleaved audio and video frames */
    if ( video_st ) {

       write_video_frame(oc, video_st, img);

       picture->pts += av_rescale_q(1, video_st->codec->time_base,
                                    video_st->time_base);
    }


    if ( audio_st )
    {

       while( audio_time <= video_time) {
          if ( ! write_audio_frame(oc, audio_st, img) )
             break;
          audio_time = audio_st->pts.val * av_q2d(audio_st->time_base);
       }

    }
    
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

    if (!(fmt->flags & AVFMT_NOFILE))
       /* Close the output file. */
       avio_close(oc->pb);

    /* free the stream */
    avformat_free_context(oc);

    return true;
}

} // namespace mrv
