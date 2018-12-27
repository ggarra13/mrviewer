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
 * @file   mrvVersion.cpp
 * @author gga
 * @date   Thu Nov 02 05:41:29 2006
 *
 * @brief  Version information for viewer
 *
 *
 */

#ifdef LINUX
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/version.h>
#include <libswscale/version.h>
#include <libswresample/version.h>
}


#include <GL/glew.h>
#include <MagickWand/MagickWand.h>

#include <OpenColorIO/OpenColorIO.h>

namespace OCIO = OCIO_NAMESPACE;

#include <OpenImageIO/argparse.h>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/sysutil.h>
#include <OpenImageIO/strutil.h>
#include <OpenImageIO/filesystem.h>
#include <OpenImageIO/filter.h>
#include <OpenImageIO/color.h>
#include <OpenImageIO/timer.h>
#include <OpenImageIO/oiioversion.h>

using namespace OIIO;
using namespace ImageBufAlgo;

#include <libraw/libraw.h>

#include <fltk/visual.h>
#include <fltk/Monitor.h>
#include <fltk/Browser.h>

#include <ImfVersion.h>

#include <boost/version.hpp>

#include "core/mrvOS.h"
#include "core/mrvCPU.h"
#include "gui/mrvVersion.h"
#include "gui/mrvImageView.h"
#include "gui/mrvIO.h"
#include "core/mrvOS.h"
#include "video/mrvDrawEngine.h"
#include "mrViewer.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif


namespace mrv
{



  static const char* kVersion = "4.3.8";
  static const char* kBuild = "- Built " __DATE__ " " __TIME__;

#if INTPTR_MAX == INT64_MAX
static const char* kArch = "64";
#elif INTPTR_MAX == INT32_MAX
static const char* kArch = "32";
#else
#error Unknown pointer size or missing size macros!
#endif


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

    f = new FormatInfo( true, false, false, "TX", "OpenImageIO",
                        "Arnold's TX" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "IFF", "internal",
                        "Autodesk Maya IFF" );
    formats.push_back(f);

    f = new FormatInfo( true, false, false, "DDS", "internal",
                        "Microsoft Direct Draw Surface" );
    formats.push_back(f);

    f = new FormatInfo( true, true, false, "PIC", "internal",
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

    f = new FormatInfo( true, true, false, "IFF", "internal",
                        "Maya IFF files" );
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

        f = new FormatInfo( m->decoder, m->encoder,
                            false,
                            m->name, "ImageMagick", m->description );
        formats.push_back( f );
      }
    magick_info = (const MagickInfo **)
      RelinquishMagickMemory((void *) magick_info);

    // Finally, add OIIO formats
    {
        std::string format_list;
        OIIO::getattribute ("input_format_list", format_list);
        typedef std::vector< std::string > InVec;
        InVec invec;
        mrv::split( invec, format_list, ',' );
        typedef std::list< std::string > InList;
        InList inlist;
        std::copy( invec.begin(), invec.end(), std::back_inserter( inlist ) );
        OIIO::getattribute ("output_format_list", format_list);
        InVec outvec;
        mrv::split( outvec, format_list, ',' );
        InVec::iterator i = outvec.begin();
        InVec::iterator e = outvec.end();

        // Add input & output formats first
        for ( ; i != e; ++i )
        {
            InList::iterator it = find( inlist.begin(), inlist.end(), *i );
            if ( it != inlist.end() )
            {
                f = new FormatInfo( true, true, false,
                                    (*i).c_str(), "OIIO", "" );
                formats.push_back( f );
                inlist.erase( it );
            }
        }

        // Add input formats only
        InList::iterator j = inlist.begin();
        InList::iterator t = inlist.end();
        for ( ; j != t; ++j )
        {
            f = new FormatInfo( true, false, false, (*j).c_str(), "OIIO", "" );
            formats.push_back( f );
        }
    }
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
        << ( cap & AV_CODEC_CAP_DRAW_HORIZ_BAND ? "S\t":" \t" )
        << ( cap & AV_CODEC_CAP_DR1 ? "D\t":" \t" )
        << ( cap & AV_CODEC_CAP_TRUNCATED ? "T\t":" \t" )
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
      "esa",
      "tss",
      "tdls",
      "ntss",
      "fss",
      "ds",
      "hexds",
      "epzs",
      "umh",
      NULL,
    };

    using namespace std;
    std::ostringstream o;

    const char** pp = motion_str;
    while (*pp) {
      o << *pp << endl;
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

    o << "mrViewer " << kArch << " bits - v" << kVersion << " "
      << kBuild << endl
#ifdef __GNUC__
      << "With gcc " << __GLIBCXX__ << endl
#else
      << "With msvc " << _MSC_VER << endl
#endif
      << "(C) 2007-2018 Film Aura, LLC." << endl
      << endl
      << "mrViewer depends on:" << endl
      << endl
      << "Boost v" << boost_major << "."
      << boost_minor << "." << boost_teeny << endl
      << "http://www.boost.org/" << endl
      << endl
      << "FLTK 2.0" << endl
      << "http://www.fltk.org/" << endl
      << "(C) 2000-2018 Bill Spitzak & others" << endl
      << endl
      << "GLEW " << glewGetString( GLEW_VERSION ) << endl
      << "http://glew.sourceforge.net/" << endl
      << "(C) SGI, Lev Povalahev, Milan Ikits" << endl
      << endl
      << MagickGetVersion(&magic) << endl
      << MagickGetCopyright() << endl
      << endl
      << "libavutil          v" << AV_STRINGIFY( LIBAVUTIL_VERSION ) << endl
      << "libavcodec      v" << AV_STRINGIFY( LIBAVCODEC_VERSION ) << endl
      << "libavformat     v" << AV_STRINGIFY( LIBAVFORMAT_VERSION ) << endl
      << "libavfilter        v" << AV_STRINGIFY( LIBAVFILTER_VERSION ) << endl
      << "libswresample v" << AV_STRINGIFY( LIBSWRESAMPLE_VERSION ) << endl
      << "libswscale       v" << AV_STRINGIFY( LIBSWSCALE_VERSION ) << endl
      << "http://ffmpeg.mplayerhq.hu/" << endl
      << "License: " << avcodec_license() << endl
      << "(C) 2000-2018 Fabrice Bellard, et al." << endl
      << "Configuration: " << avcodec_configuration() << endl << endl
      << "ILM OpenEXR v" << OPENEXR_VERSION_STRING <<  " or later" << endl
      << "http://www.openexr.org/" << endl
      << "(C) 2005-2018 Industrial Light & Magic" << endl
      << endl
      << "OpenColorIO v" << OCIO::GetVersion() <<  " or later" << endl
      << "http://www.opencolorio.org/" << endl
      << "(C) 2005-2018 Sony Pictures Imageworks" << endl
      << endl
      << "OpenImageIO v" << OIIO_VERSION_STRING <<  " or later" << endl
      << "http://www.openimageio.org/" << endl
      << "(C) 2003-2018 Larry Gritz, et al. " << endl
      << "All rights reserved." << endl;
    int columns = 80;
    std::string libs = OIIO::get_string_attribute("library_list");
    if (libs.size()) {
        std::vector<string_view> libvec;
        Strutil::split (libs, libvec, ";");
        for (auto& lib : libvec) {
            size_t pos = lib.find(':');
            lib.remove_prefix (pos+1);
        }
        o << "Dependent libraries:\n    "
          << Strutil::wordwrap(Strutil::join (libvec, ", "), columns, 4)
          << std::endl;
    }

    o << endl
      << "AMPAS ACES v1.0 or later" << endl
      << "https://github.com/ampas/aces-dev" << endl
      << "(C) 2018 AMPAS" << endl
      << endl
      << "AMPAS CTL v1.5 or later" << endl
      << "https://github.com/ampas/CTL" << endl
      << "(C) 2014-2018 AMPAS" << endl
      << endl
#ifdef GPL
      << "libdvdread" << endl
      << "libdvdnav" << endl
      << "libdvdcss" << endl
      << endl
#endif
      << "IccProfLib (ICC v4.2)" << endl
      << "(C) 2003-2018 - The International Color Consortium" << endl
      << endl
      << "LibRaw " << libraw_version() << endl
      << "(C) 2008-2018 LibRaw LLC (info@libraw.org)"
      << endl
      << endl;

    return o.str();
  }

  std::string cpu_information()
  {
    return GetCpuCaps(&gCpuCaps);
  }

