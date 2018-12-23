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
#include <libavutil/avassert.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "core/mrvColorOps.h"
#include "core/aviImage.h"
#include "core/mrvMath.h"
#include "core/mrvSwizzleAudio.h"
#include "core/mrvFrameFunctors.h"
#include "core/mrvColorSpaces.h"

#include "gui/mrvPreferences.h"
#include "gui/mrvIO.h"

#include "aviSave.h"


namespace {
const char* kModule = "save";
}

#define MAKE_TAG(a,b,c,d) (unsigned int) ( (unsigned int)(a)	\
                                           + (unsigned int)(b << 8)	\
                                           + (unsigned int)(c << 16)	\
                                           + (unsigned int)(d <<24 ))

#define LOG(x) std::cerr << x << std::endl;

// #undef DBG
// #define DBG(x) std::cerr << x << std::endl;


namespace mrv {


static AVFrame *picture = NULL;
static int64_t frame_count = 0;

int encode(AVCodecContext *avctx, AVPacket *pkt, AVFrame *frame,
           int *got_packet)
{
    int ret;

    *got_packet = 0;

    ret = avcodec_send_frame(avctx, frame);
    if (ret < 0 && ret != AVERROR_EOF )
        return ret;

    ret = avcodec_receive_packet(avctx, pkt);
    if (!ret)
        *got_packet = 1;
    if (ret == AVERROR(EAGAIN))
        return 0;

    return ret;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(AVCodec *codec, unsigned sample_rate)
{
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return sample_rate;

    p = codec->supported_samplerates;
    while (*p) {
        if ( *p == sample_rate )
            return *p;
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;
}

static AVSampleFormat select_sample_format( AVCodec* c, AVSampleFormat input )
{
    AVSampleFormat r;
    if ( input == AV_SAMPLE_FMT_FLT )
        r = AV_SAMPLE_FMT_FLTP;
    else if ( input == AV_SAMPLE_FMT_S16 )
        r = AV_SAMPLE_FMT_S16P;
    else if ( input == AV_SAMPLE_FMT_U8 )
        r = AV_SAMPLE_FMT_U8P;
    else if ( input == AV_SAMPLE_FMT_S32 )
        r = AV_SAMPLE_FMT_S32P;
    else if ( input == AV_SAMPLE_FMT_DBL )
        r = AV_SAMPLE_FMT_DBLP;

    const AVSampleFormat* p = c->sample_fmts;
    while (*p != -1) {
        if ( *p == r )
            return r;
        ++p;
    }

    return AV_SAMPLE_FMT_FLTP;
}

/* select layout with the highest channel count */
static uint64_t select_channel_layout(const AVCodec *codec,
                                      unsigned num_channels)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        unsigned nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels == num_channels) {
            best_ch_layout   = *p;
            break;
        }
        p++;
    }
    return best_ch_layout;
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
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;


    /* Write the compressed frame to the media file. */
#ifdef DEBUG_PACKET
    log_packet(fmt_ctx, pkt);
#endif
    return av_interleaved_write_frame(fmt_ctx, pkt);
}


/**************************************************************/
/* audio output */

AVSampleFormat aformat;
AVFrame* audio_frame;
static int64_t frame_audio = 0;
static AVAudioFifo* fifo = NULL;
static uint8_t **src_samples_data = NULL;
static int       src_samples_linesize;
static int       src_nb_samples;

static int max_dst_nb_samples;
uint8_t **dst_samples_data = NULL;
int       dst_samples_linesize;
boost::uint64_t samples_count = 0;

struct SwrContext *swr_ctx = NULL;
struct SwsContext* sws_ctx = NULL;


