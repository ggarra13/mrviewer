/**
 * @file   mrvVersion.cpp
 * @author gga
 * @date   Thu Nov 02 05:41:29 2006
 * 
 * @brief  Version information for viewer
 * 
 * 
 */

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

}

#include <GL/glew.h>
#include <wand/magick-wand.h>


#include <fltk/visual.h>
#include <fltk/Monitor.h>
#include <fltk/Browser.h>

#include <boost/version.hpp>

#include "core/mrvOS.h"
#include "core/mrvCPU.h"
#include "gui/mrvVersion.h"
#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
#include "core/mrvOS.h"
#include "video/mrvDrawEngine.h"
#include "mrViewer.h"



namespace mrv
{



  static const char* kVersion = "2.4.1 - Built " __DATE__ " " __TIME__;


  struct FormatInfo
  {
    bool encode;
    bool decode;
    bool blob;
    std::string name;
    std::string module;
    std::string description;


    FormatInfo(bool dec, bool enc, bool blo, 
	       const char* n, const char* mod, const char* desc ) :
      encode( enc ),
      decode( dec ),
      blob( blo ),
      name( n ),
      module( mod ),
      description( desc )
    {
    }

    bool operator<( const FormatInfo& b ) const
    {
      return ( strcasecmp( name.c_str(), b.name.c_str() ) < 0 ); 
    }

  };

  struct SortFormatsFunctor
  {
    bool operator()( const FormatInfo* a, const FormatInfo* b ) const
    {
      return *a < *b;
    }
  };


  typedef std::vector< FormatInfo* > FormatList;


  const char* version()
  {
    return kVersion;
  }

void ffmpeg_formats( fltk::Browser& browser )
  {
    using namespace std;


    AVInputFormat* ifmt = NULL;
    AVOutputFormat* ofmt = NULL;
    const char* last_name = NULL;


    FormatList formats;
    FormatInfo* f = NULL;

    f = new FormatInfo( true, true, false, "EXR", "OpenEXR",
			"ILM OpenEXR" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "IFF", "internal",
			"Autodesk Maya IFF" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "DDS", "internal",
			"Microsoft Direct Draw Surface" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "PIC", "internal",
			"Softimage pic" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "SHMAP", "internal",
			"mental images shadow maps" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "MAP", "internal",
			"mental images ripmap (non-tiled)" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "Z", "internal",
			"Pixar depth maps" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "CT", "internal",
			"mental images color files" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "NT", "internal",
			"mental images normal files" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "BT", "internal",
			"mental images bit files" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "MT", "internal",
			"mental images motion files" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "ST", "internal",
			"mental images scalar files" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "ZT", "internal",
			"mental images depth files" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "HDR", "internal",
			"Radiance HDR images" );
    formats.push_back(f);

    last_name= "000";
    for(;;){
      bool decode = false;
      bool encode = false;
      const char* name=NULL;
      const char* long_name=NULL;

      for(ofmt = av_oformat_next(NULL); ofmt; ofmt = av_oformat_next(ofmt)) {
	if((name == NULL || strcmp(ofmt->name, name)<0) &&
	   strcmp(ofmt->name, last_name)>0){
	  name = ofmt->name;
	  long_name = ofmt->long_name;
	  encode = true;
	}
      }
      for(ifmt = av_iformat_next(NULL); ifmt; ifmt = av_iformat_next(ifmt)) {
	if((name == NULL || strcmp(ifmt->name, name)<0) &&
	   strcmp(ifmt->name, last_name)>0){
	  name = ifmt->name;
	  long_name = ifmt->long_name;
	  encode = false;
	}
	if(name && strcmp(ifmt->name, name)==0)
	  decode = true;
      }
      if(name==NULL)
	break;
      last_name= name;

      f = new FormatInfo( decode, encode, false, name, "FFMPEG", long_name );
      formats.push_back( f );
    }

    // Now, add image formats from imagemagick

    ExceptionInfo* exception = AcquireExceptionInfo();
    size_t num;
    const MagickInfo** magick_info = GetMagickInfoList("*", &num, exception);
    exception=DestroyExceptionInfo(exception);

    for ( unsigned i = 0; i < num; ++i )
      {
	const MagickInfo* m = magick_info[i];

	if (m->stealth != MagickFalse)
	  continue;


	f = new FormatInfo( m->decoder, m->encoder, 
			    m->blob_support != MagickFalse, 
			    m->name, "ImageMagick", m->description );
	formats.push_back( f );
      }
    magick_info = (const MagickInfo **)
      RelinquishMagickMemory((void *) magick_info);


    // Sort formats alphabetically
    std::sort( formats.begin(), formats.end(), SortFormatsFunctor() );

    // Now concatenate all the stuff into a string
    {
      FormatList::const_iterator i = formats.begin();
      FormatList::const_iterator e = formats.end();
      for ( ; i != e; ++i )
	{
	  f = *i;
	  std::ostringstream o;
	  o << ( f->decode ? "D\t" : " \t" ) 
	    << ( f->encode ? "E\t" : " \t" ) 
	    << ( f->blob   ? "B\t" : " \t" ) 
	    << f->name << "\t"
	    << f->module << "\t"
	    << f->description;
	  browser.add( o.str().c_str() );
	  delete f;
	}
    }

  }

static void ffmpeg_codecs(fltk::Browser& browser, int type)
  {
    using namespace std;

    AVCodec *p, *p2;
    const char* last_name;

    std::ostringstream o;
    last_name= "000";
    for(;;){
      int decode=0;
      int encode=0;
      int cap=0;
      const char *type_str;

      p2=NULL;

      for(p = av_codec_next(NULL); p; p = av_codec_next(p) ) {
	if((p2==NULL || strcmp(p->name, p2->name)<0) &&
	   strcmp(p->name, last_name)>0){
	  p2= p;
	  decode= encode= cap=0;
	}
	if(p2 && strcmp(p->name, p2->name)==0){
	  if(p->decode) decode=1;
	  if(p->encode2) encode=1;
	  cap |= p->capabilities;
	}
      }

      if(p2==NULL)
	break;
      last_name= p2->name;

      if ( p2->type != type )
	continue;

      switch(p2->type) {
      case AVMEDIA_TYPE_VIDEO:
	type_str = "V";
	break;
      case AVMEDIA_TYPE_AUDIO:
	type_str = "A";
	break;
      case AVMEDIA_TYPE_SUBTITLE:
	type_str = "S";
	break;
      default:
	type_str = "?";
	break;
      }

      std::ostringstream o;
      o << ( decode ? "D\t" : " \t" )
	<< ( encode ? "E\t" : " \t" )
	<< "\t"
	<< ( cap & CODEC_CAP_DRAW_HORIZ_BAND ? "S\t":" \t" )
	<< ( cap & CODEC_CAP_DR1 ? "D\t":" \t" )
	<< ( cap & CODEC_CAP_TRUNCATED ? "T\t":" \t" )
	<< p2->name;

      browser.add( o.str().c_str() );

      /* if(p2->decoder && decode==0)
	 printf(" use %s for decoding", p2->decoder->name);*/
    }

  }


