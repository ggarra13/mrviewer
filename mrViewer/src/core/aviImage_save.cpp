

#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#include <inttypes.h>

#include <cstdio>
#include <cmath>

using namespace std;

#ifdef _WIN32
#define snprintf _snprintf
#define isfinite(x) _finite(x)
#endif

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "core/aviImage.h"
#include "gui/mrvIO.h"
#include "core/mrvFrameFunctors.h"
#include "core/mrvColorSpaces.h"
#include "aviSave.h"

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
static int64_t frame_count = 0, video_outbuf_size;

/* just pick the highest supported samplerate */
static int select_sample_rate(AVCodec *codec, unsigned sample_rate)
{
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if ( *p == sample_rate )
        {
            best_samplerate = *p; break;
        }
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;
}

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
    if ( pkt->pts != AV_NOPTS_VALUE )
        pkt->pts = av_rescale_q(pkt->pts, *time_base, st->time_base );
    if ( pkt->dts != AV_NOPTS_VALUE )
        pkt->dts = av_rescale_q(pkt->dts, *time_base, st->time_base );
    if ( pkt->duration > 0 )
        pkt->duration = av_rescale_q(pkt->duration, *time_base, st->time_base);
   pkt->stream_index = st->index;
   
   /* Write the compressed frame to the media file. */
#ifdef DEBUG
   log_packet(fmt_ctx, pkt);
#endif
   return av_interleaved_write_frame(fmt_ctx, pkt);
}

/* Add an output stream. */
static AVStream *add_stream(AVFormatContext *oc, AVCodec **codec,
                            enum AVCodecID codec_id, const CMedia* const img,
                            const AviSaveUI* opts)
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

    switch ((*codec)->type) {
       case AVMEDIA_TYPE_AUDIO:
          c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
          c->sample_fmt  = (*codec)->sample_fmts ?
                           (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
          if ( c->sample_fmt == AV_SAMPLE_FMT_S32P )
              c->sample_fmt = AV_SAMPLE_FMT_S16P;
          c->bit_rate    = opts->audio_bitrate;
          c->sample_rate = select_sample_rate( *codec, img->audio_frequency() );
          c->channels    = img->audio_channels();
          if ( c->channels == 2 ) c->channel_layout = AV_CH_LAYOUT_STEREO;
          c->time_base.den = 1000 * (double) img->fps();
          c->time_base.num = 1000;
          /*
          c->time_base.num = 1;
          c->time_base.den = c->sample_rate;
          */
          if (( codec_id == AV_CODEC_ID_MP3 ) || (codec_id == AV_CODEC_ID_AC3))
             c->block_align = 0;
          break;

       case AVMEDIA_TYPE_VIDEO:
           c->codec_id = codec_id;
           c->bit_rate = opts->video_bitrate;
          // c->rc_min_rate = c->bit_rate;
          // c->rc_max_rate = c->bit_rate;
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
          // c->qmin = ptr->qmin;
          // c->qmax = ptr->qmax;
          // c->me_method = ptr->me_method;
          // c->me_subpel_quality = ptr->me_subpel_quality;
          // c->i_quant_factor = ptr->i_quant_factor;
          // c->qcompress = ptr->qcompress;
          // c->max_qdiff = ptr->max_qdiff;

          if ( opts->video_color == "YUV420" )
              c->pix_fmt = AV_PIX_FMT_YUV420P;
          if ( opts->video_color == "YUV422" )
              c->pix_fmt = AV_PIX_FMT_YUV422P;
          else if ( opts->video_color == "YUV444" )
              c->pix_fmt = AV_PIX_FMT_YUV444P;
          if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
             /* just for testing, we also add B frames */
             c->max_b_frames = 2;
          }
          if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
             /* just for testing, we also add B frames */
             c->mb_decision = 2;
          }
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

bool flush = false;
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
int       samples_count = 0;

struct SwrContext *swr_ctx = NULL;

static bool open_audio_static(AVFormatContext *oc, AVCodec* codec,
			      AVStream* st, const CMedia* img,
                              const AviSaveUI* opts)

