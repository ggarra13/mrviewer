/*
    mrViewer - the professional movie and flipbook player
    Copyright (C) 2007-2020  Gonzalo Garramuño

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

#include <FL/Fl_Progress.H>
#include <FL/Fl_Output.H>
#include <FL/Enumerations.H>
#include <FL/fl_ask.H>

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
#include <FL/Fl_Native_File_Chooser.H>
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

static const std::string kMoviePattern = "3gp,asf,avc,avchd,avi,braw,BRAW,divx,dv,flv,m2ts,m2t,mkv,m4v,mp4,mpg,mpeg,mov,mxf,ogm,ogv,qt,r3d,R3D,ts,vob,VOB,vp9,webm,wmv,y4m";

static const std::string kImagePattern =
    "3fr,arw,bay,bmp,bmq,bit,cap,cin,cine,cr2,crw,cs1,ct,dc2,dcr,dng,dpx,dsc,drf,erf,exr,fff,gif,hdr,hdri,k25,kc2,kdc,ia,iff,iiq,jpg,jpeg,map,mef,nt,mdc,miff,mos,mrw,mt,nef,nrw,orf,pef,pic,png,ppm,pnm,pgm,pbm,psd,ptx,pxn,qtk,raf,ras,raw,rdc,rgb,rla,rpf,rw2,rwl,rwz,shmap,sgi,sr2,srf,srw,st,sti,sun,sxr,tga,tif,tiff,tx,x3f,zt";

static const std::string kProfilePattern = "icc,icm";

static const std::string kAudioPattern = "au,aiff,flac,m4a,mp3,ogg,wav";

static const std::string kSubtitlePattern = "srt,sub,ass,vtt";

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

// WINDOWS crashes if pattern is sent as is.  We need to sanitize the
// pattern here.  From:
// All (*.{png,ogg})\t
// to:
// All\t{*.png,ogg}\n
std::string pattern_to_native( std::string pattern )
{
    std::string ret;
    size_t pos = 0, pos2 = 0;
    while ( pos != std::string::npos )
    {
        pos = pattern.find( ' ' );
        if ( pos == std::string::npos ) break;
        ret += pattern.substr( 0, pos ) + '\t';
        size_t pos3 = pattern.find( '(', pos );
        size_t pos2 = pattern.find( ')', pos );
        ret += pattern.substr( pos3 + 1, pos2 - pos3 - 1 ) + '\n';
        if ( pos2 + 2 < pattern.size() )
            pattern = pattern.substr( pos2 + 2, pattern.size() );
        else
            pattern.clear();
    }
    return ret;
}

    const std::string file_save_single_requester(
        const char* title,
        const char* pattern,
        const char* startfile,
        const bool compact_images = true
        )
{
    std::string file;
    try
    {
        bool native = mrv::Preferences::native_file_chooser;
        if ( native )
        {
            Fl::check(); // Create native chooser
            Fl_Native_File_Chooser native;
            native.title(title);
            native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
            std::string native_pattern = pattern_to_native( pattern );
            native.filter(native_pattern.c_str());
            native.preset_file(startfile);
            // Show native chooser
           switch ( native.show() )
           {
              case -1:
                 LOG_ERROR( native.errmsg() );
                 break;	// ERROR
              case  1:
                break; // CANCEL
              default:                                                                        // PICKED FILE
                  if ( native.filename() ) {
                     file = native.filename();
                  }
           }
        }
        else
        {
            const char* f = flu_save_chooser( title, pattern, startfile, compact_images );
            if ( !f ) {
                return "";
            }
            file = f;
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

stringArray file_multi_requester(
    const char* title,
    const char* pattern,
    const char* startfile,
    const bool compact_images
)
{
    stringArray filelist;

    try
    {
        if ( !startfile ) startfile = "";
        bool native = mrv::Preferences::native_file_chooser;
        if ( native )
        {
            Fl::check(); // Create native chooser
            Fl_Native_File_Chooser native;
            native.title(title);
            native.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
            std::string native_pattern = pattern_to_native( pattern );
            native.filter(native_pattern.c_str());
            native.preset_file(startfile);
            // Show native chooser
           switch ( native.show() )
           {
              case -1:
                 LOG_ERROR( native.errmsg() );
                 break;	// ERROR
              case  1:
                break; // CANCEL
              default:                                                                        // PICKED FILE
                  if ( native.count() > 0 ) {
                      for ( int i = 0; i < native.count(); ++i )
                      {
                          filelist.push_back( native.filename(i) );
                      }
                  }
             }
        }
        else
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

std::string file_single_requester(
    const char* title,
    const char* pattern,
    const char* startfile
)
{
    std::string file;
    try {
        if ( !startfile ) startfile = "";
        bool native = mrv::Preferences::native_file_chooser;
        if ( native )
        {
            Fl::check(); // Create native chooser
            Fl_Native_File_Chooser native;
            native.title(title);
            native.type(Fl_Native_File_Chooser::BROWSE_FILE);
            std::string native_pattern = pattern_to_native( pattern );
            native.filter(native_pattern.c_str());
            native.preset_file(startfile);
            // Show native chooser
           switch ( native.show() )
           {
              case -1:
                 LOG_ERROR( native.errmsg() );
                 break;	// ERROR
              case  1:
                break; // CANCEL
              default:                                                                        // PICKED FILE
                  if ( native.count() > 0 )
                  {
                      file = native.filename();
                  }
           }
        }
        else
        {
            const char* f = flu_file_chooser( title, pattern, startfile );
            if ( f ) file = f;
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
                       ViewerUI* main )
{
    std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                kReelPattern + "})";
    std::string title = _("Load Reel(s)");

    stringArray files = file_multi_requester( title.c_str(),
                                              kREEL_PATTERN.c_str(),
                                              startfile, true );
    if ( main && (!main->uiMain || !main->uiMain->visible())) {
        return stringArray();
    }

    return files;
}


/**
 * Opens a file requester to load a directory of image files
 *
 * @param startfile  start filename (directory)
 *
 * @return A directory to be opened or NULL
 */