/* Add an output stream. */
static AVStream *add_stream(AVFormatContext *oc, AVCodec **codec,
                            const char* name, // codec name
                            enum AVCodecID codec_id, const CMedia* const img,
                            const AviSaveUI* opts)
{
    AVCodecContext *c;
    AVStream *st;

    const AVCodecDescriptor *desc;
    *codec = avcodec_find_encoder_by_name( name );
    if ( !*codec )
    {
        desc = avcodec_descriptor_get_by_name( name );
        if ( desc )
        {
            codec_id = desc->id;
            *codec = avcodec_find_encoder(codec_id);
        }
    }

    /* find the encoder */
    if (!(*codec)) {
        LOG_ERROR( _("Could not find encoder for '") << name << "'" );
        return NULL;
    }


    LOG_INFO( _("Open encoder ") << (*codec)->name );

    st = avformat_new_stream(oc, *codec);
    if (!st) {
        LOG_ERROR( _("Could not allocate stream") );
        return NULL;
    }
    st->id = oc->nb_streams-1;
    c = st->codec;

    /* Some formats want stream headers to be separate. */
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    switch ((*codec)->type) {
       case AVMEDIA_TYPE_AUDIO:
          c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
          aformat = AudioEngine::ffmpeg_format( img->audio_format() );
          if ( opts->audio_codec == "pcm_s16le" )
              c->sample_fmt = AV_SAMPLE_FMT_S16;
          else
              c->sample_fmt = select_sample_format(*codec, aformat );
          c->bit_rate    = opts->audio_bitrate;
          c->sample_rate = select_sample_rate( *codec, img->audio_frequency() );
          c->channels    = img->audio_channels();
          c->time_base.num = st->time_base.num = 1;
          c->time_base.den = st->time_base.den = c->sample_rate;

          if((c->block_align == 1 || c->block_align == 1152 ||
              c->block_align == 576) && c->codec_id == AV_CODEC_ID_MP3)
              c->block_align = 0;
          if(c->codec_id == AV_CODEC_ID_AC3)
              c->block_align = 0;
          break;

       case AVMEDIA_TYPE_VIDEO:
           {
               c->codec_id = codec_id;
               c->bit_rate = opts->video_bitrate;
               // c->rc_min_rate = c->bit_rate;
               // c->rc_max_rate = c->bit_rate;
               /* Resolution must be a multiple of two. */
               c->width    = (( img->width() + 1 ) / 2) * 2;
               c->height   = (( img->height() + 1 ) / 2) * 2;
               /* timebase: This is the fundamental unit of time (in seconds) in terms
                * of which frame timestamps are represented. For fixed-fps content,
                * timebase should be 1/framerate and timestamp increments should be
                * identical to 1. */
               if ( opts->fps <= 0 )
                   c->time_base.den = st->time_base.den = int( 1000.0 * img->fps() );
               else
                   c->time_base.den = st->time_base.den = int( 1000.0 * opts->fps );
               c->time_base.num = st->time_base.num = 1000;
               c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
               // c->qmin = ptr->qmin;
               // c->qmax = ptr->qmax;
               // c->me_method = ptr->me_method;
               // c->me_subpel_quality = ptr->me_subpel_quality;
               // c->i_quant_factor = ptr->i_quant_factor;
               // c->qcompress = ptr->qcompress;
               // c->max_qdiff = ptr->max_qdiff;

               // Use a profile if possible
               c->profile = opts->video_profile;
               c->colorspace = (AVColorSpace) opts->yuv_hint;

               if ( opts->video_color == "YUV420" )
                   c->pix_fmt = AV_PIX_FMT_YUV420P;
               else if ( opts->video_color == "YUV422" )
                   c->pix_fmt = AV_PIX_FMT_YUV422P;
               else if ( opts->video_color == "YUV444" )
                   c->pix_fmt = AV_PIX_FMT_YUV444P;
               else if ( opts->video_color == "GBRP10LE"  )
                   c->pix_fmt = AV_PIX_FMT_GBRP10LE;

               if ( c->codec_id == AV_CODEC_ID_HEVC )
               {
                   c->codec_tag = MAKE_TAG('H', 'V', 'C', '1');

                   switch( opts->video_profile )
                   {
                       case 0:
                           c->profile = FF_PROFILE_HEVC_MAIN;
                           break;
                       case 1:
                           c->profile = FF_PROFILE_HEVC_MAIN_10;
                           break;
                       case 2:
                       default:
                           c->profile = FF_PROFILE_HEVC_REXT;
                           break;
                   }
                   break;
               }
               else if ( c->codec_id == AV_CODEC_ID_H264 )
                 {

                   switch( opts->video_profile )
                     {
                     case 0:
                       c->profile = FF_PROFILE_H264_BASELINE;break;
                     case 1:
                       c->profile = FF_PROFILE_H264_CONSTRAINED_BASELINE;break;
                     case 2:
                       c->profile = FF_PROFILE_H264_MAIN; break;
                     case 3:
                       c->profile = FF_PROFILE_H264_EXTENDED; break;
                     case 5:
                       c->profile = FF_PROFILE_H264_HIGH_10; break;
                     case 4:
                     default:
                       c->profile = FF_PROFILE_H264_HIGH;
                       break;
                     }
                 }
               else if ( c->codec_id == AV_CODEC_ID_FFV1 )
               {
                   break;
               }
               else if ( c->codec_id == AV_CODEC_ID_MPEG4 )
               {
                   switch( opts->video_profile )
                   {
                       case 0:
                           c->profile = FF_PROFILE_MPEG4_SIMPLE; break;
                       case 1:
                           c->profile = FF_PROFILE_MPEG4_CORE; break;
                       case 2:
                           c->profile = FF_PROFILE_MPEG4_MAIN; break;
                       case 3:
                       default:
                           c->profile = FF_PROFILE_MPEG4_HYBRID; break;
                       case 4:
                           c->profile = FF_PROFILE_MPEG4_ADVANCED_CORE; break;
                   }
               }
               else if ( c->codec_id == AV_CODEC_ID_PNG ||
                         c->codec_id == AV_CODEC_ID_TIFF )
               {
                   c->pix_fmt = AV_PIX_FMT_BGR32;
               }
               else if ( c->codec_id == AV_CODEC_ID_MJPEG )
               {
                   c->pix_fmt = AV_PIX_FMT_YUVJ444P;
               }
               else if ( c->codec_id == AV_CODEC_ID_PRORES )
               {
                   // ProRes supports YUV422 10bit
                   // (and YUV444P10 YUVA444P10) pixel formats.
                   if ( opts->video_color == "YUV422" )
                       c->pix_fmt = AV_PIX_FMT_YUV422P10;
                   else if ( opts->video_color == "YUV444" )
                   {
                       c->pix_fmt = AV_PIX_FMT_YUV444P10;
                       if ( img->has_alpha() )
                       {
                           c->pix_fmt = AV_PIX_FMT_YUVA444P10LE;
                           c->profile = 4;
                       }
                   }
                   // Profiles are taken from opts->video_profile
               }
               else
               {
                   // Select a pixel format based on user option.
                   if ( opts->video_color == "YUV420" )
                       c->pix_fmt = AV_PIX_FMT_YUV420P;
                   else if ( opts->video_color == "YUV422" )
                       c->pix_fmt = AV_PIX_FMT_YUV422P;
                   else if ( opts->video_color == "YUV444" )
                       c->pix_fmt = AV_PIX_FMT_YUV444P;
                   else
                       LOG_ERROR( _("Unknown c->pix_fmt (")
                                  << opts->video_color <<
                                  _(") for movie file") );
               }
               if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                   /* just for testing, we also add B frames */
                   c->max_b_frames = 2;
               }
               if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                   c->mb_decision = 2;
               }

               const char* name = avcodec_profile_name( codec_id, c->profile );
               if (name) LOG_INFO( _("Profile name ") << name );
               break;
           }
    default:
        break;
    }


    return st;
}