{
    AVCodecContext* c = st->codec;

    /* allocate and init a re-usable frame */
    audio_frame = av_frame_alloc();
    if (!audio_frame) {
        LOG_ERROR( "Could not allocate audio frame");
        return false;
    }


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

    int ret = av_samples_alloc_array_and_samples(&src_samples_data,
                                                 &src_samples_linesize, 
                                                 c->channels,
                                                 src_nb_samples, aformat, 0);
    if (ret < 0) {
        LOG_ERROR("Could not allocate source samples" );
        return false;
    }


    av_free( src_samples_data[0] ); // free samples data as we don't need it

    if ( aformat != c->sample_fmt )
    {

        /* set options */
       swr_ctx  = swr_alloc();
       if ( swr_ctx == NULL )
       {
	  LOG_ERROR( "Could not alloc swr_ctx" );
	  return false;
       }


        // av_opt_set_int       (swr_ctx, "in_channel_layout", 
        //                       av_get_default_channel_layout(c->channels), 0);
        av_opt_set_int       (swr_ctx, "in_channel_count",   c->channels, 0);
        av_opt_set_int       (swr_ctx, "in_sample_rate", img->audio_frequency(), 0);
        av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt",      aformat, 0);
        // av_opt_set_int       (swr_ctx, "out_channel_layout", c->channel_layout,
        //                       0);
        av_opt_set_int       (swr_ctx, "out_channel_count",  c->channels,   0);
        av_opt_set_int       (swr_ctx, "out_sample_rate",    c->sample_rate, 0);
        av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt",     c->sample_fmt, 0);
	

	LOG_INFO( "Audio conversion of " 
                  << " channels " << c->channels << ", freq "
		  << img->audio_frequency() << " " 
                  << av_get_sample_fmt_name( aformat ) 
		  << " to " << std::endl
		  << "              channels " << c->channels 
		  << " freq " << c->sample_rate << " "
		  << av_get_sample_fmt_name( c->sample_fmt ) << "." );
		   
	
        // assert( src_samples_data[0] != NULL );
        // assert( src_samples_linesize > 0 );

        max_dst_nb_samples = src_nb_samples;

        assert( src_nb_samples > 0 );

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

        /* initialize the resampling context */
        if ((swr_init(swr_ctx)) < 0) {
           LOG_ERROR( _("Failed to initialize the resampling context") );
	   return false;
        }

    }
    else
    {
       dst_samples_data = src_samples_data;
       max_dst_nb_samples = src_nb_samples;
    }

    assert( max_dst_nb_samples > 0 );
    dst_samples_size = av_samples_get_buffer_size(NULL, c->channels, 
						  max_dst_nb_samples,
                                                  c->sample_fmt, 0);
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
   pkt.size = 0;
   pkt.data = NULL;

   AVCodecContext* c = st->codec;

   const audio_type_ptr audio = img->get_audio_frame();

   src_nb_samples = audio->size();
   src_nb_samples /= img->audio_channels();
   src_nb_samples /= av_get_bytes_per_sample( aformat );

   if ( src_nb_samples == 0 ) {
       return false;
   }

   if (swr_ctx) {
      /* compute destination number of samples */
       dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, c->sample_rate) + src_nb_samples,
                                       c->sample_rate, c->sample_rate, AV_ROUND_UP);

       assert( src_nb_samples == dst_nb_samples );

       DBG( "dst_nb_samples= " << dst_nb_samples );

       // dst_nb_samples = src_nb_samples;

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
           DBG( " dst_samples_size: " << dst_samples_size );

           // std::cerr << "dst_samples_size=" << dst_samples_size
           //           << " channels=" << c->channels
           //           << " bps=" << av_get_bytes_per_sample( c->sample_fmt )
           //           << " div=" << ( dst_samples_size / c->channels / av_get_bytes_per_sample(c->sample_fmt ) )
           //           << " dst_nb_samples=" << dst_nb_samples
           //           << " src_nb_samples=" << src_nb_samples
           //           << " src_samples_size=" << audio->size()
           //           << std::endl;

           // assert( dst_samples_size / c->channels / av_get_bytes_per_sample(c->sample_fmt ) == dst_nb_samples );
       }

      assert( audio->size() / c->channels / av_get_bytes_per_sample(aformat) == src_nb_samples );

      /* convert to destination format */
      assert( src_nb_samples == dst_nb_samples );

      const uint8_t* data = audio->data();
      src_samples_data[0] = (uint8_t*)data;
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

   audio_frame->format = c->sample_fmt;
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

   DBG( "call avcodec_encode_audio2" );
   ret = avcodec_encode_audio2(c, &pkt, audio_frame, &got_packet);
   if (ret < 0)
   {
      LOG_ERROR( _("Could not encode audio frame: ") << 
                 mrv_err2str(buf, ret) );
      return false;
   }
   DBG( "Past avcodec_encode_audio2" );

   if (!got_packet) {
      return true;
   }

   

   ret = write_frame(oc, &c->time_base, st, &pkt);
   if (ret < 0) {
      LOG_ERROR( "Error while writing audio frame: " << 
                 mrv_err2str(buf, ret) );
      return false;
   }

   av_free_packet( &pkt );

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

    picture->format = pix_fmt;
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

