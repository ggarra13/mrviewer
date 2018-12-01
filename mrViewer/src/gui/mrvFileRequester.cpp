/*
    mrViewer - the professional movie and flipbook player
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
 * @file   mrvFileRequester.cpp
 * @author gga
 * @date   Fri Jul  6 17:37:49 2007
 *
 * @brief  This file implements several generic file requesters used by
 *         other classes.
 *
 *
 */

#include <inttypes.h>

#include <fltk/file_chooser.h>
#include <fltk/ProgressBar.h>
#include <fltk/Output.h>
#include <fltk/Cursor.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/ask.h>

#include "core/mrvString.h"
#include "core/CMedia.h"
#include "core/mrvImageOpts.h"
#include "core/aviImage.h"
#include "core/mrvACES.h"
#include "core/mrvI8N.h"
#include "core/mrvColorProfile.h"
#include "core/mrvPlayback.h"
#include "gui/mrvIO.h"
#include "gui/mrvSave.h"
#include "gui/mrvImageBrowser.h"
#include "gui/mrvFileRequester.h"
#include "gui/mrvPreferences.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"
#include "gui/mrvLogDisplay.h"
#include "gui/mrvProgressReport.h"
#include "gui/mrvMainWindow.h"
#include "gui/mrvOCIOBrowser.h"
#include "mrViewer.h"
#include "aviSave.h"

#include <GL/gl.h>

#include "FLU/Flu_File_Chooser.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace {

static const char* kModule = "file";

#ifdef WIN32
#define kSeparator ";"
#else
#define kSeparator ":"
#endif

  // File extension patterns
  static const std::string kReelPattern = "reel";

  static const std::string kMoviePattern = "3gp,asf,avc,avchd,avi,divx,dv,flv,m2ts,m2t,mkv,m4v,mp4,mpg,mpeg,mov,mxf,ogm,ogv,qt,ts,vob,vp9,webm,wmv,y4m,";

  static const std::string kImagePattern =
    "3fr,arw,bay,bmp,bmq,bit,cap,cin,cine,cr2,crw,cs1,ct,dc2,dcr,dng,dpx,dsc,drf,erf,exr,fff,gif,hdr,hdri,k25,kc2,kdc,ia,iff,iiq,jpg,jpeg,map,mef,nt,mdc,miff,mos,mrw,mt,nef,nrw,orf,pef,pic,png,ppm,pnm,pgm,pbm,psd,ptx,pxn,qtk,raf,ras,raw,rdc,rgb,rla,rpf,rw2,rwl,rwz,shmap,sgi,sr2,srf,srw,st,sti,sun,sxr,tga,tif,tiff,tx,x3f,zt";

  static const std::string kProfilePattern = "icc,icm";

  static const std::string kAudioPattern = "m4a,mp3,ogg,wav";

  static const std::string kSubtitlePattern = "srt,sub,ass";

  static const std::string kCTLPattern = "ctl";

  static const std::string kXMLPattern = "xml";

  static const std::string kOCIOPattern = "ocio";


  // Actual FLTK file requester patterns



// static const std::string kSAVE_IMAGE_PATTERN = _("Images (*.{") +
//                                                kImagePattern + "})";


// static const std::string kCTL_PATTERN = _("CTL script (*.{") +
//                                         kCTLPattern + "})\t";


}


