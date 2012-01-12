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

#include <fltk/file_chooser.h>

#include "CMedia.h"
#include "mrvImageBrowser.h"
#include "mrvColorProfile.h"
#include "mrvFileRequester.h"
#include "mrvPreferences.h"
#include "mrViewer.h"

#include "FLU/Flu_File_Chooser.h"


namespace {

  // File extension patterns
  static const std::string kReelPattern = "reel";

  static const std::string kMoviePattern = "mp4,MP4,mpg,MPG,mpeg,MPEG,mov,MOV,qt,QT,avi,AVI,divx,DIVX,wmv,WMV,vob,VOB";

  static const std::string kImagePattern =
    "bmp,bit,cin,ct,dpx,exr,iff,jpg,jpeg,map,nt,mt,pic,png,psd,rgb,rpf,"
    "shmap,sgi,st,tga,tif,tiff,zt";

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

    mrv::colorProfile::set_monitor_profile( profile, index );
  }



  void save_image_file( const CMedia* image, const char* startdir )
  {
    if (!image) return;

    const char* file = flu_save_chooser("Save Image", 
					kSAVE_IMAGE_PATTERN.c_str(), startdir);
    if ( !file ) return;
    image->save( file );
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