static AVFrame *frame;

static bool open_video(AVFormatContext *oc, AVCodec* codec, AVStream *st,
		       const CMedia* img, const AviSaveUI* opts )
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

    /* If the output format is not YUV420P, then a temporary YUV420P
     * picture is needed too. It is then converted to the required
     * output format. */
    if (c->pix_fmt != AV_PIX_FMT_YUV444P && c->pix_fmt != AV_PIX_FMT_YUV420P) 
    {
        int ret = avpicture_alloc(&src_picture, AV_PIX_FMT_YUV444P, c->width, c->height);
        if (ret < 0) 
	  {
            LOG_ERROR( "Could not allocate temporary picture: " <<
                       ret );
            return false;
	  }
    }

    /* copy data and linesize picture pointers to frame */
    *((AVPicture *)picture) = dst_picture;

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
static void fill_yuv_image(AVCodecContext* c,AVFrame *pict, const CMedia* img )
{

   CMedia* m = (CMedia*) img;

   image_type_ptr hires = img->hires();

   unsigned w = img->width();
   unsigned h = img->height();

   float one_gamma = 1.0f / img->gamma();

   for ( unsigned y = 0; y < h; ++y )
   {
      for ( unsigned x = 0; x < w; ++x )
      {
	 ImagePixel p = hires->pixel( x, y );

         if ( p.r > 0.0f && isfinite(p.r) )
             p.r = powf( p.r, one_gamma );
         if ( p.g > 0.0f && isfinite(p.g) )
             p.g = powf( p.g, one_gamma );
         if ( p.b > 0.0f && isfinite(p.b) )
             p.b = powf( p.b, one_gamma );

         if      (p.r < 0.0f) p.r = 0.0f;
         else if (p.r > 1.0f) p.r = 1.0f;
         if      (p.g < 0.0f) p.g = 0.0f;
         else if (p.g > 1.0f) p.g = 1.0f;
         if      (p.b < 0.0f) p.b = 0.0f;
         else if (p.b > 1.0f) p.b = 1.0f;

	 ImagePixel yuv = color::rgb::to_ITU601( p );

	 pict->data[0][y * pict->linesize[0]   + x   ] = uint8_t(yuv.r);

         unsigned x2 = x, y2 = y;
         if ( c->pix_fmt == AV_PIX_FMT_YUV422P )
         {
             x2 /= 2;
         }
         else if ( c->pix_fmt == AV_PIX_FMT_YUV420P )
         {
             x2 /= 2;
             y2 /= 2;
         }

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

   fill_yuv_image( c, picture, img );

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

       char buf[AV_ERROR_MAX_STRING_SIZE];

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

          av_free_packet( &pkt );

          if (ret < 0) {
              LOG_ERROR( "Error while writing video frame: " << 
                         mrv_err2str(buf, ret) );
              return false;
          }
      }
   }

   frame_count++;

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
#if 1
    audio_cache_t::const_iterator end = _audio.end();
    audio_cache_t::const_iterator i = std::lower_bound( _audio.begin(), end, 
							_frame,
							LessThanFunctor() );
#else
    audio_cache_t::const_iterator end = _audio.end();
    audio_cache_t::const_iterator i = std::find_if( _audio.begin(), end,
                                                    EqualFunctor(_frame) );
#endif
    if ( i == end )
    {
        LOG_ERROR( "Missing audio frame " << _frame );
        return audio_type_ptr( new audio_type( _frame, 0, 0, NULL, 0) );
    }


    return *i;
}