  void ffmpeg_audio_codecs(fltk::Browser& browser )
  {
     return ffmpeg_codecs( browser, AVMEDIA_TYPE_AUDIO );
  }

  void ffmpeg_video_codecs(fltk::Browser& browser )
  {
     return ffmpeg_codecs( browser, AVMEDIA_TYPE_VIDEO );
  }

  void ffmpeg_subtitle_codecs(fltk::Browser& browser )
  {
     return ffmpeg_codecs( browser, AVMEDIA_TYPE_SUBTITLE );
  }


  std::string ffmpeg_protocols()
  {
    std::ostringstream o;
#if LIBAVUTIL_VERSION_MAJOR > 50
    void* opaque = NULL;
    const char* up;
    for( up = avio_enum_protocols( &opaque, 0 ); up; 
	 up = avio_enum_protocols( &opaque, 0 ) )
      {
	o << " " << up << ":";
      }
#else
    URLProtocol* up;
    for(up = av_protocol_next(NULL); up; up = av_protocol_next(up) )
     {
    	o << " " << up->name << ":";
     }
#endif
    return o.str();
  }

  std::string ffmpeg_motion_estimation_methods()
  {
    static const char *motion_str[] = {
      "zero",
      "full",
      "log",
      "phods",
      "epzs",
      "x1",
      "hex",
      "umh",
      "iter",
      NULL,
    };

    using namespace std;
    std::ostringstream o;

    const char** pp = motion_str;
    while (*pp) {
      o << *pp << "\t";
      unsigned idx = (pp - motion_str + 1);
      switch(idx)
	{
	case ME_ZERO:
	  o << "(fastest)"; break;
	case ME_FULL:
	  o << "(slowest)"; break;
	case ME_EPZS:
	  o << "(default)"; break;
	case ME_HEX:
	case ME_UMH:
	  o << "[x264 codec]"; break;
	case ME_ITER:
	  o << "[snow codec]"; break;
	default:
	  break;
	}
      o << endl;
      pp++;
    }

    return o.str();
  }