static bool open_sound(AVFormatContext *oc, AVCodec* codec,
                       AVStream* st, const CMedia* img,
                       const AviSaveUI* opts)

{
    AVCodecContext* c = st->codec;

    /* allocate and init a re-usable frame */
    audio_frame = av_frame_alloc();
    if (!audio_frame) {
        LOG_ERROR( _("Could not allocate audio frame") );
        return false;
    }

    c->channel_layout = select_channel_layout( codec, c->channels );

   /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
       LOG_ERROR( _("Could not open audio codec" ) );
       return false;
    }

    int ret = avcodec_parameters_from_context(st->codecpar, c);
    if ( ret < 0 )
    {
        LOG_ERROR( _("Could not copy context to parameters") );
        return false;
    }

    if ( opts->metadata )
    {
        const CMedia::Attributes& attrs = img->attributes();
        CMedia::Attributes::const_iterator i = attrs.begin();
        CMedia::Attributes::const_iterator e = attrs.end();

        //
        // Save Audio Attributes
        //
        for ( ; i != e; ++i )
        {
            if ( i->first.find( _("Audio ") ) == 0 )
            {
                std::string      key = i->first;
                Imf::Attribute*  attr = i->second;
                std::string      val  = CMedia::attr2str( attr );
                key = key.substr( 6, key.size() );
                av_dict_set( &st->metadata, key.c_str(), val.c_str(), 0 );
            }
        }
    }

    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
    {
        c->frame_size = 10000;
    }

    src_nb_samples = c->frame_size;

    audio_frame->nb_samples     = c->frame_size;
    audio_frame->channels       = c->channels;
    audio_frame->channel_layout = c->channel_layout;
    audio_frame->format         = c->sample_fmt;
    audio_frame->sample_rate    = c->sample_rate;

    /**
     * Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified.
     */
    if ((ret = av_frame_get_buffer(audio_frame, 0)) < 0) {
        LOG_ERROR( _("Could not allocate output frame samples. Error: ")
                   << get_error_text( ret ) );
        av_frame_free(&audio_frame);
        return false;
    }

    ret = av_samples_alloc_array_and_samples(&src_samples_data,
                                             &src_samples_linesize,
                                             c->channels,
                                             src_nb_samples, aformat, 1);
    if (ret < 0) {
        LOG_ERROR( _("Could not allocate source samples. Error: ")
                   << get_error_text( ret ) );
        av_frame_free(&audio_frame);
        return false;
    }


    av_free( src_samples_data[0] ); // free samples data as we don't need it

    if ( aformat != c->sample_fmt )
    {

        /* set options */
       swr_ctx  = swr_alloc();
       if ( swr_ctx == NULL )
       {
           LOG_ERROR( _("Could not alloc swr_ctx") );
           av_frame_free(&audio_frame);
           return false;
       }

       uint64_t in_layout = av_get_default_channel_layout(c->channels);
       av_opt_set_int       (swr_ctx, N_("in_channel_layout"),
                             in_layout, 0);
       av_opt_set_int       (swr_ctx, N_("in_channel_count"),
                             c->channels, 0);
       av_opt_set_int       (swr_ctx, N_("in_sample_rate"),
                             img->audio_frequency(), 0);
       av_opt_set_sample_fmt(swr_ctx, N_("in_sample_fmt"),      aformat, 0);
       av_opt_set_int       (swr_ctx, N_("out_channel_layout"),
                             c->channel_layout, 0);
       av_opt_set_int       (swr_ctx, N_("out_channel_count"), c->channels, 0);
       av_opt_set_int       (swr_ctx, N_("out_sample_rate"), c->sample_rate,
                             0);
       av_opt_set_sample_fmt(swr_ctx, N_("out_sample_fmt"), c->sample_fmt, 0);

       char buf[256], buf2[256];
       av_get_channel_layout_string( buf, 256, c->channels, in_layout );
       av_get_channel_layout_string( buf2, 256, c->channels, c->channel_layout);

       LOG_INFO( _( "Audio conversion of " )
                 << buf
                 << _(" channels ") << c->channels << _(", freq ")
                 << img->audio_frequency() << N_(" ")
                 << av_get_sample_fmt_name( aformat )
                 << _(" to ") << std::endl
                 << N_("              ")
                 << buf2
                 << _(" channels ") << c->channels
                 << _(", freq ") << c->sample_rate << N_(" ")
                 << av_get_sample_fmt_name( c->sample_fmt ) << N_(".") );

        // assert( src_samples_data[0] != NULL );
        // assert( src_samples_linesize > 0 );

        max_dst_nb_samples = src_nb_samples;

        av_assert0( src_nb_samples > 0 );

        int ret = av_samples_alloc_array_and_samples(&dst_samples_data,
                                                     &dst_samples_linesize,
                                                     c->channels,
                                                     max_dst_nb_samples,
                                                     c->sample_fmt, 1);
        if ( ret < 0 )
        {
           LOG_ERROR( _("Could not allocate destination samples") );
           av_frame_free(&audio_frame);
           return false;
        }

        /* initialize the resampling context */
        if ((swr_init(swr_ctx)) < 0) {
           LOG_ERROR( _("Failed to initialize the resampling context") );
           av_frame_free(&audio_frame);
           return false;
        }

    }
    else
    {
       dst_samples_data = src_samples_data;
       av_assert0( src_nb_samples > 0 );
       max_dst_nb_samples = src_nb_samples;
    }

    av_assert0( max_dst_nb_samples > 0 );

    return true;
}