namespace mrv
{

const char* file_save_single_requester(
                                       const char* title,
                                       const char* pattern,
                                       const char* startfile,
                                       const bool compact_images = true
                                       )
{
    const char* file = NULL;
    try
    {
#ifdef _WIN32
        bool native = mrv::Preferences::native_file_chooser;
        fltk::use_system_file_chooser( native );
        if ( native )
        {
            fltk::check();
            file = fltk::file_chooser( title, pattern, startfile,
                                       compact_images );
            if ( !file ) return "";
        }
        else
#endif
        {
            file = flu_save_chooser( title, pattern, startfile,
                                     compact_images );
        }
        if ( !file ) return "";
    }
    catch ( const std::exception& e )
    {
        LOG_ERROR( e.what() );
    }
    catch ( ... )
    {
    }

    return file;
}

stringArray file_multi_requester(
                                 const char* title,
                                 const char* pattern,
                                 const char* startfile,
                                 const bool compact_images = true
                                 )
{
    stringArray filelist;

    try
    {
        if ( !startfile ) startfile = "";
#ifdef _WIN32
        bool native = mrv::Preferences::native_file_chooser;
        fltk::use_system_file_chooser( native );
        if ( native )
        {
            fltk::check();
            const char* file = fltk::file_chooser( title,
                                                   pattern,
                                                   startfile );
            if ( file )
                split( filelist, file, '\n' );
        }
        else
#endif
        {
            flu_multi_file_chooser( title, pattern, startfile,
                                    filelist, compact_images );
        }
    }
    catch ( const std::exception& e )
    {
        LOG_ERROR( e.what() );
    }
    catch ( ... )
    {
    }
    return filelist;
}

const char* file_single_requester(
                                  const char* title,
                                  const char* pattern,
                                  const char* startfile
                                  )
{
    const char* file = NULL;
    try {
#ifdef _WIN32
        bool native = mrv::Preferences::native_file_chooser;
        fltk::use_system_file_chooser( native );
        if ( native )
        {
            fltk::check();
            if ( !startfile ) startfile = "";
            file = fltk::file_chooser( title, pattern, startfile );
            // if ( file )
            //     split( filelist, file, '\n' );
            // file = filelist[0].c_str();
        }
        else
#endif
        {
            file = flu_file_chooser( title, pattern, startfile );
        }
    }
    catch ( const std::exception& e )
    {
        LOG_ERROR( e.what() );
    }
    catch ( ... )
    {
    }


    return file;
}

  /**
   * Open one or more mrViewer's reels
   *
   * @param startfile optional start file or directory
   *
   * @return Each reel to be loaded
   */
stringArray open_reel( const char* startfile,
                       const mrv::ViewerUI* main )
  {
      std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                  kReelPattern + "})\t";
      std::string title = _("Load Reel(s)");

      stringArray files = file_multi_requester( title.c_str(),
                                               kREEL_PATTERN.c_str(),
                                               startfile );
      if ( main && (!main->uiMain || !main->uiMain->visible())) {
          return stringArray();
      }

      return files;
  }


  /**
   * Opens a file requester to load image files
   *
   * @param startfile  start filename (directory)
   *
   * @return Each file to be opened
   */
std::string open_directory( const char* startfile, const mrv::ViewerUI* main )
{
    std::string dir;
    std::string title = _("Load Directory");
    const char* d = flu_dir_chooser( title.c_str(), startfile );
    if (d) dir = d;
    return dir;
}

  /**
   * Opens a file requester to load image files
   *
   * @param startfile  start filename (directory)
   *
   * @return Each file to be opened
   */
stringArray open_image_file( const char* startfile, const bool compact_images,
                             const mrv::ViewerUI* main )
  {
    const std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                      kReelPattern + "})\t";
    const std::string kIMAGE_PATTERN = _("Images (*.{") +
                                       kImagePattern + "})";
    const std::string kALL_PATTERN = _("All Recognized (*.{") +
                                     kImagePattern + "," + kMoviePattern +
                                     "," + kReelPattern + "," +
                                     kAudioPattern + "})\t" +
                                     _("Images (*.{") + kImagePattern +
                                     "})\t" +
                                     _("Movies (*.{") + kMoviePattern +
                                     "})\t" +
                                     _("Audios (*.(") + kAudioPattern +
                                     "})\t" + kREEL_PATTERN;

    std::string pattern = kIMAGE_PATTERN;
    std::string title = _("Load Image");
    if ( compact_images ) {
        title = _("Load Movie or Sequence");
        pattern = kALL_PATTERN;
    }

    return file_multi_requester( title.c_str(), pattern.c_str(),
                                 startfile, compact_images );
  }




void attach_ocio_input_color_space( CMedia* img, ImageView* view )
{
    std::string ret = make_ocio_browser( img->ocio_input_color_space(),
                                         mrv::OCIOBrowser::kInputColorSpace );
    if ( ret.empty() ) return;

    img->ocio_input_color_space( ret );
    char buf[256];
    sprintf( buf, "ICS \"%s\"", ret.c_str() );
    view->send_network( buf );
    view->redraw();
}

void attach_ocio_display( CMedia* img, ImageView* view )
{
    std::string ret = make_ocio_browser( mrv::Preferences::OCIO_Display,
                                         mrv::OCIOBrowser::kDisplay );
    if ( ret.empty() ) return;
    mrv::Preferences::OCIO_Display = ret;
    img->image_damage( CMedia::kDamageAll );
    view->redraw();
}

void attach_ocio_view( CMedia* img, ImageView* view )
{
    std::string ret = make_ocio_browser( mrv::Preferences::OCIO_View,
                                         mrv::OCIOBrowser::kView );
    if ( ret.empty() ) return;
    mrv::Preferences::OCIO_View = ret;
    fltk::PopupMenu* m = view->main()->gammaDefaults;
    for ( int i = 0; i < m->children(); ++i )
    {
        if ( ret == m->child(i)->label() )
        {
            m->value(i);
            break;
        }
    }
    m->label( strdup( _(ret.c_str()) ) );
    m->redraw();
    img->image_damage( CMedia::kDamageAll );
    view->redraw();
}