bool aviImage::open_movie( const char* filename, const CMedia* img,
                           const AviSaveUI* opts )
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

   fmt = oc->oformat;
   assert( fmt != NULL );

   if ( opts->video_codec == "H264" )
       fmt->video_codec = AV_CODEC_ID_H264;
   else if ( opts->video_codec == "HEVC" )
       fmt->video_codec = AV_CODEC_ID_HEVC;
   else if ( opts->video_codec == "MPEG4" )
       fmt->video_codec = AV_CODEC_ID_MPEG4;


   if ( opts->audio_codec == "NONE" )
       fmt->audio_codec = AV_CODEC_ID_NONE;    // works but s16p
   else if ( opts->audio_codec == "MP3" )
       fmt->audio_codec = AV_CODEC_ID_MP3;    // works but s16p
   else if ( opts->audio_codec == "AC3" )
       fmt->audio_codec = AV_CODEC_ID_AC3; // works out of sync
   else if ( opts->audio_codec == "AAC" )
       fmt->audio_codec = AV_CODEC_ID_AAC; // best - crashes
   else if ( opts->audio_codec == "VORBIS" )
       fmt->audio_codec = AV_CODEC_ID_VORBIS; // best - crashes
   else if ( opts->audio_codec == "PCM" )
       fmt->audio_codec = AV_CODEC_ID_PCM_S16LE;



   video_st = NULL;
   audio_st = NULL;
   if (img->has_picture() && fmt->video_codec != AV_CODEC_ID_NONE) {
       video_st = add_stream(oc, &video_codec, fmt->video_codec, img, opts);
   }

   if (img->has_audio() && fmt->audio_codec != AV_CODEC_ID_NONE) {
      audio_st = add_stream(oc, &audio_cdc, fmt->audio_codec,
                            img, opts );
   }

   if ( video_st == NULL && audio_st == NULL )
   {
      LOG_ERROR( "No audio nor video stream created" );
      return false;
   }


   /* Now that all the parameters are set, we can open the audio and
    * video codecs and allocate the necessary encode buffers. */
   if (video_st)
       if ( ! open_video(oc, video_codec, video_st, img, opts ) )
	 return false;

   if (audio_st)
       if ( ! open_audio_static(oc, audio_cdc, audio_st, img, opts) )
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

   double STREAM_DURATION = (double) img->duration() / (double) img->fps();

    /* Compute current audio and video time. */
   audio_time = ( audio_st ? audio_st->pts.val * av_q2d( audio_st->time_base )
		  : INFINITY );
   video_time = ( video_st ? video_st->pts.val * av_q2d( video_st->time_base )
		  : INFINITY );


    

    /* write interleaved audio and video frames */

    if ( audio_st )
    {
       while( audio_time <= video_time) {
           if ( ! write_audio_frame(oc, audio_st, img) )
             break;
	   audio_time = audio_st->pts.val * av_q2d( audio_st->time_base);
       }
    }

    if ( video_st ) {

       write_video_frame(oc, video_st, img);

       picture->pts += av_rescale_q(1, video_st->codec->time_base,
                                    video_st->time_base);
    }

    
   return true;
}

bool flush_video_and_audio( const CMedia* img )
{
    int stop_encoding = 0;
    int ret = 0;

    AVStream* st[2];
    st[0] = audio_st;
    st[1] = video_st;
    for ( int i = 0; i < 2; ++i ) {
        AVStream* s = st[i];
        if ( !s ) continue;

        AVCodecContext* c = s->codec;

        if ( !( c->codec->capabilities & CODEC_CAP_DELAY ) )
            continue;

        if (c->codec_type == AVMEDIA_TYPE_AUDIO && c->frame_size <= 1)
            continue;
        if (c->codec_type == AVMEDIA_TYPE_VIDEO && (oc->oformat->flags & AVFMT_RAWPICTURE) && c->codec->id == AV_CODEC_ID_RAWVIDEO)
            continue;

        for (;;) {
            int (*encode)(AVCodecContext*, AVPacket*, const AVFrame*, int*) = NULL;
            const char *desc;
            
            switch (s->codec->codec_type) {
                case AVMEDIA_TYPE_AUDIO:
                    encode = avcodec_encode_audio2;
                    desc   = "audio";
                    break;
                case AVMEDIA_TYPE_VIDEO:
                    encode = avcodec_encode_video2;
                    desc   = "video";
                    break;
                default:
                    stop_encoding = 1;
            }

            if (encode) {
                AVPacket pkt;
                int got_packet = 0;
                av_init_packet(&pkt);
                pkt.data = NULL;
                pkt.size = 0;
                
                ret = encode(c, &pkt, NULL, &got_packet);

                if (ret < 0) {
                    LOG_ERROR( "Failed " << desc << " encoding" );
                    return false;
                }
 
                if (!got_packet ) {
                    stop_encoding = 1;
                    LOG_INFO( "Stopped encoding cached " << desc << " frames" );
                    break;
                }

                ret = write_frame(oc, &c->time_base, s, &pkt);

                if ( ret < 0 )
                {
                    LOG_ERROR( "Error writing " << desc << " frame" );
                    stop_encoding = 1;
                    break;
                }


                if (s->codec->codec_type == AVMEDIA_TYPE_VIDEO)
                    frame_count++;
            }

            if (stop_encoding)
                break;
        }
    }

    return true;
}

bool aviImage::close_movie( const CMedia* img )
{
    if ( !flush_video_and_audio(img) )
    {
        LOG_ERROR( "Flushing of buffers failed" );
    }

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