static bool write_audio_frame(AVFormatContext *oc, AVStream *st,
                              CMedia* img)
{
   AVPacket pkt = {0};
   int got_packet, ret, dst_nb_samples;


   av_init_packet(&pkt);

   AVCodecContext* c = st->codec;

   int tries;
   audio_type_ptr audio = img->get_audio_frame( frame_audio );

   if ( !audio ) {
       LOG_ERROR( _("audio frame is missing") );
       return false;
   }

   ++frame_audio;

   if ( audio->frame() == AV_NOPTS_VALUE ) {
       LOG_ERROR( "audio frame has NOPTS value" );
       return false;
   }

   src_nb_samples = audio->size();
   src_nb_samples /= img->audio_channels();
   src_nb_samples /= av_get_bytes_per_sample( aformat );

   if ( src_nb_samples == 0 ) {
       LOG_ERROR( _( "src_nb_samples is 0") );
       return false;
   }


   if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
   {
       LOG_INFO( _("Codec has variable frame size.  Setting it to ")
                 << src_nb_samples );
       av_assert0( src_nb_samples > 0 );
       c->frame_size = src_nb_samples;
   }


   if ( !fifo )
   {
       fifo = av_audio_fifo_alloc(c->sample_fmt, c->channels, 1);
       if ( !fifo )
       {
           LOG_ERROR( _("Could not allocate fifo buffer") );
           return false;
       }
   }

   unsigned frame_size = c->frame_size;

   const uint8_t* data = audio->data();
   src_samples_data[0] = (uint8_t*)data;

   if (swr_ctx) {
      /* compute destination number of samples */
       unsigned src_rate = img->audio_frequency();
       unsigned dst_rate = c->sample_rate;

       dst_nb_samples = static_cast<unsigned>(
                                              av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) +
                                                             src_nb_samples, dst_rate, src_rate,
                                                             AV_ROUND_UP) );


       if (dst_nb_samples > max_dst_nb_samples) {

           av_free(dst_samples_data[0]);

           ret = av_samples_alloc(dst_samples_data, &dst_samples_linesize,
                                  c->channels, dst_nb_samples,
                                  c->sample_fmt, 1);
           if (ret < 0)
           {
               LOG_ERROR( _("Cannot allocate dst samples") );
               return false;
           }

           max_dst_nb_samples = dst_nb_samples;
       }

      assert( audio->size() / c->channels / av_get_bytes_per_sample(aformat) == src_nb_samples );

      /* convert to destination format */

      ret = swr_convert(swr_ctx,
                        dst_samples_data, dst_nb_samples,
                        (const uint8_t **)src_samples_data,
                        src_nb_samples);
      if (ret < 0) {
         LOG_ERROR( _("Error while converting audio: ")
                    << get_error_text(ret) );
         return false;
      }

      if ( c->channels >= 6 )
      {
          if ( c->sample_fmt == AV_SAMPLE_FMT_FLTP )
          {
              SwizzlePlanar<float> t( (void**)dst_samples_data );
              t.do_it();
          }
          else if ( c->sample_fmt == AV_SAMPLE_FMT_S32P )
          {
              SwizzlePlanar<int32_t> t( (void**)dst_samples_data );
              t.do_it();
          }
          else if ( c->sample_fmt == AV_SAMPLE_FMT_S16P )
          {
              SwizzlePlanar<int16_t> t( (void**)dst_samples_data );
              t.do_it();
          }
      }


      ret = av_audio_fifo_write(fifo, (void**)dst_samples_data, dst_nb_samples);
      if ( ret != dst_nb_samples )
      {
          if ( ret < 0 )
          {
              LOG_ERROR( _("Could not write to fifo buffer. Error: ")
                         << get_error_text(ret) );
          }
          LOG_WARNING( _( "Did not write all dst samples to fifo. Error: ")
                       << get_error_text( ret ) );
          return false;
      }

   } else {
      dst_nb_samples = src_nb_samples;

      ret = av_audio_fifo_write(fifo, (void**)src_samples_data, src_nb_samples);
      if ( ret != dst_nb_samples )
      {
          if ( ret < 0 )
          {
              LOG_ERROR( _("Could not write to fifo buffer. Error:")
                         << get_error_text(ret) );
          }
          LOG_WARNING( _( "Did not write all src samples to fifo. Error: ")
                       << get_error_text( ret ) );
          return false;
      }
   }

   audio_frame->pts = AV_NOPTS_VALUE;
   audio_frame->nb_samples     = frame_size;
   audio_frame->channels       = c->channels;
   audio_frame->channel_layout = c->channel_layout;
   audio_frame->format         = c->sample_fmt;
   audio_frame->sample_rate    = c->sample_rate;

   AVRational ratio = { 1, c->sample_rate };

   DBG( "frame_size= " << frame_size << "  audio->size()= " << audio->size() );


   while ( av_audio_fifo_size( fifo ) >= (int)frame_size )
   {

       ret = av_audio_fifo_read(fifo, (void**)audio_frame->extended_data,
                                frame_size);
       if ( ret < 0 )
       {
           LOG_ERROR( _("Could not read samples from fifo buffer.  Error: ")
                      << get_error_text(ret) );
           return false;
       }


       audio_frame->pts = av_rescale_q( samples_count, ratio,
                                        c->time_base );

       ret = encode(c, &pkt, audio_frame, &got_packet);
       if (ret < 0)
       {
           LOG_ERROR( _("Could not encode audio frame: ") <<
                      get_error_text(ret) );
	   av_packet_unref( &pkt );
           return false;
       }


       if (!got_packet) {
           // Empty packet - do not save
           continue;
       }


       samples_count += frame_size;


       ret = write_frame(oc, &c->time_base, st, &pkt);
       if (ret < 0) {
           LOG_ERROR( "Error while writing audio frame: " <<
                      get_error_text(ret) );
	   av_packet_unref( &pkt );
           return false;
       }

   }

   // if ( av_audio_fifo_size(fifo) % frame_size != 0 )
   // {
   //     ret = av_audio_fifo_read(fifo, (void**)audio_frame->extended_data,
   //                              frame_size);
   //     if ( ret < 0 )
   //     {
   // 	   LOG_ERROR( _("Could not read samples from fifo buffer.  Error: ")
   //                    << get_error_text(ret) );
   // 	   av_packet_unref( &pkt );
   // 	   return false;
   //     }

   //     memset( audio_frame->extended_data[0], 0,
   // 	       frame_size - av_audio_fifo_size(fifo) * c->channels *
   // 	       av_get_bytes_per_sample(aformat) );
       

   //     audio_frame->pts = av_rescale_q( samples_count, ratio,
   //                                      c->time_base );

   //     ret = encode(c, &pkt, audio_frame, &got_packet);
   //     if (ret < 0 || !got_packet )
   //     {
   //         LOG_ERROR( _("Could not encode audio frame: ") <<
   //                    get_error_text(ret) );
   // 	   av_packet_unref( &pkt );
   //         return false;
   //     }

   //     samples_count += frame_size;

   //     ret = write_frame(oc, &c->time_base, st, &pkt);
   //     if (ret < 0) {
   //         LOG_ERROR( "Error while writing audio frame: " <<
   //                    get_error_text(ret) );
   // 	   av_packet_unref( &pkt );
   //         return false;
   //     }

   // }


   av_packet_unref( &pkt );

   return true;

}