  /**
   * Opens a file requester to load an icc profile
   *
   * @param startfile  start filename (directory)
   *
   * @return  opened audio file or null
   */
  const char* open_icc_profile( const char* startfile,
                                const char* title,
                                const mrv::ViewerUI* main )
  {
      stringArray filelist;

    if ( !startfile )
        startfile = getenv("ICC_PROFILES");

    if ( !startfile || strlen( startfile ) <= 0 )
      {
#if defined(WIN32) || defined(WIN64)
        char buf[256];
        sprintf( buf, "%s/SYSTEM32/spool/drivers/color",
                 getenv("WINDIR") );
        startfile = buf;
#else
        startfile = "/usr/share/color/icc";
#endif
      }

    std::string kICC_PATTERN   = _("Color Profiles (*." ) +
                                   kProfilePattern + ")";

    const char* profile = file_single_requester( title, kICC_PATTERN.c_str(),
                                                 startfile );


    if ( profile ) mrv::colorProfile::add( profile );
    return profile;
  }



const char* open_ctl_dir( const char* startfile,
                          const char* title,
                          const mrv::ViewerUI* main )
{
    std::string path, modulepath, ext;

    if ( !startfile )
        startfile = getenv("CTL_MODULE_PATH");

    if ( startfile )
    {
        modulepath = "CTL_MODULE_PATH=";
        modulepath += startfile;
        path = startfile;
        size_t len = path.find( kSeparator );
        path = path.substr( 0, len-1 );
    }

    const char* profile = flu_dir_chooser(title,
                                          path.c_str());
    if ( profile )
    {
        path = profile;
        modulepath += kSeparator;
        modulepath += path;
        putenv( strdup( modulepath.c_str() ) );
        profile = strdup( ext.c_str() );
    }
    return profile;
}

/**
   * Opens a file requester to load subtitle files
   *
   * @param startfile  start filename (directory)
   *
   * @return  opened subtitle file or null
   */
  const char* open_subtitle_file( const char* startfile,
                                  const mrv::ViewerUI* main )
  {
      std::string kSUBTITLE_PATTERN = _( "Subtitles (*.{" ) +
                                      kSubtitlePattern + "})\t";

      std::string title = _("Load Subtitle");

      return file_single_requester( title.c_str(),
                                    kSUBTITLE_PATTERN.c_str(),
                                    startfile );
  }

/**
   * Opens a file requester to load audio files
   *
   * @param startfile  start filename (directory)
   *
   * @return  opened audio file or null
   */
  const char* open_audio_file( const char* startfile,
                               const mrv::ViewerUI* main )
  {
      std::string kAUDIO_PATTERN = _( "Audios (*.{" ) +
                                   kAudioPattern + "})\t";

      std::string title = _("Load Audio");

      return file_single_requester( title.c_str(),
                                    kAUDIO_PATTERN.c_str(),
                                    startfile );
  }




  void attach_icc_profile( CMedia* image,
                           const char* startfile,
                           const mrv::ViewerUI* main )
  {
    if ( !image ) return;

    const char* profile = open_icc_profile( startfile, _("Attach ICC Profile"),
                                            main );
    image->icc_profile( profile );
  }