#ifdef _WIN32
void  memory_information( uint64_t& totalVirtualMem,
                          uint64_t& virtualMemUsed,
                          uint64_t& virtualMemUsedByMe,
                          uint64_t& totalPhysMem,
                          uint64_t& physMemUsed,
                          uint64_t& physMemUsedByMe)
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    totalVirtualMem = memInfo.ullTotalPageFile;
    virtualMemUsed = totalVirtualMem - memInfo.ullAvailPageFile;
    totalVirtualMem /= (1024*1024);
    virtualMemUsed /= (1024*1024);
    totalPhysMem = memInfo.ullTotalPhys;
    physMemUsed = totalPhysMem - memInfo.ullAvailPhys;
    totalPhysMem /= (1024*1024);
    physMemUsed /= (1024*1024);

    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(),
                         (PPROCESS_MEMORY_COUNTERS) &pmc, sizeof(pmc));
    virtualMemUsedByMe = pmc.PrivateUsage;
    virtualMemUsedByMe /= (1024*1024);
    physMemUsedByMe = pmc.WorkingSetSize;
    physMemUsedByMe /= (1024*1024);
}
#endif

#ifdef LINUX

static int parseLine(char* line){
    int i = strlen(line);
    while (*line < '0' || *line > '9') line++;
    line[i-3] = '\0';
    i = atoi(line);
    return i;
}


static int getValue(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];


    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

int getValue2(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];


    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return result;
}

void  memory_information( uint64_t& totalVirtualMem,
                          uint64_t& virtualMemUsed,
                          uint64_t& virtualMemUsedByMe,
                          uint64_t& totalPhysMem,
                          uint64_t& physMemUsed,
                          uint64_t& physMemUsedByMe)
{

    struct sysinfo memInfo;
    sysinfo (&memInfo);

    totalVirtualMem = memInfo.totalram;
    // Add other values in next statement to avoid int overflow
    // on right hand side...
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;
    totalVirtualMem /= (1024*1024);

    virtualMemUsed = memInfo.totalram - memInfo.freeram;
    // Add other values in next statement to avoid int overflow on
    // right hand side...
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;
    virtualMemUsed /= (1024*1024);

    totalPhysMem = memInfo.totalram;
    //Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;
    totalPhysMem /= (1024*1024);

    physMemUsed = memInfo.totalram - memInfo.freeram;
    //Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;
    physMemUsed /= (1024*1024);

    virtualMemUsedByMe = getValue();
    virtualMemUsedByMe /= 1024;

    physMemUsedByMe = getValue2();
    physMemUsedByMe /= 1024;

}  // memory_information
#endif // LINUX

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