static void close_audio_static(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);

    av_audio_fifo_free( fifo ); fifo = NULL;
    swr_free( &swr_ctx );

    if (dst_samples_data != src_samples_data) {
        av_free(dst_samples_data[0]);
        av_free(dst_samples_data);
    }
    src_samples_data = NULL;
    src_samples_linesize = 0;
    src_nb_samples = 0;
    max_dst_nb_samples = 0;
    dst_samples_data = NULL;
    dst_samples_linesize = 0;
    av_frame_free(&audio_frame);
}

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    int ret;

    picture = av_frame_alloc();
    if (!picture)
        return NULL;

    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(picture->data, picture->linesize, width, height,
                         pix_fmt, 32);
    if ( ret < 0 )
    {
        LOG_ERROR( _("Could not allocate raw picture buffer") );
        return NULL;
    }

    return picture;
}

AVPixelFormat ffmpeg_pixel_format( const mrv::image_type::Format& f,
                                   const mrv::image_type::PixelType& p )
{
    switch( f )
    {
        case mrv::image_type::kITU_601_YCbCr410A:
        case mrv::image_type::kITU_709_YCbCr410A:
        case mrv::image_type::kITU_601_YCbCr410:
        case mrv::image_type::kITU_709_YCbCr410:
            return AV_PIX_FMT_YUV410P;
        case mrv::image_type::kITU_601_YCbCr420:
        case mrv::image_type::kITU_709_YCbCr420:
            return AV_PIX_FMT_YUV420P;
        case mrv::image_type::kITU_601_YCbCr420A: // @todo: not done
        case mrv::image_type::kITU_709_YCbCr420A: // @todo: not done
            return AV_PIX_FMT_YUVA420P;
        case mrv::image_type::kITU_601_YCbCr422:
        case mrv::image_type::kITU_709_YCbCr422:
            return AV_PIX_FMT_YUV422P;
        case mrv::image_type::kITU_601_YCbCr422A: // @todo: not done
        case mrv::image_type::kITU_709_YCbCr422A: // @todo: not done
            return AV_PIX_FMT_YUVA422P;
        case mrv::image_type::kITU_601_YCbCr444:
        case mrv::image_type::kITU_709_YCbCr444:
            return AV_PIX_FMT_YUV444P;
        case mrv::image_type::kITU_601_YCbCr444A: // @todo: not done
        case mrv::image_type::kITU_709_YCbCr444A: // @todo: not done
            return AV_PIX_FMT_YUVA444P;
        case mrv::image_type::kLummaA:
            return AV_PIX_FMT_GRAY8A;
        case mrv::image_type::kLumma:
            if ( p == mrv::image_type::kShort )
                return AV_PIX_FMT_GRAY16LE;
            return AV_PIX_FMT_GRAY8;
        case mrv::image_type::kRGB:
            if ( p == mrv::image_type::kShort )
                return AV_PIX_FMT_RGB48;
            return AV_PIX_FMT_RGB24;
        case mrv::image_type::kRGBA:
            if ( p == mrv::image_type::kShort )
                return AV_PIX_FMT_RGBA64;
            return AV_PIX_FMT_RGBA;
        case mrv::image_type::kBGR:
            if ( p == mrv::image_type::kShort )
                return AV_PIX_FMT_BGR48;
            return AV_PIX_FMT_BGR24;
        case mrv::image_type::kBGRA:
            if ( p == mrv::image_type::kShort )
                return AV_PIX_FMT_BGRA64;
            return AV_PIX_FMT_BGRA;
        default:
            return AV_PIX_FMT_NONE;
    }
}


static AVFrame *frame;