  //
  // Redirects ffmpeg's av_log messages to mrViewer's log window.
  //
  void av_log_redirect( void* ptr, int level, const char* fmt, va_list vl )
  {
    static const char* kModule = "ffmpeg";
    char buf[1024];  buf[1023] = 0;
    vsnprintf( buf, 1023, fmt, vl );
    
    if ( level < AV_LOG_WARNING )
      mrvLOG_ERROR( kModule, buf );
    else if ( level < AV_LOG_INFO )
      mrvLOG_WARNING( kModule, buf );
    else if ( level < AV_LOG_VERBOSE )
      mrvLOG_INFO( kModule, buf );
    else {
      // log verbose
    }
  }


  std::string about_message()
  {
    using namespace std;


// #ifdef DEBUG
//     av_log_set_level(99);
// #else
    av_log_set_level(-1);
// #endif

    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    av_log_set_callback( mrv::av_log_redirect );


    avcodec_register_all();

    av_register_all();
    avformat_network_init();



    size_t magic = 0;
    std::ostringstream o;

    unsigned int boost_major = BOOST_VERSION / 100000;
    unsigned int boost_minor = BOOST_VERSION / 100 % 1000;
    unsigned int boost_teeny = BOOST_VERSION % 100;

    o << "mrViewer - v" << kVersion << endl
      << "(C) 2007 Film Aura, LLC." << endl
      << endl
      << "mrViewer depends on:" << endl
      << endl
      << "Boost v" << boost_major << "." 
      << boost_minor << "." << boost_teeny << endl
      << "http://www.boost.org/" << endl
      << endl
      << "FLTK 2.0" << endl
      << "http://www.fltk.org/" << endl
      << "(C) 2000-2006 Bill Spitzak & others" << endl
      << endl
      << "GLEW " << glewGetString( GLEW_VERSION ) << endl
      << "http://glew.sourceforge.net/" << endl
      << "(C) SGI, Lev Povalahev, Milan Ikits" << endl
      << endl
      << MagickGetVersion(&magic) << endl
      << MagickGetCopyright() << endl
      << endl
      << "libavutil\tv" << AV_STRINGIFY( LIBAVUTIL_VERSION ) << endl
      << "libavcodec\tv" << AV_STRINGIFY( LIBAVCODEC_VERSION ) << endl
      << "libavformat\tv" << AV_STRINGIFY( LIBAVFORMAT_VERSION ) << endl
      << "http://ffmpeg.mplayerhq.hu/" << endl
      << "(C) 2000-2006 Fabrice Bellard, et al." << endl
      << endl
      << "ILM OpenEXR v1.7 or later" << endl
      << "http://www.openexr.org/" << endl
      << "(C) 2005-2007 Industrial Light & Magic" << endl
      << endl
      << "AMPAS CTL v1.2 or later" << endl
      << "http://www.openexr.org/" << endl
      << "(C) 2007 AMPAS and Industrial Light & Magic" << endl
      << endl
#ifdef GPL
      << "libdvdread" << endl
      << "libdvdnav" << endl
      << "libdvdcss" << endl
      << endl
#endif
      << "IccProfLib (ICC v4.2)" << endl
      << "(C) 2003-2006 - The International Color Consortium" << endl
      << endl
      << "PostgreSQL" << endl
      << "http://www.postgresql.org" << endl
      << "(C) 1996-2005, The PostgreSQL Global Development Group" << endl
      << "(C) 1994 The Regents of the University of California" << endl
      << endl
      << "InstallJammer" << endl
      << "http://www.installjammer.com" << endl
      << "(C) 2005-2007, Damon Courtney" << endl
      << endl;

    return o.str();
  }


  std::string cpu_information()
  {
    return GetCpuCaps(&gCpuCaps);
  }


  std::string gpu_information( mrv::ViewerUI* uiMain )
  {
    using std::endl;
    std::ostringstream o;

    const fltk::Monitor* monitors;
    int num_monitors = fltk::Monitor::list(&monitors);
    o << "Monitors:\t" << num_monitors << endl;

    mrv::DrawEngine* engine = uiMain->uiView->engine();
    if ( engine )
      {
	o << uiMain->uiView->engine()->options();
      }
    else
      {
	o << "No GPU." << endl;
      }

    o << "HW Stereo:\t" 
      << ( uiMain->uiView->can_do( fltk::STEREO ) ? "Yes" : "No" )
      << endl
      << "HW Overlay:\t" 
      << ( uiMain->uiView->can_do_overlay() ? "Yes" : "No" )
      << endl; 

    return o.str();
  }


}
