
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
 * @file   mrvFileRequester.cpp
 * @author gga
 * @date   Fri Jul  6 17:37:49 2007
 * 
 * @brief  This file implements several generic file requesters used by
 *         other classes.
 * 
 * 
 */

#define __STDC_FORMAT_MACROS

#include <inttypes.h>

#include <fltk/file_chooser.h>
#include <fltk/ProgressBar.h>
#include <fltk/run.h>
#include <fltk/ask.h>

#include "CMedia.h"
#include "mrvImageBrowser.h"
#include "mrvColorProfile.h"
#include "mrvFileRequester.h"
#include "mrvPreferences.h"
#include "mrViewer.h"
#include "aviSave.h"
#include "core/mrvImageOpts.h"
#include "core/aviImage.h"
#include "core/mrvACES.h"
#include "core/mrvI8N.h"
#include "gui/mrvIO.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"

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

  static const std::string kMoviePattern = "mp4,MP4,mpg,MPG,mpeg,MPEG,mov,MOV,qt,QT,avi,AVI,flv,FLV,divx,DIVX,wmv,WMV,vob,VOB";

  static const std::string kImagePattern =
    "bmp,bit,cin,ct,dpx,exr,gif,GIF,hdr,iff,jpg,JPG,jpeg,JPEG,map,nt,mt,pic,png,psd,rgb,rpf,shmap,sgi,st,sxr,tga,tif,tiff,zt";

  static const std::string kProfilePattern = "icc,icm,ICC,ICM";

  static const std::string kAudioPattern = "mp3,MP3,ogg,OGG,wav,WAV";

  static const std::string kCTLPattern = "ctl,CTL";


  // Actual FLTK file requester patterns
  static const std::string kICC_PATTERN   = 
    "Color Profiles (*." + kProfilePattern + ")";

  static const std::string kREEL_PATTERN = 
    "Reels (*.{"  + kReelPattern + "})\t"
    ;

  static const std::string kAUDIO_PATTERN = 
    "Audios (*.{" + kAudioPattern + "})\t"
    ;

  static const std::string kSAVE_IMAGE_PATTERN = 
    "Images (*.{" + kImagePattern + "})";

  static const std::string kIMAGE_PATTERN = 
    "All Recognized (*.{" + kImagePattern + "," + kMoviePattern + "," + 
    kReelPattern + "," + kAudioPattern + "})\t" +
    "Images (*.{" + kImagePattern + "})\t" +
    "Movies (*.{" + kMoviePattern + "})\t" +
    "Audios (*.(" + kAudioPattern + "})\t" +
    kREEL_PATTERN;

static const std::string kCTL_PATTERN =
"CTL script (*.{" + kCTLPattern + "})\t";

}


namespace mrv
{


  /** 
   * Open one or more mrViewer's reels
   * 
   * @param startfile optional start file or directory
   * 
   * @return Each reel to be loaded
   */
  stringArray open_reel( const char* startfile )
  {
    stringArray filelist;
    flu_multi_file_chooser( _("Load Reel(s)"), 
			    kREEL_PATTERN.c_str(), startfile,
			    filelist, false );
    return filelist;
  }


  /** 
   * Opens a file requester to load image files
   * 
   * @param startfile  start filename (directory)
   * 
   * @return Each file to be opened
   */
stringArray open_image_file( const char* startfile, const bool compact_images )
  {
    stringArray filelist;

    std::string title = _("Load Image");
    if ( compact_images ) title = _("Load Movie or Sequence");


    flu_multi_file_chooser( title.c_str(), 
			    kIMAGE_PATTERN.c_str(), startfile,
			    filelist, compact_images );

    return filelist;
  }