static bool open_video(AVFormatContext *oc, AVCodec* codec, AVStream *st,
                       const CMedia* img, const AviSaveUI* opts )
{
    AVCodecContext* c = st->codec;

    AVDictionary* info = NULL;

    if ( opts->metadata )
        av_dict_set( &info, "movflags", "+use_metadata_tags", 0 );

    /* open the codec */
    if (avcodec_open2(c, codec, &info) < 0) {
       LOG_ERROR( _("Could not open video codec") );
       return false;
    }

    int ret = avcodec_parameters_from_context(st->codecpar, c);
    if ( ret < 0 )
    {
        LOG_ERROR( _("Could not copy context to parameters") );
        return false;
    }

    if ( opts->metadata )
    {
        const CMedia::Attributes& attrs = img->attributes();
        CMedia::Attributes::const_iterator i = attrs.begin();
        CMedia::Attributes::const_iterator e = attrs.end();

        //
        // Save Main Attributes
        //
        for ( ; i != e; ++i )
        {
            if (( i->first.find( _("Video ") ) == 0 ) ||
                ( i->first.find( _("Audio ") ) == 0 ) ) {
                continue;
            }

            std::string key = i->first;
            Imf::Attribute*  attr = i->second;
            std::string      val  = CMedia::attr2str( attr );
            av_dict_set( &oc->metadata, key.c_str(), val.c_str(), 0 );
        }

        //
        // Save Video Attributes
        //
        for ( i = attrs.begin(); i != e; ++i )
        {
            if ( i->first.find( _("Video ") ) == 0 )
            {
                std::string key = i->first;
                Imf::Attribute*  attr = i->second;
                std::string      val  = CMedia::attr2str( attr );
                key = key.substr( 6, key.size() );
                av_dict_set( &st->metadata, key.c_str(), val.c_str(), 0 );
            }
        }

    }

    /* Allocate the encoded raw frame. */
    picture = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!picture) {
       LOG_ERROR( _("Could not allocate picture") );
       return false;
    }

    picture->pts = 0;

    return true;
}

static void close_video(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
    av_frame_free(&picture);
}


/* prepare a yuv image */
static void fill_yuv_image(AVCodecContext* c,AVFrame *pict, const CMedia* img)
{

   CMedia* m = (CMedia*) img;

   image_type_ptr hires = img->hires();

   if ( !hires )
   {
       LOG_ERROR( "Missing picture" );
       return;
   }


   unsigned w = c->width;
   unsigned h = c->height;

   if ( hires->width() < w )  w = hires->width();
   if ( hires->height() < h ) h = hires->height();

   float one_gamma = 1.0f / img->gamma();

   const std::string& display = mrv::Preferences::OCIO_Display;
   const std::string& view = mrv::Preferences::OCIO_View;

   mrv::image_type_ptr ptr = hires;  // lut based image
   mrv::image_type_ptr sho = hires;  // 16-bits image
   VideoFrame::Format format = image_type::kRGB;
   if ( hires->channels() == 4 ) format = image_type::kRGBA;

   if ( Preferences::use_ocio && !display.empty() && !view.empty() &&
        Preferences::uiMain->uiView->use_lut() )
   {

       ptr = mrv::image_type_ptr( new image_type( hires->frame(),
                                                 w, h,
                                                 hires->channels(),
                                                 format,
                                                 mrv::image_type::kFloat ) );
       copy_image( ptr, hires );
       bake_ocio( ptr, img );
   }

   sho = mrv::image_type_ptr( new image_type( hires->frame(),
                                              w, h,
                                              hires->channels(),
                                              format,
                                              mrv::image_type::kByte ) );
   if ( mrv::is_equal( one_gamma, 1.0f ) )
   {
       for ( unsigned y = 0; y < h; ++y )
       {
           for ( unsigned x = 0; x < w; ++x )
           {
               ImagePixel p = ptr->pixel( x, y );
               p.clamp();
               sho->pixel( x, y, p );
           }
       }
   }
   else
   {
       // Gamma correct image
       for ( unsigned y = 0; y < h; ++y )
       {
           for ( unsigned x = 0; x < w; ++x )
           {
               ImagePixel p = ptr->pixel( x, y );

               // This code is equivalent to p.r = powf( p.r, gamma )
               // but faster
               if ( p.r > 0.0f && isfinite(p.r) )
                   p.r = expf( logf(p.r) * one_gamma );
               if ( p.g > 0.0f && isfinite(p.g) )
                   p.g = expf( logf(p.g) * one_gamma );
               if ( p.b > 0.0f && isfinite(p.b) )
                   p.b = expf( logf(p.b) * one_gamma );

               p.clamp();

               sho->pixel(x, y, p );
           }
       }
   }
   hires = sho;


   AVPixelFormat fmt = ffmpeg_pixel_format( hires->format(),
                                            hires->pixel_type() );
   sws_ctx = sws_getCachedContext( sws_ctx, w, h,
                                   fmt, c->width, c->height, c->pix_fmt, 0,
                                   NULL, NULL, NULL );
   if ( !sws_ctx )
   {
       LOG_ERROR( _("Failed to initialize swscale conversion context") );
       return;
   }

   uint8_t* buf = (uint8_t*)hires->data().get();
   uint8_t* data[4] = {NULL, NULL, NULL, NULL};
   int linesize[4] = { 0, 0, 0, 0 };
   av_image_fill_arrays( data, linesize, buf, fmt, w, h, 1 );
   sws_scale( sws_ctx, data, linesize, 0, h, picture->data, picture->linesize );

}

static bool write_video_frame(AVFormatContext* oc, AVStream* st,
                              const CMedia* img )
{
   int ret;
   AVCodecContext* c = st->codec;

   fill_yuv_image( c, picture, img );

   AVPacket pkt = { 0 };
   av_init_packet(&pkt);

   int got_packet = 0;

   /* encode the image */
   picture->pts = frame_count;
   ret = encode(c, &pkt, picture, &got_packet);
   if (ret < 0) {
       LOG_ERROR( _("Error while encoding video frame: ") <<
                  get_error_text(ret) );
       return false;
   }

   /* If size is zero, it means the image was buffered. */
   if ( got_packet )
   {
       ret = write_frame( oc, &c->time_base, st, &pkt );

       av_packet_unref( &pkt );

       if (ret < 0) {
           LOG_ERROR( _("Error while writing video frame: ") <<
                      get_error_text(ret) );
           return false;
       }

   }

   frame_count++;

   return true;
}