std::string open_directory( const char* startfile, ViewerUI* main )
{
    std::string dir;
    std::string title = _("Load Directory");
    bool native = mrv::Preferences::native_file_chooser;
    if ( native )
    {
        Fl_Native_File_Chooser native;
        native.title( title.c_str() );
        native.directory(startfile);
        native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
        // Show native chooser
        switch ( native.show() ) {
            case -1: LOG_ERROR(native.errmsg());
                break;	// ERROR
            case  1:
                break;		// CANCEL
            default:  // PICKED DIR
                if ( native.filename() ) {
                    dir = native.filename();
                }
                break;
        }
    }
    else
    {
        const char* d = flu_dir_chooser( title.c_str(), startfile );
        if (d) dir = d;
    }
    fs::path path = dir;
    return path.generic_string();
}

/**
 * Opens a file requester to load image files
 *
 * @param startfile  start filename (directory)
 *
 * @return Each file to be opened
 */
stringArray open_image_file( const char* startfile, const bool compact_images,
                             ViewerUI* main )
{
    const std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                      kReelPattern + "})\t";
    const std::string kAUDIO_PATTERN = ("Audios (*.{") + kAudioPattern +
                                       "})\t";
    const std::string kIMAGE_PATTERN = _("Images (*.{") +
                                       kImagePattern + "})\t";
    const std::string kALL_PATTERN = _("All (*.{") +
                                     kImagePattern + "," + kMoviePattern +
                                     "," + kReelPattern + "," +
                                     kAudioPattern + "})\t" +
                                     kIMAGE_PATTERN +
                                     kAUDIO_PATTERN +
                                     _("Movies (*.{") + kMoviePattern +
                                     "})\t" + kREEL_PATTERN;

    std::string pattern = kIMAGE_PATTERN + kAUDIO_PATTERN;
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
    std::string ret = make_ocio_chooser( img->ocio_input_color_space(),
                                         mrv::OCIOBrowser::kInputColorSpace );

    img->ocio_input_color_space( ret );
    char buf[256];
    sprintf( buf, "ICS \"%s\"", ret.c_str() );
    view->send_network( buf );
    view->redraw();
}

void attach_ocio_display( CMedia* img, ImageView* view )
{
    std::string ret = make_ocio_chooser( mrv::Preferences::OCIO_Display,
                                         mrv::OCIOBrowser::kDisplay );
    if ( ret.empty() ) return;
    mrv::Preferences::OCIO_Display = ret;
    img->image_damage( CMedia::kDamageAll );
    view->redraw();
}