  void attach_icc_profile( CMedia* image,
                           const mrv::ViewerUI* main )
  {
    if (!image) return;
    attach_icc_profile( image, image->icc_profile(), main );
  }


void attach_rt_script( CMedia* image, const std::string& script,
                       const mrv::ViewerUI* main )
{
    if ( ! script.empty() )
        main->uiView->send_network( "RT \"" + script + "\"" );

    if ( script.empty() ) image->rendering_transform( NULL );
    else  image->rendering_transform( script.c_str() );
}

void attach_ctl_script( CMedia* image, const char* startfile,
                        const mrv::ViewerUI* main )
{
    if ( !image || !main ) return;

    std::string script = make_ctl_browser( startfile, "RRT,RT" );

    attach_rt_script( image, script, main );
}

void attach_look_mod_transform( CMedia* image, const std::string& script,
                                const size_t idx,
                                const mrv::ViewerUI* main )
{
    char buf[1024];
    sprintf( buf, "LMT %d \"%s\"", idx, script.c_str() );
    main->uiView->send_network( buf );

    if ( idx >= image->number_of_lmts() && script != "" )
        image->append_look_mod_transform( script.c_str() );
    else
        image->look_mod_transform( idx, script.c_str() );
}

void attach_ctl_lmt_script( CMedia* image, const char* startfile,
                            const size_t idx,
                            const mrv::ViewerUI* main )
  {
    if ( !image || !main ) return;

    // @todo: pass index to look mod
    std::string script = make_ctl_browser( startfile, "LMT,ACEScsc" );

    attach_look_mod_transform( image, script, idx, main );

  }




  void attach_ctl_script( CMedia* image,
                          const mrv::ViewerUI* main )
  {
      if (!image || !main ) {
          return;
      }

    const char* transform = image->rendering_transform();
    if ( !transform )
      transform = mrv::CMedia::rendering_transform_float.c_str();

    attach_ctl_script( image, transform, main );
  }

  void attach_ctl_idt_script( CMedia* image, const char* startfile,
                              const mrv::ViewerUI* main )
  {
    if ( !image || !main ) return;

    std::string script = make_ctl_browser( startfile, "ACEScsc,IDT" );

    char buf[1024];
    sprintf( buf, "IDT \"%s\"", script.c_str() );
    main->uiView->send_network( buf );

    image->idt_transform( script.c_str() );
  }

  void attach_ctl_idt_script( CMedia* image,
                              const mrv::ViewerUI* main )
  {
    if ( !image || !main ) return;

    const char* transform = image->idt_transform();
    if ( !transform )  transform = "";
    attach_ctl_idt_script( image, transform, main );
  }

void attach_ctl_lmt_script( CMedia* image, const size_t idx,
                            const mrv::ViewerUI* main )
{
    if ( !image || !main ) return;

    const char* transform = image->look_mod_transform(idx);
    if ( !transform )  transform = "";

    attach_ctl_lmt_script( image, transform, idx, main );
}


std::string open_ocio_config( const char* startfile )
{
    std::string kOCIO_PATTERN = _("OCIO config (*.{") +
                               kOCIOPattern + "})\t";
    std::string title = _("Load OCIO Config");

    const char* file = file_single_requester( title.c_str(),
                                              kOCIO_PATTERN.c_str(),
                                              startfile );
    if ( !file ) return "";
    return file;
}

  void read_clip_xml_metadata( CMedia* img,
                               const mrv::ViewerUI* main )
  {
    if ( !img ) return;

    std::string xml = aces_xml_filename( img->fileroot() );

    std::string kXML_PATTERN = _("XML Clip Metadata (*.{") +
                               kXMLPattern + "})\t";

    std::string title = _("Load XML Clip Metadata");

    stringArray filelist;

    const char* file = file_single_requester( title.c_str(),
                                              kXML_PATTERN.c_str(),
                                              xml.c_str() );
    if ( !file ) return;

    load_aces_xml( img, file );

  }

void save_clip_xml_metadata( const CMedia* img,
                             const mrv::ViewerUI* main )
{
    if ( !img ) return;

    std::string xml = aces_xml_filename( img->fileroot() );

    std::string kXML_PATTERN = _("XML Clip Metadata (*.{") +
                               kXMLPattern + "})\t";

    std::string title = _( "Save XML Clip Metadata" );

    const char* file = file_save_single_requester( title.c_str(),
                                                   kXML_PATTERN.c_str(),
                                                   xml.c_str() );
    if ( !file || strlen(file) == 0 ) return;

    save_aces_xml( img, file );
}

void monitor_ctl_script( const mrv::ViewerUI* main,
                         const unsigned index, const char* startfile )
  {
    if ( !startfile )
      startfile = mrv::Preferences::ODT_CTL_transform.c_str();

    std::string script = make_ctl_browser( startfile, "ODT" );
    if ( script.empty() ) return;

    mrv::Preferences::ODT_CTL_transform = script;
    main->uiView->send_network( "ODT \"" + script + "\"" );

    main->uiView->redraw();

    // @todo: prefs
    //     uiCTL_display_transform->static_text( script );
    //     uiCTL_display_transform->do_callback();
  }