static AVFormatContext *oc = NULL;
static AVOutputFormat* fmt = NULL;
static AVStream* audio_st = NULL, *video_st = NULL;

AVCodec* audio_cdc, *video_codec;




static inline
int64_t get_valid_channel_layout(int64_t channel_layout, int channels)
{
    if (channel_layout && av_get_channel_layout_nb_channels(channel_layout) == channels)
        return channel_layout;
    else
        return 0;
}



/* prepare an audio frame of 'frame_size' samples and
   'nb_channels' channels */
audio_type_ptr CMedia::get_audio_frame(const int64_t f )
{
    int64_t x = f + audio_offset() - 1;

    if ( x < first_frame() )
    {
        unsigned size = audio_bytes_per_frame();
        uint8_t* data = new uint8_t[ size ];
        memset( data, 0, size );
        return audio_type_ptr( new audio_type( x, audio_frequency(),
                                               audio_channels(), data, size) );
    }

    audio_cache_t::iterator end = _audio.end();
    audio_cache_t::iterator i = end;
#if 0  // less correct
    {
        i = std::lower_bound( _audio.begin(), end, x, LessThanFunctor() );
        if ( i != end ) {
            return *i;
        }
    }
#else
    {
        i = std::find_if( _audio.begin(), end, EqualFunctor(x) );
        if ( i != end ) return *i;
    }
#endif

    IMG_ERROR( _("Missing audio frame ") << x );
    return audio_type_ptr( new audio_type( AV_NOPTS_VALUE, audio_frequency(),
                                           audio_channels(), NULL, 0) );
}



bool aviImage::open_movie( const char* filename, const CMedia* img,
                           AviSaveUI* opts )
{
   assert( filename != NULL );
   assert( img != NULL );

   frame_audio = img->frame() + img->audio_offset();

   samples_count = 0;
   frame_count = 0;

   //avcodec_register_all(); // called by av_register_all()
   //av_register_all(); // called in mrVersion.cpp

   if ( strcmp( img->filename(), filename ) == 0 )
   {
       mrvALERT( _("You are saving over the movie you are playing. "
                   "Aborting...") );
       return false;
   }

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
       LOG_WARNING( _("Could not deduce output format from file extension: using MPEG.") );

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

   if ( opts->video_codec == _("None") )
   {
       fmt->video_codec = AV_CODEC_ID_NONE;
   }

   if ( opts->video_codec == "png" )
       fmt->video_codec = AV_CODEC_ID_PNG;
   else if ( opts->video_codec == "tiff" )
       fmt->video_codec = AV_CODEC_ID_TIFF;
   else if ( opts->video_codec == "hevc" )
       fmt->video_codec = AV_CODEC_ID_HEVC;
   else if ( opts->video_codec == "h264" )
       fmt->video_codec = AV_CODEC_ID_H264;
   else if ( opts->video_codec == "mpeg4" )
       fmt->video_codec = AV_CODEC_ID_MPEG4;
   else if ( opts->video_codec == "prores_ks" )
       fmt->video_codec = AV_CODEC_ID_PRORES;
   else if ( opts->video_codec == "ffv1" )
       fmt->video_codec = AV_CODEC_ID_FFV1;
   else if ( fmt->video_codec == AV_CODEC_ID_NONE )
   {
       // empty on purpose
   }
   else
   {
       AVCodec* c = avcodec_find_encoder( fmt->video_codec );
       opts->video_codec = c->name;
       LOG_INFO( "Codec " << c->name << " " << c->long_name << " "
                 << fmt->video_codec << " selected"  );
   }

   if ( opts->audio_codec == _("None") )
       fmt->audio_codec = AV_CODEC_ID_NONE;
   else if ( opts->audio_codec == "mp3" )
       fmt->audio_codec = AV_CODEC_ID_MP3;
   else if ( opts->audio_codec == "ac3" )
       fmt->audio_codec = AV_CODEC_ID_AC3;
   else if ( opts->audio_codec == "aac" )
       fmt->audio_codec = AV_CODEC_ID_AAC;
   else if ( opts->audio_codec == "vorbis" )
       fmt->audio_codec = AV_CODEC_ID_VORBIS;
   else if ( opts->audio_codec == "pcm" )
       fmt->audio_codec = AV_CODEC_ID_PCM_S16LE;


   video_st = NULL;
   audio_st = NULL;
   if (img->has_picture() && fmt->video_codec != AV_CODEC_ID_NONE) {
       video_st = add_stream(oc, &video_codec, opts->video_codec.c_str(),
                             fmt->video_codec, img, opts);
   }

   if (img->has_audio() && fmt->audio_codec != AV_CODEC_ID_NONE) {
       audio_st = add_stream(oc, &audio_cdc, opts->audio_codec.c_str(),
                             fmt->audio_codec, img, opts );
   }

   if ( video_st == NULL && audio_st == NULL )
   {
       LOG_ERROR( _("No audio nor video stream created") );
       return false;
   }


   /* Now that all the parameters are set, we can open the audio and
    * video codecs and allocate the necessary encode buffers. */
   if (video_st)
     {
       if ( ! open_video(oc, video_codec, video_st, img, opts ) )
         return false;
     }

   if (audio_st)
       if ( ! open_sound(oc, audio_cdc, audio_st, img, opts) )
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

    AVDictionary* info = NULL;

    av_dict_set( &info, "movflags", "+use_metadata_tags", 0 );

   /* Write the stream header, if any. */
   err = avformat_write_header(oc, &info);
   if ( err < 0 )
   {
      LOG_ERROR( _("Error occurred when opening output file: ") <<
                 get_error_text(err) );
      return false;
   }

   CMedia* image = const_cast<CMedia*>(img);
   image->playback( CMedia::kSaving );

   return true;

}