void attach_ocio_view( CMedia* img, ImageView* view )
{
    std::string ret = make_ocio_chooser( mrv::Preferences::OCIO_View,
                                         mrv::OCIOBrowser::kView );
    if ( ret.empty() ) return;
    mrv::Preferences::OCIO_View = ret;
    Fl_Menu_Button* m = view->main()->gammaDefaults;
    for ( int i = 0; i < m->size(); ++i )
    {
        const char* lbl = m->menu()[i].label();
        if ( !lbl ) continue;
        if ( ret == lbl )
        {
            m->value(i);
            break;
        }
    }
    m->copy_label( _(ret.c_str()) );
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
std::string open_icc_profile( const char* startfile,
                              const char* title,
                              ViewerUI* main )
{
    stringArray filelist;

    if ( !startfile )
        startfile = getenv("ICC_PROFILES");

    if ( !startfile || strlen( startfile ) == 0 )
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

    std::string kICC_PATTERN   = _("Color Profiles (*.{" ) +
                                 kProfilePattern + "})";

    std::string profile = file_single_requester( title, kICC_PATTERN.c_str(),
                                                 startfile );


    if ( !profile.empty() ) mrv::colorProfile::add( profile.c_str() );
    return profile;
}



const char* open_ctl_dir( const char* startfile,
                          const char* title,
                          ViewerUI* main )
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
std::string open_subtitle_file( const char* startfile,
                                ViewerUI* main )
{
    std::string kSUBTITLE_PATTERN = _( "Subtitles (*.{" ) +
                                    kSubtitlePattern + "})\n";

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
std::string open_audio_file( const char* startfile,
                             ViewerUI* main )
{
    std::string kAUDIO_PATTERN = _( "Audios (*.{" ) +
                                 kAudioPattern + "})";

    std::string title = _("Load Audio");

    return file_single_requester( title.c_str(),
                                  kAUDIO_PATTERN.c_str(),
                                  startfile );
}




void attach_icc_profile( CMedia* image,
                         const char* startfile,
                         ViewerUI* main )
{
    if ( !image ) return;

    std::string profile = open_icc_profile( startfile, _("Attach ICC Profile"),
                                            main );
    if ( !profile.empty() )
        image->icc_profile( profile.c_str() );
    else
        image->icc_profile( NULL );
}



void attach_icc_profile( CMedia* image,
                         ViewerUI* main )
{
    if (!image) return;
    attach_icc_profile( image, image->icc_profile(), main );
}


void attach_rt_script( CMedia* image, const std::string& script,
                       ViewerUI* main )
{
    if ( ! script.empty() )
        main->uiView->send_network( "RT \"" + script + "\"" );

    if ( script.empty() ) image->rendering_transform( NULL );
    else  image->rendering_transform( script.c_str() );
}

void attach_ctl_script( CMedia* image, const char* startfile,
                        ViewerUI* main )
{
    if ( !image || !main ) return;

    std::string script = make_ctl_browser( startfile, "RRT,RT" );

    attach_rt_script( image, script, main );
}

void attach_look_mod_transform( CMedia* image, const std::string& script,
                                const size_t idx,
                                ViewerUI* main )
{
    char buf[1024];
    sprintf( buf, "LMT %zd \"%s\"", idx, script.c_str() );
    main->uiView->send_network( buf );

    if ( idx >= image->number_of_lmts() && script != "" )
        image->append_look_mod_transform( script.c_str() );
    else
        image->look_mod_transform( idx, script.c_str() );
}

void attach_ctl_lmt_script( CMedia* image, const char* startfile,
                            const size_t idx,
                            ViewerUI* main )
{
    if ( !image || !main ) return;

    // @todo: pass index to look mod
    std::string script = make_ctl_browser( startfile, "LMT,ACEScsc" );

    attach_look_mod_transform( image, script, idx, main );

}




void attach_ctl_script( CMedia* image,
                        ViewerUI* main )
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
                            ViewerUI* main )
{
    if ( !image || !main ) return;

    std::string script = make_ctl_browser( startfile, "ACEScsc,IDT" );

    char buf[1024];
    sprintf( buf, "IDT \"%s\"", script.c_str() );
    main->uiView->send_network( buf );

    image->idt_transform( script.c_str() );
}

void attach_ctl_idt_script( CMedia* image,
                            ViewerUI* main )
{
    if ( !image || !main ) return;

    const char* transform = image->idt_transform();
    if ( !transform )  transform = "";
    attach_ctl_idt_script( image, transform, main );
}

void attach_ctl_lmt_script( CMedia* image, const size_t idx,
                            ViewerUI* main )
{
    if ( !image || !main ) return;

    const char* transform = image->look_mod_transform(idx);
    if ( !transform )  transform = "";

    attach_ctl_lmt_script( image, transform, idx, main );
}


std::string open_ocio_config( const char* startfile )
{
    std::string kOCIO_PATTERN = _("OCIO config (*.{") +
                                kOCIOPattern + "})";
    std::string title = _("Load OCIO Config");

    std::string file = file_single_requester( title.c_str(),
                                              kOCIO_PATTERN.c_str(),
                                              startfile );
    return file;
}

void read_clip_xml_metadata( CMedia* img,
                             ViewerUI* main )
{
    if ( !img ) return;

    std::string xml = aces_xml_filename( img->fileroot() );

    std::string kXML_PATTERN = _("XML Clip Metadata (*.{") +
                               kXMLPattern + "})";

    std::string title = _("Load XML Clip Metadata");

    stringArray filelist;

    std::string file = file_single_requester( title.c_str(),
                       kXML_PATTERN.c_str(),
                       xml.c_str() );
    if ( file.empty() ) return;

    load_aces_xml( img, file.c_str() );

}

void save_clip_xml_metadata( const CMedia* img,
                             ViewerUI* main )
{
    if ( !img ) return;

    std::string xml = aces_xml_filename( img->fileroot() );

    std::string kXML_PATTERN = _("XML Clip Metadata (*.{") +
                               kXMLPattern + "})";

    std::string title = _( "Save XML Clip Metadata" );

    std::string file = file_save_single_requester( title.c_str(),
                       kXML_PATTERN.c_str(),
                       xml.c_str() );
    if ( file.empty() ) return;

    save_aces_xml( img, file.c_str() );
}

void monitor_ctl_script( ViewerUI* main,
                         const unsigned index, const char* startfile )
{
    if ( !startfile )
        startfile = mrv::Preferences::ODT_CTL_transform.c_str();

    std::string script = make_ctl_browser( startfile, "ODT" );

    mrv::Preferences::ODT_CTL_transform = script;
    main->uiView->send_network( "ODT \"" + script + "\"" );

    main->uiView->redraw();

    // @todo: prefs
    //     uiCTL_display_transform->static_text( script );
    //     uiCTL_display_transform->do_callback();
}

void monitor_icc_profile( ViewerUI* main,
                          const unsigned index )
{
    std::string profile = open_icc_profile( NULL,
                                            "Load Monitor Profile" );
    if ( profile.empty() ) return;

    mrv::Preferences::ODT_ICC_profile = profile;
    mrv::colorProfile::set_monitor_profile( profile.c_str(), index );

    main->uiView->redraw();
}



void save_image_file( CMedia* image, const char* startdir, bool aces,
                      bool all_layers,
                      ViewerUI* main )
{
    if (!image) return;

    std::string title = "Save Image";

    const std::string kIMAGE_PATTERN = _("Images (*.{") +
                                       kImagePattern + "})\t";
    std::string pattern = kIMAGE_PATTERN;

    if (!startdir) startdir = "";

    const std::string& file = file_save_single_requester( title.c_str(),
                                                   pattern.c_str(),
                                                   startdir, false );
    if ( file.empty() ) return;


    std::string ext = file;
    size_t pos = ext.rfind( '.' );
    if ( pos != std::string::npos && pos != ext.size() )
        ext = ext.substr( pos, ext.size() );


    ImageOpts* opts = ImageOpts::build( main, ext, image->has_deep_data() );
    if ( opts->active() )
    {
        // Set icon back to WAIT
        main->uiView->toggle_wait();
        main->uiView->handle( FL_ENTER );
        Fl::check();

        image->save( file.c_str(), opts );

        // Change icon back to ARROW/CROSSHAIR
        main->uiView->toggle_wait();
        main->uiView->handle( FL_ENTER );
        Fl::check();

        save_xml( image, opts, file.c_str() );
    }

    delete opts;
}

void save_sequence_file( ViewerUI* uiMain,
                         const char* startdir, const bool opengl)
{
    const std::string kIMAGE_PATTERN = _("Images (*.{") +
                                       kImagePattern + "})\t";

    std::string title = _("Save Sequence");
    stringArray filelist;
    if ( !startdir ) startdir = "";

    const std::string& file = file_save_single_requester( title.c_str(),
                                                          kIMAGE_PATTERN.c_str(),
                                                          startdir, true );
    if ( file.empty() ) return;

    save_movie_or_sequence( file.c_str(), uiMain, opengl );

}





/**
 * Save a reel under a new filename
 *
 * @param startdir start directory to save to
 *
 * @return filename of reel to save or NULL
 */
std::string save_reel( const char* startdir,
                       ViewerUI* main )
{
    std::string kREEL_PATTERN = _( "Reels (*.{" ) +
                                kReelPattern + "})\n";

    std::string title = _("Save Reel");
    if ( !startdir ) startdir = "";


    return file_save_single_requester(title.c_str(), kREEL_PATTERN.c_str(),
                                      startdir);
}



}  // namespace mrv
