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
#include "core/aviImage.h"
#include "gui/mrvImageView.h"
#include "gui/mrvTimeline.h"

#include <GL/gl.h>

#include "FLU/Flu_File_Chooser.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace {

  // File extension patterns
  static const std::string kReelPattern = "reel";

  static const std::string kMoviePattern = "mp4,MP4,mpg,MPG,mpeg,MPEG,mov,MOV,qt,QT,avi,AVI,divx,DIVX,wmv,WMV,vob,VOB";

  static const std::string kImagePattern =
    "bmp,bit,cin,ct,dpx,exr,iff,jpg,JPG,jpeg,JPEG,map,nt,mt,pic,png,psd,rgb,rpf,"
    "shmap,sgi,st,sxr,tga,tif,tiff,zt";

  static const std::string kProfilePattern = "icc,icm,ICC,ICM";

  static const std::string kAudioPattern = "mp3,MP3,ogg,OGG,wav,WAV";


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
    flu_multi_file_chooser( "Load Reel(s)", 
			    kREEL_PATTERN.c_str(), startfile,
			    filelist );
    return filelist;
  }


  /** 
   * Opens a file requester to load image files
   * 
   * @param startfile  start filename (directory)
   * 
   * @return Each file to be opened
   */
  stringArray open_image_file( const char* startfile )
  {
    stringArray filelist;
    flu_multi_file_chooser( "Load Image(s)", 
			    kIMAGE_PATTERN.c_str(), startfile,
			    filelist );

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

    std::string script = make_ctl_browser( startfile, "RT_" );
    image->rendering_transform( script.c_str() );
  }



  void attach_ctl_script( CMedia* image )
  { 
    if (!image) return;

    const char* transform = image->rendering_transform();
    if ( !transform ) 
      transform = mrv::CMedia::rendering_transform_float.c_str();

    attach_ctl_script( image, transform );
  }



  void monitor_ctl_script( const unsigned index, const char* startfile )
  {
    if ( !startfile )
      startfile = mrv::Preferences::ODT_CTL_transform.c_str();

    std::string script = make_ctl_browser( startfile, "ODT_" );
    if ( script.empty() ) return;

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



  void save_image_file( CMedia* image, const char* startdir )
  {
    if (!image) return;

    const char* file = flu_save_chooser("Save Image", 
					kIMAGE_PATTERN.c_str(), startdir);
    if ( !file ) return;

    image->save( file );
  }

void save_sequence_file( CMedia* img, const mrv::ViewerUI* uiMain, 
			 const char* startdir)
{
    if (!img) return;

    const char* file = flu_save_chooser("Save Sequence", 
					kIMAGE_PATTERN.c_str(), startdir);
    if ( !file ) return;

    
     std::string tmp = file;
     std::transform( tmp.begin(), tmp.end(), tmp.begin(),
		     (int(*)(int)) tolower);
     std::string ext = tmp.c_str() + tmp.size() - 4;

     bool movie = false;
     if ( ext == ".avi" || ext == ".mov" || ext == ".mp4" || ext == ".wmv" )
      {
	 movie = true;
      }

     std::string root, fileseq = file;
     bool ok = mrv::fileroot( root, fileseq );
     if ( !ok && !movie ) return;

     mrv::Timeline* timeline = uiMain->uiTimeline;
     int64_t first = timeline->minimum();
     int64_t last  = timeline->maximum();

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
     progress->range( 0, last - first + 1 );
     progress->align( fltk::ALIGN_TOP );
     char title[1024];
     sprintf( title, "Saving Sequence(s) %" PRId64 " - %" PRId64,
	      first, last );
     progress->label( title );
     progress->showtext(true);
     w->set_modal();
     w->end();
     w->show();

     fltk::check();
     
     int64_t dts = first;
     int64_t frame = first;
     int64_t failed_frame = frame-1;

     const char* fileroot = root.c_str();

     mrv::media old;
     bool open_movie = false;
     int movie_count = 1;

     bool edl = uiMain->uiTimeline->edl();

     for ( ; frame <= last; ++frame )
     {
	int step = 1;
	
	uiMain->uiReelWindow->uiBrowser->seek( frame );
	mrv::media fg = uiMain->uiView->foreground();
	if (!fg) break;

	CMedia* img = fg->image();

	if ( old != fg )
	{
	   old = fg;
	   if ( open_movie )
	   {
	      aviImage::close_movie();
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

	      if ( aviImage::open_movie( buf, img ) )
	      {
		 open_movie = true;
		 ++movie_count;
	      }
	   }
	}

	
	{
	   
	   if (movie)
	   {
	      aviImage::save_movie_frame( img );
	   }
	   else 
	   {
	      char buf[1024];
	      sprintf( buf, fileroot, frame );
	      img->save( buf );
	   }
	}
	
	progress->step(1);
	fltk::check();
	
	if ( !w->visible() ) {
	   break;
	}
     }

     if ( open_movie )
     {
	aviImage::close_movie();
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