bool write_va_frame( CMedia* img )
{

   double audio_time, video_time;

    /* Compute current audio and video time. */
   audio_time = ( audio_st ? ( double(audio_frame->pts) *
                               av_q2d( audio_st->time_base ) )
                  : INFINITY );
   // This is wrong as it does not get updated properly with h264
   //video_time = ( video_st ? video_st->pts.val * av_q2d( video_st->time_base )
   //             : INFINITY );

   video_time = ( video_st ? ( double(picture->pts) *
                               av_q2d( video_st->codec->time_base ) )
                  : INFINITY );


   // std::cerr << "VIDEO TIME " << video_time << " " << picture->pts
   //           << " " << video_st->time_base.num
   //           << "/" << video_st->time_base.den
   //           << " c: " << video_st->codec->time_base.num
   //           << "/" << video_st->codec->time_base.den << std::endl;


    /* write interleaved audio and video frames */


    if ( video_st ) {
        write_video_frame(oc, video_st, img);

       // av_rescale_q(1, video_st->codec->time_base,
       // video_st->time_base);
    }

    if ( audio_st )
    {
        while( audio_time <= video_time ) {
            if ( ! write_audio_frame(oc, audio_st, img) )
                break;
            audio_time = (double)audio_frame->pts *
                         av_q2d( audio_st->time_base);
       }
    }

   return true;
}

bool aviImage::save_movie_frame( CMedia* img )
{


   if (!audio_st && !video_st)
      return false;

   return write_va_frame( img );
}


bool flush_video_and_audio( const CMedia* img )
{
    int ret = 0;

    if ( audio_st && fifo )
    {
        AVCodecContext* c = audio_st->codec;

        unsigned cache_size = av_audio_fifo_size( fifo );

        AVRational ratio = { 1, c->sample_rate };

        int got_packet = 0;
        AVPacket pkt = { 0 };
        av_init_packet(&pkt);

        // Send last packet to encode
        if ( cache_size > 0 )
        {
            while( got_packet && ret >= 0 )
            {
                ret = av_audio_fifo_read(fifo,
                                         (void**)audio_frame->extended_data,
                                         cache_size);
                if (ret < 0)
                {
                    LOG_ERROR( _("Could not read audio fifo: ") <<
                               get_error_text(ret) );
                }

                c->frame_size = cache_size;
                audio_frame->nb_samples = cache_size;
                audio_frame->pts = av_rescale_q( samples_count, ratio,
                                                 c->time_base );

                ret = encode(c, &pkt, audio_frame, &got_packet);
                if (ret < 0)
                {
                    LOG_ERROR( _("Could not encode audio frame: ") <<
                               get_error_text(ret) );
                }

                samples_count += cache_size;
            }
        }

        av_packet_unref( &pkt );

    }

    AVStream* st[2];
    st[0] = audio_st;
    st[1] = video_st;
    for ( int i = 0; i < 2; ++i ) {

        AVStream* s = st[i];
        if ( !s ) continue;


        int encoding = 1;
        int stop_encoding = 0;
        ret = 0;
        AVCodecContext* c = s->codec;

        if ( !( c->codec->capabilities & AV_CODEC_CAP_DELAY ) )
            continue;

        if (c->codec_type == AVMEDIA_TYPE_AUDIO && c->frame_size <= 1)
            continue;
        // if (c->codec_type == AVMEDIA_TYPE_VIDEO && (oc->oformat->flags & AVFMT_RAWPICTURE) && c->codecpar->id == AV_CODEC_ID_RAWVIDEO)
        //     continue;

        while( ret >= 0 ) {

            const char* desc;
            switch (s->codec->codec_type) {
                case AVMEDIA_TYPE_AUDIO:
                    desc   = "audio";
                    break;
                case AVMEDIA_TYPE_VIDEO:
                    desc   = "video";
                    break;
                default:
                    stop_encoding = 1;
            }

            if (encoding) {
                AVPacket pkt = {0};
                int got_packet = 0;
                av_init_packet(&pkt);

                ret = encode(c, &pkt, NULL, &got_packet);

                if (ret < 0 && ret != AVERROR_EOF) {
                    LOG_ERROR( _("Failed ") << desc << _(" encoding") );
                    return false;
                }

                if (!got_packet ) {
                    stop_encoding = 1;
                    LOG_INFO( _("Stopped encoding cached ") << desc
                              << _(" frames") );
                    break;
                }

                ret = write_frame(oc, &c->time_base, s, &pkt);

                av_packet_unref( &pkt );

                if ( ret < 0 )
                {
                    LOG_ERROR( _("Error writing ") << desc << _(" frame") );
                    stop_encoding = 1;
                    break;
                }

            }

            if ( stop_encoding )
                break;
        }
    }

    return true;
}

bool aviImage::close_movie( const CMedia* img )
{
    if ( !flush_video_and_audio(img) )
    {
        LOG_ERROR( _("Flushing of buffers failed") );
    }

    if ( sws_ctx )
    {
        sws_freeContext( sws_ctx );
        sws_ctx = NULL;
    }

    CMedia* image = const_cast<CMedia*>(img);
    image->playback( CMedia::kStopped );

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

    LOG_INFO( _("Closed movie file") );

    return true;
}

} // namespace mrv