  /** 
   * Opens a file requester to load an icc profile
   * 
   * @param startfile  start filename (directory)
   * 
   * @return  opened audio file or null
   */
  const char* open_icc_profile( const char* startfile,
				const char* title )
  {
    std::string path;

    if ( !startfile )
        startfile = getenv("ICC_PROFILES");

    if ( !startfile || strlen( startfile ) <= 0 )
      {
#if defined(WIN32) || defined(WIN64)
	char buf[256];
	sprintf( buf, "%s/SYSTEM32/spool/drivers/color", 
		 getenv("WINDIR") );
	path = buf;
#else
	path = "/usr/share/color/icc";
#endif
      }
    else
      {
	path = startfile;
      }

    const char* profile = flu_file_chooser(title, 
					   kICC_PATTERN.c_str(), 
					   path.c_str());
    if ( profile ) mrv::colorProfile::add( profile );
    return profile;
  }



  const char* open_ctl_script( const char* startfile,
                               const char* title )
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

    const char* profile = flu_file_chooser(title, 
					   kCTL_PATTERN.c_str(), 
					   path.c_str());
    if ( profile )
    {
        path = profile;
        size_t len = path.rfind( '/' );
        if ( len == 0 ) return NULL;

        ext  = path.substr( len+1, path.size() );
        path = path.substr( 0, len );
        modulepath += kSeparator;
        modulepath += path;
        putenv( strdup( modulepath.c_str() ) );
        profile = strdup( ext.c_str() );
    }
    return profile;
  }

  /** 
   * Opens a file requester to load audio files
   * 
   * @param startfile  start filename (directory)
   * 
   * @return  opened audio file or null
   */
  const char* open_audio_file( const char* startfile )
  {
    return flu_file_chooser("Load Audio", 
			    kAUDIO_PATTERN.c_str(), startfile);
  }




  void attach_icc_profile( CMedia* image,
			   const char* startfile )
  {
    if ( !image ) return;

    const char* profile = open_icc_profile( startfile );
    image->icc_profile( profile );
  }



  void attach_icc_profile( CMedia* image )
  { 
    if (!image) return;
    attach_icc_profile( image, image->icc_profile() );
  }


  void attach_ctl_script( CMedia* image, const char* startfile )
  {
    if ( !image ) return;

    std::string script = make_ctl_browser( startfile, "RRT" );
    image->rendering_transform( script.c_str() );
  }

void attach_ctl_lmt_script( CMedia* image, const char* startfile,
                            const size_t idx )
  {
    if ( !image ) return;

    // @todo: pass index to look mod
    std::string script = make_ctl_browser( startfile, "LMT" );
    if ( idx >= image->number_of_lmts() )
        image->append_look_mod_transform( script.c_str() );
    else
        image->look_mod_transform( idx, script.c_str() );
  }



  void attach_ctl_script( CMedia* image )
  { 
    if (!image) return;

    const char* transform = image->rendering_transform();
    if ( !transform ) 
      transform = mrv::CMedia::rendering_transform_float.c_str();

    attach_ctl_script( image, transform );
  }

  void attach_ctl_idt_script( CMedia* image, const char* startfile )
  {
    if ( !image ) return;

    std::string script = make_ctl_browser( startfile, "IDT" );
    image->idt_transform( script.c_str() );
  }

  void attach_ctl_idt_script( CMedia* image )
  {
    if ( !image ) return;

    const char* transform = image->idt_transform();
    if ( !transform )  transform = "";
    attach_ctl_idt_script( image, transform );
  }

void attach_ctl_lmt_script( CMedia* image, const size_t idx )
{
    if ( !image ) return;

    // @todo: pass the index here
    const char* transform = image->look_mod_transform(idx);
    if ( !transform )  transform = "";
    attach_ctl_lmt_script( image, transform, idx );
}




  void monitor_ctl_script( const unsigned index, const char* startfile )
  {
    if ( !startfile )
      startfile = mrv::Preferences::ODT_CTL_transform.c_str();

    std::string script = make_ctl_browser( startfile, "ODT" );
    if ( script.empty() ) return;

    mrv::Preferences::ODT_CTL_transform = script;

    // @todo: prefs
    //     uiCTL_display_transform->static_text( script );
    //     uiCTL_display_transform->do_callback();
  }
			
  void monitor_icc_profile( const unsigned index )
  {
    const char* profile = open_icc_profile( NULL, 
					    "Load Monitor Profile" );
    if ( !profile ) return;

    mrv::Preferences::ODT_ICC_profile = profile;
    mrv::colorProfile::set_monitor_profile( profile, index );
  }