  void monitor_icc_profile( const mrv::ViewerUI* main,
                            const unsigned index )
  {
    const char* profile = open_icc_profile( NULL,
                                            "Load Monitor Profile" );
    if ( !profile ) return;

    mrv::Preferences::ODT_ICC_profile = profile;
    mrv::colorProfile::set_monitor_profile( profile, index );

    main->uiView->redraw();
  }



void save_image_file( CMedia* image, const char* startdir, bool aces,
                      bool all_layers,
                      const mrv::ViewerUI* main )
{
   if (!image) return;

   std::string title = "Save Image";

   std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                               kReelPattern + "})\t";
   std::string kIMAGE_PATTERN = _("All Recognized (*.{") +
                                 kImagePattern + "," + kMoviePattern +
                                "," + kReelPattern + "," +
                                 kAudioPattern + "})\t" +
                                _("Images (*.{") + kImagePattern +
                                "})\t" +
                                _("Movies (*.{") + kMoviePattern +
                                "})\t" +
                                _("Audios (*.(") + kAudioPattern +
                                "})\t" + kREEL_PATTERN;
   std::string pattern = kIMAGE_PATTERN;

   if (!startdir) startdir = "";

   const char* file = file_save_single_requester( title.c_str(),
                                                  pattern.c_str(),
                                                  startdir, false );
   if ( file == NULL || strlen(file) == 0 ) return;


    std::string ext = file;
    size_t pos = ext.rfind( '.' );
    if ( pos != std::string::npos && pos != ext.size() )
        ext = ext.substr( pos, ext.size() );


    ImageOpts* opts = ImageOpts::build( main, ext, image->has_deep_data() );
    if ( opts->active() )
    {
        // Set icon back to WAIT
        main->uiView->toggle_wait();
        main->uiView->handle( fltk::ENTER );
        fltk::check();

        image->save( file, opts );

       // Change icon back to ARROW/CROSSHAIR
       main->uiView->toggle_wait();
       main->uiView->handle( fltk::ENTER );
       fltk::check();

       save_xml( image, opts, file );
   }

   delete opts;
}

void save_sequence_file( const mrv::ViewerUI* uiMain,
                         const char* startdir, const bool opengl)
{
    std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                kReelPattern + "})\t";
   std::string kIMAGE_PATTERN = _("All Recognized (*.{") +
                                 kImagePattern + "," + kMoviePattern +
                                "," + kReelPattern + "," +
                                 kAudioPattern + "})\t" +
                                _("Images (*.{") + kImagePattern +
                                "})\t" +
                                _("Movies (*.{") + kMoviePattern +
                                "})\t" +
                                _("Audios (*.(") + kAudioPattern +
                                "})\t" + kREEL_PATTERN;

   std::string title = _("Save Sequence");
   stringArray filelist;
   const char* file = NULL;
   if ( !startdir ) startdir = "";

   file = file_save_single_requester( title.c_str(), kIMAGE_PATTERN.c_str(),
                                      startdir, true );
   if ( !file || strlen(file) == 0 ) return;

   save_movie_or_sequence( file, uiMain, opengl );

}


  /**
   * Attach a new audio file to loaded sequence
   *
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_audio( CMedia* image, const char* startfile,
                     const mrv::ViewerUI* main )
  {
    if ( !image ) return;
    if ( !image->is_sequence() ) return;

    const char* audio = open_audio_file( startfile );
    if ( !audio ) return;

    // image->audio( audio );
  }


  /**
   * Save a reel under a new filename
   *
   * @param startdir start directory to save to
   *
   * @return filename of reel to save or NULL
   */
const char* save_reel( const char* startdir,
                       const mrv::ViewerUI* main )
{
    std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                kReelPattern + "})\t";

    std::string title = _("Save Reel");
    if ( !startdir ) startdir = "";


    return file_save_single_requester(title.c_str(), kREEL_PATTERN.c_str(),
                                      startdir);
}



}  // namespace mrv