bool save_xml( const CMedia* img, mrv::ImageOpts* ipts,
               const char* file )
{
    namespace fs = boost::filesystem;

    if ( ipts && ipts->ACES_metadata() )
    {
        std::string root, frame, view, ext;
        mrv::split_sequence( root, frame, view, ext, file );

        fs::path f = root;
        std::string filename = f.filename().c_str();

        std::string xml;
        xml = f.parent_path().c_str();
        xml += "/ACESclip.";
        xml += filename;
        xml += "xml";

        LOG_INFO( "Saving XML ACES file " << xml );

        save_aces_xml( img, xml.c_str() );
    }
    return true;
}


void save_image_file( CMedia* image, const char* startdir )
{
   if (!image) return;

   const char* file = flu_save_chooser("Save Image", 
				       kIMAGE_PATTERN.c_str(), startdir);
   if ( !file ) return;

   std::string tmp = file;
   std::transform( tmp.begin(), tmp.end(), tmp.begin(),
		   (int(*)(int)) tolower);
   std::string ext = tmp.c_str() + tmp.size() - 4;

   ImageOpts* opts = ImageOpts::build( ext );
   if ( opts->active() )
   {
       image->save( file, opts );

       save_xml( image, opts, file );
   }

   delete opts;
}

void save_sequence_file( const mrv::ViewerUI* uiMain, 
			 const char* startdir, const bool opengl)
{

   const char* file = flu_save_chooser("Save Sequence", 
				       kIMAGE_PATTERN.c_str(), startdir);
   if ( !file ) return;

   
   std::string tmp = file;
   std::transform( tmp.begin(), tmp.end(), tmp.begin(),
		   (int(*)(int)) tolower);
   std::string ext = tmp.c_str() + tmp.size() - 4;

   bool movie = false;

   if ( ext == ".avi" || ext == ".mov" || ext == ".mp4" || ext == ".wmv" || 
	ext == ".mpg" || ext == ".mpeg" || ext == ".flv" )
   {
      movie = true;
   }

   std::string root, fileseq = file;
   bool ok = mrv::fileroot( root, fileseq );
   if ( !ok && !movie ) return;

   
   mrv::Timeline* timeline = uiMain->uiTimeline;
   int64_t first = int64_t( timeline->minimum() );
   int64_t last  = int64_t( timeline->maximum() );
   
   if ( movie )
   {
      root = root.substr( 0, root.size() - 4 );
   }

   fltk::ProgressBar* progress = NULL;
   fltk::Window* main = (fltk::Window*)uiMain->uiMain;
   fltk::Window* w = new fltk::Window( main->x(), main->y() + main->h()/2, 
				       main->w(), 80 );
   w->child_of(main);
   w->begin();
   progress = new fltk::ProgressBar( 0, 20, w->w(), w->h()-20 );
   progress->range( 0, double(last - first + 1) );
   progress->align( fltk::ALIGN_TOP );
   char title[1024];
   sprintf( title, _("Saving Sequence(s) %" PRId64 " - %" PRId64 ),
	    first, last );
   progress->label( title );
   progress->showtext(true);
   w->set_modal();
   w->end();
   
   fltk::check();
   
   int64_t dts = first;
   int64_t frame = first;
   int64_t failed_frame = frame-1;
   
   const char* fileroot = root.c_str();

   mrv::media old;
   bool open_movie = false;
   int movie_count = 1;
     
   bool edl = uiMain->uiTimeline->edl();

   int audio_stream = -1;

   ImageOpts* ipts = NULL;
   CMedia* img = NULL;

   if ( !movie )
   {
       mrv::media fg = uiMain->uiView->foreground();

       ipts = ImageOpts::build( ext );
       if ( !fg || !ipts->active() ) {
           delete ipts;
           return;
       }
       ipts->opengl( opengl );


       img = fg->image();

       save_xml( img, ipts, file );
   }


   for ( ; frame <= last; ++frame )
   {

       uiMain->uiReelWindow->uiBrowser->seek( frame );

      mrv::media fg = uiMain->uiView->foreground();
      if (!fg) break;

      img = fg->image();

      if ( old != fg )
      {

	 old = fg;
	 if ( open_movie )
	 {
            aviImage::close_movie(img);
	    open_movie = false;
	 }
         if ( movie )
	 {
	    char buf[256];
	    if ( edl )
	    {
	       sprintf( buf, "%s%d%s", root.c_str(), movie_count,
			  ext.c_str() );
	    }
	    else
	    {
	       sprintf( buf, "%s%s", root.c_str(), ext.c_str() );
	    }

	    if ( fs::exists( buf ) )
	    {
	       int ok = fltk::ask( "Do you want to replace '%s'",
				   buf );
	       if (!ok) 
	       {
		  break;
	       }
	    }

            AviSaveUI* opts = new AviSaveUI;
            if ( opts->video_bitrate == 0 &&
                 opts->audio_bitrate == 0 )
            {
                delete opts;
                delete w;
                w = NULL;
                break;
            }

            audio_stream = img->audio_stream();
            if ( opts->audio_codec == "NONE" )
            {
                img->audio_stream( -1 );
            }

	    if ( aviImage::open_movie( buf, img, opts ) )
	    {
               LOG_INFO( "Open movie '" << buf << "' to save." );
	       open_movie = true;
	       ++movie_count;
	    }

            delete opts;
	 }
      }

      if ( w )  w->show();
	
      {
	   
	 if (movie && open_movie)
	 {
             aviImage::save_movie_frame( img );
	 }
	 else 
	 {
             // Store old image
             mrv::image_type_ptr old_i = img->hires();
             if ( opengl )
             {
                 unsigned w = uiMain->uiView->w();
                 unsigned h = uiMain->uiView->h();

                 mrv::image_type_ptr hires( 
                 new mrv::image_type( img->frame(),
                                      w, h, 4,
                                      mrv::image_type::kRGBA,
                                      mrv::image_type::kFloat )
                 );

                 float* data = new float[ 4 * w * h ];

                 glPixelStorei( GL_PACK_ALIGNMENT, 1 );

                 int x = 0;
                 int y = 0;


                 glReadPixels( x, y, w, h, GL_RGBA, GL_FLOAT, data );

                 // Flip image vertically
                 for ( unsigned x = 0; x < w; ++x )
                 {
                     unsigned y2 = h-1;
                     for ( unsigned y = 0; y < h; ++y, --y2 )
                     {
                         unsigned xyw4 = 4 * (x + y * w);
                         mrv::ImagePixel p;
                         p.r = data[   xyw4 ];
                         p.g = data[ 1+xyw4 ];
                         p.b = data[ 2+xyw4 ];
                         p.a = data[ 3+xyw4 ];
                         hires->pixel( x, y2, p );
                     }
                 }

                 // Set new hires image from snapshot
                 img->hires( hires );
             }

	    char buf[1024];
	    sprintf( buf, fileroot, frame );
	    img->save( buf, ipts );

            if ( opengl )
            {
                img->hires( old_i );
            }
	 }
      }
	
      progress->step(1);
      fltk::check();
	
      if ( !w->visible() ) {
          break;
      }
   }


   delete ipts;

   if ( open_movie && img )
   {
      img->audio_stream( audio_stream );
      aviImage::close_movie(img);
      open_movie = false;
   }

   if ( w )
   {
      w->hide();
      w->destroy();
   }
}


  /** 
   * Attach a new audio file to loaded sequence
   * 
   * @param image      already loaded image
   * @param startfile  start filename (directory)
   */
  void attach_audio( CMedia* image, const char* startfile )
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
  const char* save_reel( const char* startdir )
  {
    return flu_save_chooser("Save Reel", 
			    kREEL_PATTERN.c_str(), startdir);
  }



}  // namespace mrv


